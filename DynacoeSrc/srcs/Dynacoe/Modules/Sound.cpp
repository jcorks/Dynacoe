/*

Copyright (c) 2018, Johnathan Corkery. (jcorkery@umich.edu)
All rights reserved.

This file is part of the Dynacoe project (https://github.com/jcorks/Dynacoe)
Dynacoe was released under the MIT License, as detailed below.



Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished 
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.



*/


#include <Dynacoe/Modules/Sound.h>

#include <Dynacoe/Components/Clock.h>
#include <Dynacoe/Util/Filesys.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Util/Time.h>
#include <Dynacoe/AudioBlock.h>
#include <set>
#include <cassert>
#include <cmath>
#include <sstream>
#include <cstring>

#define WAV_HEADER_SIZE 44
#define AudioSample_LIMIT 32767
#define BUFFER_SIZE 512
#include <cstdio>


#ifdef DC_OS_WINDOWS
    #include <thread>
    #include <mutex>
#else
    #include <thread>
    #include <mutex>
#endif


using namespace Dynacoe;
using namespace std;





// Bare-bones resizing array that has the following properties:
// - pure data moving; no invocation of copy constructors / assignment operator
// - tight allocation, not meant for high volume
// - no stl or static memory usage.
// - assertions for out-of-bounds usage
template <typename T>
class StateArray {
  public:
    StateArray() :
        dataSize(0),
        dataAllocated(0),
        data(nullptr) {}

    ~StateArray() {
        free(data);
    }


    StateArray & operator=(const StateArray & other) {
        if (dataAllocated <= other.dataSize) {
            data = (uint8_t*)realloc(data, other.dataSize);
            dataAllocated = other.dataSize;
        }
        dataSize = other.dataSize;
        memcpy(data, other.data, other.dataSize);
    }

    StateArray(const StateArray & other) : StateArray() {
        *this = other;
    }

    void Push(const T & in) {
        if (dataSize + sizeof(T) >= dataAllocated) {
            data = (uint8_t*)realloc(data, dataSize + sizeof(T));
            dataAllocated = dataSize + sizeof(T);
        }
        memcpy(data + dataSize, &in, sizeof(T));
        dataSize += sizeof(T);
    }

    void Remove(uint32_t i) {
        uint32_t index = i*sizeof(T);
        assert(index < dataSize);
        if (i != GetCount()-1)
            memmove(data+index, data+index+sizeof(T), dataSize-(index+sizeof(T)));
        dataSize -= sizeof(T);
    }

    void Clear() {
        dataSize = 0;
    }

    uint32_t GetCount() {
        return dataSize / sizeof(T);
    }

    T & Get(uint32_t i) {
        uint32_t index = i*sizeof(T);
        assert(index < dataSize);
        return *(T*)(data+index);
    }

  private:
    uint32_t dataSize;
    uint32_t dataAllocated;
    uint8_t * data;
};




// bject that represents a chunk of active audio data to
// be sent to the audio server
struct AudioStreamObject {
    AudioStreamObject(AudioBlock * s) {
        chunk = s;
    }

    AudioBlock * chunk;

    // clients write
    // processor reads
    struct ClientSideData {
        ClientSideData() :
            volume{1.f},
            panning{.5},
            pause{false},
            repeat{false},
            channel{0},
            seek{0}{}

        float volume;
        float panning;
        bool pause;
        bool repeat;
        uint8_t channel;
        uint32_t seek;
    };



    // processor writes
    // client reads
    struct ProcessorSideData {
        ProcessorSideData() :
            sampleIndex{0},
            duration{0},
            destroy{0}{}

        uint32_t sampleIndex; // samples
        uint32_t duration; // usually the size of the chunk in bytes
        bool destroy;
    };


    ClientSideData client;
    ProcessorSideData processor;
};


// Object to represent an independent audio channel with an effect chain
struct AudioEffectChannel {
    AudioEffectChannel() {
        data = nullptr;
        sizeBytes = 0;
        volume = 1.f;
        panning = .5f;
    };

