

#ifdef DC_OS_WINDOWS
    #include <thread>
    #include <mutex>
#else
    #include <thread>
    #include <mutex>
#endif

#include <Dynacoe/Modules/Input.h>

class AudioProcessor;
static AudioProcessor * instance;

class Dynacoe_Sound_AudioStreamHandler : public AudioManager::AudioStreamHandler {
  public:
    AudioProcessor * p;

    Dynacoe_Sound_AudioStreamHandler(AudioProcessor * processor) {
        p = processor;
    }

    void operator()(uint32_t count, float * data);

};


class AudioProcessor {
  public:


    AudioManager * manager;
    AudioProcessor(AudioProcessorIO * c) {
        instance = this;

        limiterScale = 1.f;
        ioShared = c;

        // ALL channels
        for(uint32_t i = 0; i < CHANNEL_COUNT; ++i) {
            AudioEffectChannel buffer;

            // allocates buffer. Is never destroyed intentionally.
            buffer.SetSize(1024*10);
            io.channels.Push(buffer);
        }

        // copy back over to ioShared
        *c = io;
        manager = (AudioManager *) Backend::CreateDefaultAudioManager();
        manager->Connect(new Dynacoe_Sound_AudioStreamHandler(instance));


        /*
        // start processing
        thread = new std::thread(ThreadControl);
        #ifdef DC_OS_WINDOWS
            #include <windows.h>

            HANDLE h = (HANDLE) ((std::thread*)thread)->native_handle();
            SetThreadPriority(h, THREAD_PRIORITY_TIME_CRITICAL);
        #endif
        */
    }


    void RunCommands() {
        for(uint32_t i = 0; i < io.commands.GetCount(); ++i) {     
            switch(io.commands.Get(i)) {
              case AudioProcessorIO::Command::DumpAllSamples:
                for(uint32_t i = 0; i < io.current.GetCount(); ++i) {
                    io.current.Get(i)->processor.destroy = true;
                }
                break;


            }
        }
        io.commands.Clear();

    }

