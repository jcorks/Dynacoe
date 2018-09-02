// data exchanged between the clinet and the processor
class AudioProcessorIO {
  public:

    AudioProcessorIO() {
        status = 0;
        lock = 0;
    }


    enum class Command {
        DumpAllSamples,

    };

    enum class StateFlag {
        None,
        Active = 1,
    };

    uint32_t status;
    int lock;

    // current objects being processed by the stream
    StateArray<AudioStreamObject*> current;

    // client-given commands to the processor
    StateArray<Command> commands;

    // effect channels
    StateArray<AudioEffectChannel> channels;

    // Newly created stream obejcts
    StateArray<AudioStreamObject *> in;


};