    void SetSize(uint32_t bytes) {
        if (data) free(data);
        data = (float*)malloc(bytes);
        sizeBytes = bytes;
    }


    void Zero() {
        memset(data, 0, sizeBytes);
    }

    float * data;
    uint32_t sizeBytes;
    float volume;
    float panning;

    // TODO:
    StateArray<AudioEffect*> effectChain;
};






static std::recursive_mutex audio_processor_mutex;
static Table<AudioStreamObject *> activeSounds;
static std::vector<LookupID> activeSoundIDs;



// data exchanged between the clinet and the processor
class AudioProcessorIO {
  public:

    AudioProcessorIO() {
        status = 0;
    }


    enum class Command {
        DumpAllSamples,
        RecordBegin,
        RecordEnd
    };

    enum class StateFlag {
        None,
        Active = 1,
        Recording = 2,
    };

    uint32_t status;


    // current objects being processed by the stream
    StateArray<AudioStreamObject*> current;

    // new. incoming objects from the client
    StateArray<AudioStreamObject*> in;

    // client-given commands to the processor
    StateArray<Command> commands;

    // effect channels
    StateArray<AudioEffectChannel> channels;




};







class AudioProcessor;
static AudioProcessor * instance;
class AudioProcessor {
  public:


    AudioManager * manager;

    AudioProcessor(AudioProcessorIO * c) {
        instance = this;
        manager = (AudioManager *) Backend::CreateDefaultAudioManager();
        manager->Connect();

        outputBufferSize = 1024; //512*2; //1024*7;
        outputBuffer = (float*)new char[outputBufferSize];
        memset(outputBuffer, 0, outputBufferSize);
        outputBufferSamplesLeft = 0;
        client = c;

        // ALL channels
        for(uint32_t i = 0; i <= 255; ++i) {
            AudioEffectChannel buffer;

            // allocates buffer. Is never destroyed intentionally.
            buffer.SetSize(outputBufferSize);
            io.channels.Push(buffer);
        }

        // copy back over to client
        *c = io;

        // start processing
        thread = new std::thread(ThreadControl);
        #ifdef DC_OS_WINDOWS
            #include <windows.h>

            HANDLE h = (HANDLE) ((std::thread*)thread)->native_handle();
            SetThreadPriority(h, THREAD_PRIORITY_TIME_CRITICAL);
        #endif
    }


    void RunCommands() {
        for(uint32_t i = 0; i < io.commands.GetCount(); ++i) {
            switch(io.commands.Get(i)) {
              case AudioProcessorIO::Command::DumpAllSamples:
                for(uint32_t i = 0; i < io.current.GetCount(); ++i) {
                    io.current.Get(i)->processor.destroy = true;
                }
                break;

              case AudioProcessorIO::Command::RecordEnd:
                AudioBlock * out;
                out = new AudioBlock("$Audio::Recording");
                out->Define(&outputBufferCopy[0], outputBufferCopy.size());
                if (lastRecording)
                    delete lastRecording;

                lastRecording = out;
                io.status &= ~(uint32_t)AudioProcessorIO::StateFlag::Recording;
                break;


              case AudioProcessorIO::Command::RecordBegin:
                outputBufferCopy.clear();
                io.status &= (uint32_t)AudioProcessorIO::StateFlag::Recording;
                break;
            }
        }
        io.commands.Clear();

    }

