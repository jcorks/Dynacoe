

#ifdef DC_OS_WINDOWS
    #include <thread>
    #include <mutex>
#else
    #include <thread>
    #include <mutex>
#endif


class AudioProcessor {
  public:


    AudioManager * manager;
    AudioProcessor(AudioProcessorIO * c, StateArray * newSounds) {
        instance = this;
        manager = (AudioManager *) Backend::CreateDefaultAudioManager();
        manager->Connect();

        outputBufferSize = 1024*80; //512*2; //1024*7;
        outputBuffer = (float*)new char[outputBufferSize];
        memset(outputBuffer, 0, outputBufferSize);
        limiterScale = 1.f;
        client = c;

        // ALL channels
        for(uint32_t i = 0; i < CHANNEL_COUNT; ++i) {
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


            }
        }
        io.commands.Clear();

    }

    // Pushes played audio to the audio manager
    void ProcessAudio() {
        
        // Before the frame officially starts, we want to determine how 
        // many frames we wish to deliver to the audio manager
        uint32_t count = manager->PendingSamplesCount();
        int FPS = Dynacoe::Engine::GetDiagnostics().currentFPS;
        if (FPS == 0) FPS = Dynacoe::Engine::GetMaxFPS();
        uint32_t sampleRate = manager->GetSampleRate();
        uint32_t idealRate = (sampleRate/FPS)*1.1; //1.1 gives us a little leeway but shouldnt cause too much lag

        
        if (count > 2*idealRate) { // already pretty backed up, so we can relax the rate a little
            samplesThisIteration = idealRate*.5;
            //std::cout << "!!Buffer too full! dropping a frame\n";
        } else if (count < .5*idealRate){ // buffer too low so lets up the numder of delivered frames
            //std::cout << "!!Buffer too empty! adding an extra frame\n";
            samplesThisIteration = idealRate*2;
        } else { // normal. within tolerance
            samplesThisIteration = idealRate;
        }
        //std::cout << count << "frames waiting\n";
        
        

        if (samplesThisIteration*2 >= outputBufferSize/(sizeof(float))) samplesThisIteration = (outputBufferSize/sizeof(float)-1)/2;


        //Dynacoe::Console::Info() << "Samples this iteration: " << samplesThisIteration << "\n";


        if (!io.channels.lock) {
            io.channels.lock = true;
            io.channels = client->channels;


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


            //audio_processor_mutex.unlock();

            // if we have any incoming commands, they'll be processed here.
            RunCommands();
            io.channels.lock = false;
        }


        ProcessStreamChunks();
        ProcessMasterChannel();







        CommitProcessedData();
        //audio_processor_mutex.unlock();
    }

  private:
    float * outputBuffer;
    uint32_t outputBufferSize;
    uint32_t samplesThisIteration;
    float limiterScale;


    AudioProcessorIO io;
    AudioProcessorIO * client;

    //void * thread;

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


        manager->PushData(outputBuffer, samplesThisIteration);
        memset(outputBuffer, 0, samplesThisIteration*sizeof(float)*2);


    }



    void ProcessStreamChunks() {

        // first lets welcome the new sounds that were queued




        // If there are no stream objects to process, just get OUTTA HERE
        int total = io.current.GetCount();
        //set<AudioEffectChannel *> effectChannels;
        uint16_t channelVisited[CHANNEL_COUNT];
        memset(channelVisited, 0, CHANNEL_COUNT*2);



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
            effectBuffer = &io.channels.Get(streamData->client.channel);//GetAudioEffectChannel(streamData->channel);
            // if not written to set this pass, zero it out.
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
                buffer->data[n] = ((i%2==0 ? leftPan : rightPan) * buffer->volume * buffer->data[n]);
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






        //printf("Waiting on %f KB of audio\n", (outputBufferSamplesLeft/ (float) 1024));
    }
    
    void ProcessMasterChannel() {

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
            //printf("MASTER_LIMITER: %f\n", limiterScale);
        }
    }




};







