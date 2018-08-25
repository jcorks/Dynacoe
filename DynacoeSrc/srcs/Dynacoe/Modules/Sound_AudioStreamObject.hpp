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