    // Pushes played audio to the audio manager
    void ProcessAudio() {

        if (audio_processor_mutex.try_lock()) {


            // first xfer the new sounds over here.
            for(uint32_t i = 0; i < client->in.GetCount(); ++i) {
                io.current.Push(client->in.Get(i));
            }
            client->in.Clear();


            // also copy over the commands from the client.
            io.commands = client->commands;
            client->commands.Clear();



            // then copy all client data
            for(uint32_t i = 0; i < client->current.GetCount(); ++i) {
                io.current.Get(i)->client = client->current.Get(i)->client;
            }


            // then, remove dead sounds and copy all to client
            for(uint32_t i = 0; i < io.current.GetCount(); ++i) {
                AudioStreamObject * object = io.current.Get(i);
                if (object->processor.destroy) {
                    delete io.current.Get(i);
                    io.current.Remove(i);
                    i--;
                }
            }
            client->current = io.current;

            // dont forget state flags
            client->status = io.status;


            audio_processor_mutex.unlock();

            // if we have any incoming commands, they'll be processed here.
            RunCommands();
        }


        ProcessStreamChunks();
        ProcessMasterChannel();





        if (((int)io.status) & ((int)AudioProcessorIO::StateFlag::Recording)) {  // feedback buffer of the data processed
            AudioSample nextSample;
            for(unsigned int i = 0; i < outputBufferSize/2;) {
                // convert back to normal samples
                ((uint16_t*)&nextSample)[0] = (uint16_t)(outputBuffer[i++]+1)*UINT16_MAX;
                ((uint16_t*)&nextSample)[1] = (uint16_t)(outputBuffer[i++]+1)*UINT16_MAX;
                outputBufferCopy.push_back(nextSample);
            }
        }



        CommitProcessedData();
        //audio_processor_mutex.unlock();
    }

  private:
    float * outputBuffer;
    uint32_t outputBufferSize;
    uint32_t outputBufferSamplesLeft;


    AudioBlock * lastRecording;
    std::vector<AudioSample> outputBufferCopy;

    AudioProcessorIO io;
    AudioProcessorIO * client;

    void * thread;

    static void ThreadControl() {
        try {
            // TODO: determine good, stable sleep amount
            while(1) {
                instance->ProcessAudio();
                Time::SleepMS(2);
            }
        } catch(std::exception & e) {
            exit(1);
        }
    }


    void CommitProcessedData() {
        // Phase 0: Commit processed data

        // If there are any samples that have been fully computed already
        // that havent been processed by the AudioManager yet,
        // do not do any further processing and just focus on pushing to
        // the AudioManager.

        // to the audio manager, samples are floats
        //int bytesToGet = (.001 * timer.GetTimeSince() * manager->GetSampleRate())*(sizeof(float)*2;
        //int bytesToGet = outputBufferSamplesLeft*sizeof(float);
        //if (bytesToGet > outputBufferSize) bytesToGet = outputBufferSize;
        //int bytesToGet = 100;

        //uint32_t bufferSamplesCurrent = outputBufferSamplesLeft;


        //if (outputBufferSamplesLeft*sizeof(AudioSample) >= bytesToGet) {
        uint32_t samplesProcessedTotal = 0;



        while(outputBufferSamplesLeft) {

            uint32_t samplesProcessed = (outputBufferSamplesLeft) - manager->PushData(outputBuffer, outputBufferSamplesLeft);

            outputBufferSamplesLeft -= samplesProcessed;
            uint32_t bytesProcessed = samplesProcessed*sizeof(float);
            // now that the beginning has been consumed, move the buffer down

            memmove(outputBuffer, outputBuffer+(samplesProcessed), outputBufferSize-(bytesProcessed));
            memset(outputBuffer+((outputBufferSize-bytesProcessed)/(sizeof(float))), 0, bytesProcessed);

            samplesProcessedTotal += samplesProcessed;
        }



        if (outputBufferSamplesLeft) {
            printf("Buffer was too full..\n");
        }


    }



