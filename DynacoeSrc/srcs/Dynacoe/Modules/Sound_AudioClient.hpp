


// Sends commands to the audio processor
class AudioClient {


  public:

    AudioClient() {

        processor = new AudioProcessor(&ioShared);
    }


    uint32_t GetSampleRate();

    void UpdateMain() {
        //processor->ProcessAudio();

        // need a better process.. There are much more efficient ways to do this
        auto list = activeSoundIDs;
        auto source = ioOwned.current;

        for(uint32_t i = 0; i < list.size(); ++i) {
            auto current = activeSounds.Find(list[i]);
            for(uint32_t n = 0; n < source.GetCount(); ++n) {
                if (current == source.Get(n)) {
                    source.Remove(n);
                    list.erase(list.begin()+i);
                    i--;
                    break;
                }
            }
        }
        
        for(uint32_t i = 0; i < list.size(); ++i) {
            for(uint32_t n = 0; n < activeSoundIDs.size(); ++n) {
                if (activeSoundIDs[n] == list[i]) {
                    activeSoundIDs.erase(activeSoundIDs.begin()+n);
                    break;
                }
            }
            activeSounds.Remove(list[i]);
        }
        
    }




    AudioManager * GetManager() {
        // thread safe?
        return processor->manager;
    }



    void AddEffect(AudioEffect * effect, uint8_t channel) {
        ioOwned.channels.Get(channel).effectChain.Push(effect);
    }
    void RemoveEffect(AudioEffect * effect, uint8_t channel) {
        AudioEffectChannel * c = &ioOwned.channels.Get(channel);
        for(uint32_t i = 0; i < c->effectChain.GetCount(); ++i) {
            if (c->effectChain.Get(i) == effect) {
                c->effectChain.Remove(i);
                return;
            }
        }
    }

    void Reset(uint8_t channel) {
        AudioEffectChannel * c = &ioOwned.channels.Get(channel);
        c->effectChain.Clear();
    }

    void KeepAwake(uint8_t channel, bool ka) {
        AudioEffectChannel * c = &ioOwned.channels.Get(channel);
        c->keepAwake = ka;
    }


    // sets the volume for the givne effect channel
    void ChannelSetVolume(uint8_t channel, float v) {
        AudioEffectChannel * buffer = &ioOwned.channels.Get(channel);
        buffer->volume = v;
        Math::Clamp(buffer->volume, 0.f, 1.f);
    }


    void ChannelSetPanning(uint8_t channel, float panning) {
        AudioEffectChannel * buffer = &ioOwned.channels.Get(channel);
        buffer->panning = panning;
        Math::Clamp(buffer->panning, 0.f, 1.f);
    }




    bool IsSampleActive(AudioBlock * addr) {
        for(uint32_t i = 0; i < ioOwned.current.GetCount(); ++i) {
            if (ioOwned.current.Get(i)->chunk == addr)
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
        object->client.channel = channel %CHANNEL_COUNT;
        object->client.volume = volume;
        object->client.panning = panning;


        if (duration > 0)
            object->processor.duration = duration;
        else
            object->processor.duration = target->NumBytes() / sizeof(AudioSample);

        // TODO: set proper hold sampels based on time since last commit to audio manager
        ioOwned.in.Push(object);

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

    AudioProcessorIO ioOwned;
    AudioProcessorIO ioShared;
    Dynacoe::Clock timer;


    AudioProcessor * processor;
};

