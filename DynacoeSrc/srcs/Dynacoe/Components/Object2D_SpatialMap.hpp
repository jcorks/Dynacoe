

class SpatialMap {
  public:

    
    SpatialMap(float spanX_, float spanY_, float spanW_, float spanH_, uint32_t numObjects) {
        spanX = spanX_;
        spanY = spanY_;
        spanW = spanW_;
        spanH = spanH_;

        span = GetClosestEnclosingSpan(numObjects);
        field = new std::vector<uint32_t>**[span];
        for(uint32_t i = 0; i < span; ++i) {
            field[i] = new std::vector<uint32_t>*[span];
            for(uint32_t n = 0; n < span; ++n) {
                field[i][n] = nullptr;
            }
        }
    }
    
    ~SpatialMap() {
        for(uint32_t i = 0; i < span; ++i) {
            for(uint32_t n = 0; n < span; ++n) {
                if (field[i][n]) delete field[i][n];
            }
            delete[] field[i];
        }
        delete[] field;
    }
    
    void Insert(const BoundingBox & region, uint32_t index) {
        XYRange range = GetCoveredRegions(region);
        
        for(uint32_t y = range.minY; y <= range.maxY; ++y) {
            for(uint32_t x = range.minX; x <= range.maxX; ++x) {
                if (!field[x][y])
                    field[x][y] = new std::vector<uint32_t>();
                    
                field[x][y]->push_back(index);
            }
        }
    }
    
    void Query(const BoundingBox & region, std::unordered_map<uint32_t, bool> & indicesHit) {
        XYRange range = GetCoveredRegions(region);
        uint32_t len;
        for(uint32_t y = range.minY; y <= range.maxY; ++y) {
            for(uint32_t x = range.minX; x <= range.maxX; ++x) {
                std::vector<uint32_t> * obj = field[x][y];
                len = obj->size();
                if (len) {
                    for(uint32_t n = 0; n < len; ++n) {
                        indicesHit[((*obj)[n])] = true;
                    }
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
    std::vector<uint32_t> *** field;
    
};