    void ProcessStreamChunks() {

        // first lets welcome the new sounds that were queued



        /*
        if (count) {
            printf("Added %d chunks (%d total)\n", count, chunkIndex.size());
        }
        */

        // If there are no stream objects to process, just get OUTTA HERE
        int total = io.current.GetCount();
        //set<AudioEffectChannel *> effectChannels;
        uint8_t channelVisited[256];
        memset(channelVisited, 0, 256);


        // Each pass, up to outputBufferSize samples are processed
        // for audio manager consumption


        uint32_t numAudioSamples = outputBufferSize/(sizeof(AudioSample)*2);
        uint32_t numManagerSamples = outputBufferSize/(sizeof(float));
        AudioEffectChannel * effectBuffer;

        float leftPan;
        float rightPan;

        for(int n = 0; n < total; ++n) {
            AudioStreamObject * streamData = io.current.Get(n);
            if (streamData->processor.destroy || streamData->client.pause) continue;


            AudioBlock * sample = streamData->chunk;
            leftPan = (2 - streamData->client.panning*2);
            rightPan =     streamData->client.panning*2;

            Math::Clamp(leftPan, 0.f, 1.f);
            Math::Clamp(rightPan, 0.f, 1.f);


            // First check to see if the sample has already been exhausted.
            // If it has, go to the next one and mark it for deletion.
            if ( streamData->processor.sampleIndex >=
                 streamData->processor.duration) {
                if (streamData->client.repeat) {
                    streamData->processor.sampleIndex = 0;
                } else {
                    streamData->processor.destroy = true;
                    continue;
                }
            }

            // should be the channel associated with the stream chunk;
            effectBuffer = &io.channels.Get(streamData->client.channel);//GetAudioEffectChannel(streamData->channel);
            // if not written to set this pass, zero it out.
            if (!channelVisited[streamData->client.channel]) {
                effectBuffer->Zero();
                channelVisited[streamData->client.channel] = true;
            }


            // Now we know we have a good sound chunk to work with.

            // FIrst, we need to adjust where we start reading samples from. If the
            // First, put the data in the effect buffer for processing.
            // Since each AudioSample produces 2 samples of float data,
            // we only iterate through as many output sampels as we can carry
            // so here we are iterating over AudioSamples...

            uint32_t samplesProcessed = 0;
            //std::cout << "@ " << streamData->sampleIndex << " out of " << streamData->duration << std::endl;
            for(uint32_t i = 0; i < numAudioSamples && (i+streamData->processor.sampleIndex) < streamData->processor.duration; ++i) {
                /*
                if (i%2)
                    effectBuffer->data[i] = sample->GetSample( (((i/2) + streamData->sampleIndex))).NormalizedR() * (sample->GetVolume() / 256.0) *  sample->GetPanning();
                else
                    effectBuffer->data[i] = sample->GetSample( (((i/2) + streamData->sampleIndex))).NormalizedL() * (sample->GetVolume() / 256.0) *  (1.f - sample->GetPanning());
                */



                effectBuffer->data[i*2  ] +=  leftPan  * streamData->client.volume * sample->GetSample(i + streamData->processor.sampleIndex).NormalizedL();
                effectBuffer->data[i*2+1] +=  rightPan * streamData->client.volume * sample->GetSample(i + streamData->processor.sampleIndex).NormalizedR();

                assert(i*2*sizeof(float) < effectBuffer->sizeBytes);
                assert((i*2+1)*sizeof(float) < effectBuffer->sizeBytes);
                samplesProcessed++;


            }
            //std::cout << "Prepared " << samplesProcessed << " samples to effect buffer" << std::endl;
            streamData->processor.sampleIndex += samplesProcessed;
        }





        // finally apply to the master channel buffer
        for(uint32_t i = 0; i <= 255; ++i) {
            if (!channelVisited[i]) continue;


            AudioEffectChannel * buffer = &io.channels.Get(i);

            // then, apply all channel effects sequentially.
            for(uint32_t i = 0; i < buffer->effectChain.GetCount(); ++i) {
                (*buffer->effectChain.Get(i))(buffer->data, buffer->sizeBytes / (sizeof(float)*2));
            }

            // and xfer to main mix
            for(uint32_t i = 0; i < numManagerSamples; ++i) {
                outputBuffer[i] += (i%2==0? (1.f-buffer->panning) : buffer->panning) * buffer->volume * buffer->data[i];
            }
        }





        outputBufferSamplesLeft += numManagerSamples;

        //printf("Waiting on %f KB of audio\n", (outputBufferSamplesLeft/ (float) 1024));
    }