    // Pushes played audio to the audio manager
    void ProcessAudio() {



        //Dynacoe::Console::Info() << "Samples this iteration: " << samplesThisIteration << "\n";


        mtlock.lock();
        io.channels = ioShared->channels;


        // first xfer the new sounds over here.
        for(uint32_t i = 0; i < ioShared->in.GetCount(); ++i) {
            io.current.Push(ioShared->in.Get(i));
            //printf("Pushed %d (%d left)\n", i, io.current.GetCount());
        }
        ioShared->in.Clear();


        // also copy over the commands from the ioShared.
        io.commands = ioShared->commands;
        ioShared->commands.Clear();



        // then, remove dead sounds and copy all to ioShared
        for(uint32_t i = 0; i < io.current.GetCount(); ++i) {
            AudioStreamObject * object = io.current.Get(i);
            if (object->processor.destroy) {
                ioShared->out.Push(object);
                //delete io.current.Get(i); // deletion of audiostreamobjects entirely handled by the audioprocessor
                io.current.Remove(i);
                i--;
                //printf("Removed %d (%d left)\n", i+1, ioShared->out.GetCount());
            }
        }
        ioShared->current = io.current;

        // dont forget state flags
        ioShared->status = io.status;



        // if we have any incoming commands, they'll be processed here.
        RunCommands();
        mtlock.unlock();

    }



    
    void MixBuffer(int samplesThisIteration, float * outputBuffer) {

        // first lets welcome the new sounds that were queued




        // If there are no stream objects to process, just get OUTTA HERE
        int total = io.current.GetCount();
        //set<AudioEffectChannel *> effectChannels;
        uint16_t channelVisited[CHANNEL_COUNT];
        memset(channelVisited, 0, CHANNEL_COUNT*2);
        memset(outputBuffer, 0, samplesThisIteration*2*sizeof(float));



        // Each pass, up to outputBufferSize samples are processed
        // for audio manager consumption


        uint32_t numAudioSamples = samplesThisIteration;
        uint32_t numManagerSamples = samplesThisIteration*2;
        AudioEffectChannel * effectBuffer;

        float leftPan;
        float rightPan;

        for(int n = 0; n < total; ++n) {
            AudioStreamObject * streamData = io.current.Get(n);
            if (streamData->processor.destroy || streamData->client.pause) continue;


            AudioBlock * sample = streamData->chunk;
            leftPan  = panning_to_multiplier_l(streamData->client.panning);
            rightPan = panning_to_multiplier_r(streamData->client.panning);


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
            effectBuffer = &io.channels.Get(streamData->client.channel);//GetAudioEffectChannel(streamData->channel);            // if not written to set this pass, zero it out.
            if (!channelVisited[streamData->client.channel]) {
                effectBuffer->Zero();
            }
            channelVisited[streamData->client.channel]++;

            // Now we know we have a good sound chunk to work with.

            // FIrst, we need to adjust where we start reading samples from. If the
            // First, put the data in the effect buffer for processing.
            // Since each AudioSample produces 2 samples of float data,
            // we only iterate through as many output sampels as we can carry
            // so here we are iterating over AudioSamples...

            uint32_t samplesProcessed = 0;
            for(uint32_t i = 0; i < numAudioSamples && (i+streamData->processor.sampleIndex) < streamData->processor.duration; ++i) {



                effectBuffer->data[i*2  ] +=  leftPan  * streamData->client.volume * sample->GetSample(i + streamData->processor.sampleIndex).NormalizedL();
                effectBuffer->data[i*2+1] +=  rightPan * streamData->client.volume * sample->GetSample(i + streamData->processor.sampleIndex).NormalizedR();

                samplesProcessed++;


            }


            //std::cout << "Prepared " << samplesProcessed << " samples to effect buffer" << std::endl;
            streamData->processor.sampleIndex += samplesProcessed;
        }





        // finally apply to the master channel buffer
        for(uint32_t i = 0; i < CHANNEL_COUNT; ++i) {
            AudioEffectChannel * buffer = &io.channels.Get(i);
            if (!channelVisited[i] && !buffer->keepAwake) continue;

            
            float leftPan  = panning_to_multiplier_l(buffer->panning);
            float rightPan = panning_to_multiplier_r(buffer->panning);

            // scan for limiting needs
            float highest = 0;
            for(uint32_t n = 0; n < numManagerSamples; ++n) {                
                buffer->data[n] = ((i%2==0 ? leftPan : rightPan) * buffer->volume * buffer->data[n]);                       if (fabs(buffer->data[n]) > highest)
                if (fabs(buffer->data[n]) > highest)
                    highest = fabs(buffer->data[n]);
            }
            
            // current limiter is not enough
            if ((highest * buffer->limiterScale) > 1.f) {
                buffer->limiterScale = 1 / highest;
            }
            
            // aply limiter (and phase it out)
            if (buffer->limiterScale < 1.f) {
                for(uint32_t n = 0; n < numManagerSamples; ++n) {                
                    buffer->data[n] *= buffer->limiterScale;
                }    
                
                buffer->limiterScale = Mutator::StepTowards(buffer->limiterScale, 1.f, .1);
                //printf("[%d]LIMITER: %f\n", i, buffer->limiterScale);
            }




            // then, apply all channel effects sequentially.
            for(uint32_t n = 0; n < buffer->effectChain.GetCount(); ++i) {
                (*buffer->effectChain.Get(n))(buffer->data, buffer->sizeBytes / (sizeof(float)*2));
            }
            
            

            // and xfer to main mix
            for(uint32_t n = 0; n < numManagerSamples; ++n) {
                outputBuffer[n] += buffer->data[n];
            }

            

        }






        // Phase II: Master channel handling
        // Now that we know all out final buffer contents, lets
        // apply any master effects

        // HERE


        // and perform limiting
        float highest = 0;
        for(uint32_t i = 0; i < samplesThisIteration; ++i) {
            if (fabs(outputBuffer[i]) > highest)
                highest = fabs(outputBuffer[i]);
        }
        
        // needs better limiting
        if (highest * limiterScale > 1.f) {
            limiterScale = 1/highest;
        }

        // limit if needed
        if (limiterScale < 1.f) {
            for(uint32_t i = 0; i < samplesThisIteration; ++i) {
                outputBuffer[i] *= limiterScale;
            }
            limiterScale = Mutator::StepTowards(limiterScale, 1.f, .1);
        }



        //printf("Waiting on %f KB of audio\n", (outputBufferSamplesLeft/ (float) 1024));
    }
 

  private:
    float limiterScale;


    AudioProcessorIO io;
    AudioProcessorIO * ioShared;

    void * thread;

    /*
    static void ThreadControl() {
        
        // TODO: determine good, stable sleep amount
        while(1) {
            Time::SleepMS(16);
        }
    }
    */






};



void Dynacoe_Sound_AudioStreamHandler::operator()(uint32_t count, float * data) {
    //printf("sample count: %d\n", count);
    p->ProcessAudio();
    p->MixBuffer(count/2, data);
    
    /*
    int sr = p->GetSampleRate(); 
    static int counter = 0;       

    if (Dynacoe::Input::GetState(MouseButtons::Left)) {
        for(uint32_t i = 0; i < count/2; ++i) {
            float v = sin(440 * (counter / (float)sr) * 3.1459 * 2);

            data[i*2+0] = v;
            data[i*2+1] = v;
            counter++;
     
       }
    } else {
        for(uint32_t i = 0; i < count/2; ++i) {
            data[i*2+0] = 0;
            data[i*2+1] = 0;
            counter++;
     
       }

    }
    */

}



