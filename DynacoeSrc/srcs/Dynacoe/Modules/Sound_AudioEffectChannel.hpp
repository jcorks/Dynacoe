
// Object to represent an independent audio channel with an effect chain
struct AudioEffectChannel {
    AudioEffectChannel() {
        data = nullptr;
        sizeBytes = 0;
        volume = 1.f;
        panning = .5f;
        keepAwake = false;
        limiterScale = 1.f;
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
    float limiterScale;
    bool keepAwake;

    // TODO:
    StateArray<AudioEffect*> effectChain;
};