    void ProcessMasterChannel() {

        // Phase II: Master channel handling
        // Now that we know all out final buffer contents, lets
        // apply any master effects

        // HERE


        // and perform clipping
        for(uint32_t i = 0; i < outputBufferSize/sizeof(float); ++i) {
            Math::Clamp(outputBuffer[i], -1.f, 1.f);
        }
    }




};







// Sends commands to the audio processor
class AudioClient {


  public:

    AudioClient() {

        processor = new AudioProcessor(&io);
    }


    uint32_t GetSampleRate();

    void UpdateMain() {

    }




    AudioManager * GetManager() {
        // thread safe?
        return processor->manager;
    }



    void AddEffect(AudioEffect * effect, uint8_t channel) {
        io.channels.Get(channel).effectChain.Push(effect);
    }
    void RemoveEffect(AudioEffect * effect, uint8_t channel) {
        AudioEffectChannel * c = &io.channels.Get(channel);
        for(uint32_t i = 0; i < c->effectChain.GetCount(); ++i) {
            if (c->effectChain.Get(i) == effect) {
                c->effectChain.Remove(i);
                return;
            }
        }
    }


    // sets the volume for the givne effect channel
    void ChannelSetVolume(uint8_t channel, float v) {
        AudioEffectChannel * buffer = &io.channels.Get(channel);
        buffer->volume = v;
        Math::Clamp(buffer->volume, 0.f, 1.f);
    }


    void ChannelSetPanning(uint8_t channel, float panning) {
        AudioEffectChannel * buffer = &io.channels.Get(channel);
        buffer->panning = panning;
        Math::Clamp(buffer->panning, 0.f, 1.f);
    }



    // record data
    void RecordBegin() {
        io.commands.Push(AudioProcessorIO::Command::RecordBegin);
    }
    void RecordEnd() {
        io.commands.Push(AudioProcessorIO::Command::RecordEnd);
    }



    // retrieves an audio block of the last recording
    AudioBlock * GetLastRecording() {
        // TODO: need to transfer from processor ._.
        return nullptr;

    }



    bool IsSampleActive(AudioBlock * addr) {
        for(uint32_t i = 0; i < io.current.GetCount(); ++i) {
            if (io.current.Get(i)->chunk == addr)
                return true;
        }
        return false;
    }


    // play a registered sound chunk
    LookupID PlayPCM(AssetID i, float volume, float panning, uint8_t channel) {
        if (!i.Valid()) return LookupID();
        return PlayPCM(i, volume, panning,  (&Assets::Get<AudioBlock>(i))->NumBytes()/sizeof(AudioSample), channel);
    }

    // play a registered chunk for a specified number of buffer frames;
    LookupID PlayPCM(AssetID i, float volume, float panning, int duration, uint8_t channel) {
        if (!i.Valid()) return LookupID();

        AudioBlock * target = &Assets::Get<AudioBlock>(i);

        AudioStreamObject * object = new AudioStreamObject(target);
        object->client.channel;
        object->client.volume = volume;
        object->client.panning = panning;


        if (duration > 0)
            object->processor.duration = duration;
        else
            object->processor.duration = target->NumBytes() / sizeof(AudioSample);

        // TODO: set proper hold sampels based on time since last commit to audio manager
        io.in.Push(object);

        auto id = activeSounds.Insert(object);
        activeSoundIDs.push_back(id);
        return id;
    }

    // requests the stopping of all samples in the process of being in playback
    void RequestActiveSampleDump() {
        io.commands.Push(AudioProcessorIO::Command::DumpAllSamples);
    }

    // returns the number of samples that are currently being processed
    int NumActivePlaybackSamples() {
        return io.current.GetCount();
    }


