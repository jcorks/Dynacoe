
#include <cstring>

class SpatialMap {
    
    // replacement for std::vector<uint32> tailored for:
    // - reuse (lots of new/deletes -> use GetReserved/Store)
    // - Lots of inserts
    // - lots of size and data query
    class FastVec {
      private:
        int alloc;
      public:
        static FastVec ** reserve;
        static int reserveAmt;
        static int reserveAlloc;

        // preallocated with 16;
        FastVec() : alloc(16), size(0){
            data = (uint32_t*)malloc(sizeof(uint32_t)*16);
        }

        // gets an existing vector for use, faster than new'ing
        static FastVec * GetReserved() {
            FastVec * out;           
            if (reserveAmt) {
                out = reserve[reserveAmt-1];
                out->size = 0;
                reserveAmt--;
            } else {
                out = new FastVec();
            }
            return out;
        }

        // stores vector for use as another instance. More efficient than delete'ing
        void Store() {
            if (reserveAmt == reserveAlloc) {
                FastVec ** reserveNew = (FastVec**)malloc(sizeof(FastVec*)*(reserveAlloc+1024));    
                if (reserve) {
                    memcpy(reserveNew, reserve, reserveAlloc*sizeof(FastVec*));
                    free(reserve);
                }
                reserve = reserveNew; 
                reserveAlloc += 1024;
                
            }
            reserve[reserveAmt++] = this;
        }

        // not really used
        ~FastVec() {
            free(data);
        }

    
        // adds a new member        
        void push_back(uint32_t d) {
            data[size++] = d;
            if (size >= alloc) {
                uint32_t * newData = (uint32_t*)malloc(alloc*2*sizeof(uint32_t));
                memcpy(newData, data, sizeof(uint32_t)*alloc);
                free(data);
                data = newData;
                alloc = alloc*2;
            }
        }

        // public data and size
        uint32_t * data;
        int size;
        
        
    };
  public:

    
    SpatialMap(float spanX_, float spanY_, float spanW_, float spanH_, uint32_t numObjects) {
        spanX = spanX_;
        spanY = spanY_;
        spanW = spanW_;
        spanH = spanH_;

        span = GetClosestEnclosingSpan(numObjects);
        field = new FastVec**[span];
        for(uint32_t i = 0; i < span; ++i) {
            field[i] = new FastVec*[span];
            for(uint32_t n = 0; n < span; ++n) {
                field[i][n] = nullptr;
            }
        }
    }
    
    ~SpatialMap() {
        for(uint32_t i = 0; i < span; ++i) {
            for(uint32_t n = 0; n < span; ++n) {
                if (field[i][n]) field[i][n]->Store();
            }
            delete[] field[i];
        }
        delete[] field;
    }
    
    void Insert(const BoundingBox & region, uint32_t index) {
        XYRange range = GetCoveredRegions(region);
        
        for(int y = range.minY; y <= range.maxY; ++y) {
            for(int x = range.minX; x <= range.maxX; ++x) {
                if (!field[x][y])
                    field[x][y] = FastVec::GetReserved();
                    
                field[x][y]->push_back(index);
            }
        }
    }
    
    void Query(const BoundingBox & region, std::unordered_map<uint32_t, bool> & indicesHit) {
        XYRange range = GetCoveredRegions(region);
        uint32_t len;
        for(int y = range.minY; y <= range.maxY; ++y) {
            for(int x = range.minX; x <= range.maxX; ++x) {
                FastVec * obj = field[x][y];
                len = obj->size;
                if (len) {
                    for(uint32_t n = 0; n < len; ++n) {
                        indicesHit[(obj->data[n])] = true;
                    }
                }
            }
        }
        
        
    }
    
    void QueryFast(const BoundingBox & region, uint8_t * visited, std::vector<uint32_t> & ids) {
        XYRange range = GetCoveredRegions(region);
        uint32_t len;
        for(int y = range.minY; y <= range.maxY; ++y) {
            for(int x = range.minX; x <= range.maxX; ++x) {
                FastVec * obj = field[x][y];
                len = obj->size;
                for(uint32_t n = 0; n < len; ++n) {
                    if (visited[(obj->data[n])]) continue;
                    ids.push_back((obj->data[n]));
                    visited[(obj->data[n])] = true;
                }
            }
        }
        
        
    }
    
  private:
    struct XYPair {
        int x;
        int y;
    };
    
    struct XYRange {
        int minX;
        int maxX;
        int minY;
        int maxY;
    };
      
    int GetClosestEnclosingSpan(uint32_t count) {
        return ceil(sqrt(count))*4;
    }
    
    XYRange GetCoveredRegions(const BoundingBox & cover) const {
        XYRange range;
        range.minX   = XPositionToXIndex(cover.x);
        range.maxX   = XPositionToXIndex(cover.x+cover.width);
        range.minY   = YPositionToYIndex(cover.y);
        range.maxY   = YPositionToYIndex(cover.y+cover.height);
        return range;
    }
    
    int XPositionToXIndex(float x) const {
        float fracX = (x - spanX) / spanW;
        if (fracX < 0.f) fracX = 0.f;
        if (fracX > 1.f) fracX = 1.f;

        return (int)(fracX * (span-1));
    }

    int YPositionToYIndex(float y) const {
        float fracY = (y - spanY) / spanH;
        if (fracY < 0.f) fracY = 0.f;
        if (fracY > 1.f) fracY = 1.f;

        return (int)(fracY * (span-1));
    }



    
    float spanX;
    float spanY;
    float spanW;
    float spanH;
      
    int span;
    FastVec *** field;
    
};

SpatialMap::FastVec** SpatialMap::FastVec::reserve = nullptr;
int SpatialMap::FastVec::reserveAmt = 0;
int SpatialMap::FastVec::reserveAlloc = 0;

