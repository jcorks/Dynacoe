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
        data(nullptr),
        lock(false) {}

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
        return *this;
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
    
    uint8_t  lock;    

  private:
    uint32_t dataSize;
    uint32_t dataAllocated;
    uint8_t * data;
};

