#include <algorithm>
#include <cfloat>

#define UNIT_LENGTH 512
#define RAW_X_TO_INDEX_X(__X__) (int)(((__X__-globalX)/globalW)*UNIT_LENGTH)
#define RAW_Y_TO_INDEX_Y(__Y__) (int)(((__Y__-globalY)/globalH)*UNIT_LENGTH*UNIT_LENGTH)

class CollisionGroupMap {
  public:
    void Reset() {
        xSearch.Reset();
        ySearch.Reset();
    }
    void Add(int x, int y, int w, int h, Object2D * data) {
        xSearch.Insert(x, x+w, data);
        ySearch.Insert(y, y+h, data);
    }

    std::vector<Object2D*> & Query(int x, int y, int w, int h) {
        // The result we care about is just an intersection of X and Y
        // given that each datum is unique
        auto xResult = xSearch.Query(x, x+w);
        auto yResult = ySearch.Query(y, y+h);
        outputQuery.resize(xResult.size() + yResult.size());

        auto end = set_intersection(
            xResult.begin(), xResult.end(),
            yResult.begin(), yResult.end(),
            outputQuery.begin()
        );

        outputQuery.resize(end - outputQuery.begin());
        return outputQuery;
    }
  private:
    class OneDimMap {
      public:
        void Reset()  {
            for(uint32_t i = 0; i < UNIT_LENGTH; ++i) {
                objects[i].clear();
            }
        }
        // assumes less
        void Insert(int x1, int x2, Object2D * data) {
            for(int i = x1; i <= x2; ++i) {
                objects[i].push_back(data);
            }           
        }

        // assumes less
        // TODO: extrememly redundant data, potential for opt
        std::set<Object2D*> Query(int x1, int x2)  {
            outputQuery.clear();
            uint32_t count;
            Object2D ** iter;
            for(int i = x1; i <= x2; ++i) {
                iter = &(objects[i][0]);
                count =  objects[i].size();
                for(uint32_t n = 0; n < count; ++n) {
                    outputQuery.insert(*iter); iter++;
                }
            }
            return outputQuery;
        }
      private:

        std::vector<Object2D*> objects[1024];
        std::set<Object2D*> outputQuery;

    }; 

    OneDimMap xSearch;
    OneDimMap ySearch;
    std::vector<Object2D*> outputQuery;
  
};


class Dynacoe::CollisionGroup {
  public:
    CollisionGroup(int id_) {
        id = id_;
    }

    void ReEvaluate(float globalX_, float globalY_, float globalW_, float globalH_) {
        globalX  = globalX_;
        globalY  = globalY_;
        globalW = globalW_;
        globalH = globalH_;

        minX = FLT_MAX;
        minY = FLT_MAX;
        maxX = -FLT_MAX;
        maxY = -FLT_MAX;
        
        // re-eval local bounds
        Object2D * object;
        int x1, x2;
        int y1, y2;

        spaceMap.Reset();

        for(uint32_t i = 0; i < objectCount; ++i) {
            auto bounds = objects[i]->collider.GetMomentBounds();

            spaceMap.Add(
                RAW_X_TO_INDEX_X(bounds.x),
                RAW_Y_TO_INDEX_Y(bounds.y),
                RAW_X_TO_INDEX_X(bounds.x+bounds.width),
                RAW_Y_TO_INDEX_Y(bounds.y+bounds.height),
                objects[i]
            );


            if (bounds.x < minX) minX = bounds.x;
            if (bounds.y < minY) minY = bounds.y;
            if (bounds.x+bounds.width  > minX) minX = bounds.x+bounds.width;
            if (bounds.y+bounds.height > minY) minY = bounds.y+bounds.height;
        }

        spanX = maxX-minX;
        spanY = maxY-minY;

        

    }


    void CollideWith(CollisionGroup & otherObj) {

        // if the bounding box of the collision group doesnt intersect with ours
        // then dont bother
        if (!(
                otherObj.minX >= minX &&
                otherObj.maxX <= maxX &&
                otherObj.minY >= minY &&
                otherObj.maxY <= maxY
            )) {

            return; // process no collisions
        }

        uint32_t countPotential;
        Object2D * current;
        Object2D * other;
        for(uint32_t i = 0; i < objectCount; ++i) {
            auto bounds = objects[i]->collider.GetMomentBounds();
            current = objects[i];

            auto results = otherObj.spaceMap.Query(
                RAW_X_TO_INDEX_X(bounds.x),
                RAW_Y_TO_INDEX_Y(bounds.y),
                RAW_X_TO_INDEX_X(bounds.x+bounds.width),
                RAW_Y_TO_INDEX_Y(bounds.y+bounds.height)
            );

            // there are potential collisions
            countPotential = results.size();
            for(uint32_t i = 0; i < countPotential; ++i) {

                other = results[i];
                if (other == current) continue;             


                if (!current->collider.GetMomentBounds().Overlaps(other->collider.GetMomentBounds())) {
                    continue;
                }

                if (current->collider.CollidesWith(other->collider)) {               
                    current->EmitEvent("on-collide", other  ->GetHostID(), {});
                    current->collider.lastCollided = other  ->GetHostID();
                }
            }
        }

        
    }

    void Register(Object2D * object) {
        objects.push_back(object);
        objectCount++;
    }

    void Unregister(Object2D * object) {
        for(uint32_t i = 0; i < objectCount; ++i) {
            if (objects[i] == object) {
                objects.erase(objects.begin()+i);
                objectCount--;
                return;
            }
        }
    }

    int GetID() const {return id;}

    

  private:
    int id;
    float minX;
    float minY;
    float maxX;
    float maxY;
    float spanX;
    float spanY; 

    float globalX;
    float globalY;
    float globalW;
    float globalH;

    std::vector<Object2D *> objects;
    uint8_t * collided;
    uint32_t objectCount;
    CollisionGroupMap spaceMap;
};



// global collision maps
static CollisionGroup * groups[((int)Object2D::Group::ID_Z)+1] = {
    nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr
};


// global collision interactiongroup
static uint8_t groupInteract[(((int)Object2D::Group::ID_Z)+1)*(((int)Object2D::Group::ID_Z)+1)];