    // returns the validity of the Audio module.
    // Audio is only valid if an output device is capable and ready.
    bool isValid() {
        return processor->manager;
    }



  private:

    AudioProcessorIO io;
    Dynacoe::Clock timer;


    AudioProcessor * processor;
};








AudioClient *      Sound::a;





static const int DYNACOE_SAMPLE_RATE =        44100;       // (Samples processed per second)












ActiveSound::ActiveSound() {
    state = LookupID();
}

ActiveSound::ActiveSound(const ActiveSound & other) {
    state = other.state;
}

ActiveSound & ActiveSound::operator=(const ActiveSound & other) {
    state = other.state;
    return *this;
}

ActiveSound::ActiveSound(LookupID id) {
    state = id;
    Updated();
}

void ActiveSound::SetVolume(float v) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    Math::Clamp(v, 0.f, 1.f);
    s->client.volume = v;
}

void ActiveSound::SetPanning(float p) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.panning = p;
}

void ActiveSound::SetRepeat(bool b) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.repeat = b;
}

void ActiveSound::Seek(float b) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    Math::Clamp(b, 0.f, 1.f);
    s->client.seek = b;
}

void ActiveSound::SetChannel(uint32_t t) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.channel = t;
}

void ActiveSound::Stop() {
     AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    // TODO: client side stop
    s->client.volume = 0.f;
}

void ActiveSound::Pause() {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.pause = true;
}

void ActiveSound::Resume() {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.pause = false;
}

bool ActiveSound::Valid() {
    return activeSounds.Query(state);
}

uint8_t * ActiveSound::Updated() {
    if (!state.Valid()) return nullptr;
    AudioStreamObject * out = activeSounds.Find(state);

    // could use an invalidation system in the future maybe.
    //out->needsUpdate = true;
    return (uint8_t*)out;
}







void Sound::Init() {

    static bool spawned = false;

    if (spawned) return;
    spawned = true;

    a = new AudioClient();

}

void Sound::InitAfter(){}

Backend * Sound::GetBackend() {
        return a->GetManager();
}

void Sound::RunBefore() {}
void Sound::RunAfter()  {
    a->UpdateMain();
}
void Sound::DrawBefore(){}
void Sound::DrawAfter() {}


















ActiveSound Sound::PlayAudio(AssetID i, uint8_t effectChannel, float volume, float panning) {
    if (!a) return ActiveSound(LookupID());
    AudioBlock * target = &Assets::Get<AudioBlock>(i);
    Math::Clamp(volume, 0.f, 1.f);
    Math::Clamp(panning, 0.f, 1.f);
    if (volume == 0.f) return ActiveSound(LookupID());
    if (target) {
        // play a chunk, any chunk
        return ActiveSound(a->PlayPCM(i, volume, panning, effectChannel));
    } else {
        Console::Error()<<("Request to play sound failed!")<< Console::End;
        return ActiveSound(LookupID());

    }


}


void Sound::ChannelSetVolume(uint8_t channel, float v) {
    a->ChannelSetVolume(channel, v);
}

void Sound::ChannelSetPanning(uint8_t channel, float v) {
    a->ChannelSetPanning(channel, v);
}


























AssetID Sound::WriteRecording() {
    AssetID out = Assets::New(Assets::Type::Audio, "$exportedAudioBlock");
    AudioBlock & block = Assets::Get<AudioBlock>(out);
    block.AppendBlock(a->GetLastRecording());
    return out;
}




void Sound::RecordOutput(bool doIt) {
    if (doIt)
        a->RecordBegin();
}






AudioManager * Sound::GetManager() {
    return a->GetManager();
}


uint32_t AudioClient::GetSampleRate() {
    uint32_t sampleRate = processor->manager->GetSampleRate();
    return sampleRate;
}


bool Sound::IsPlaying(AssetID index) {
    if (!a) return false;
    AudioBlock * addr = &Assets::Get<AudioBlock>(index);
    return a->IsSampleActive(addr);
}
