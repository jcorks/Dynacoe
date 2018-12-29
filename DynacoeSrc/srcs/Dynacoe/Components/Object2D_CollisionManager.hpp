#include "Object2D_QTree.hpp"
#include "Object2D_SpatialMap.hpp"
#include "Object2D_CollisionGroup.hpp"

#include <cfloat>
#include <cassert>
#include <cstring>

static std::vector<uint32_t> setObjs;

class CollisionManager : public Dynacoe::Entity {
  public:
    QTree * tree;
    SpatialMap * map;
    CollisionManager() {
        SetName("Object2D::CollisionManager");
        tree = nullptr;
        map = nullptr;
    }

      
    void RegisterObject2D(Object2D * o) {
        objects.push_back(o);
    }
    
    void UnregisterObject2D(Object2D * o) {
        for(uint32_t i = 0; i < objects.size(); ++i) {
            if (objects[i] == o) {
                objects.erase(objects.begin()+i);
                return;
            }
        }
    }
    
    void OnStep() {
        uint32_t numObj = objects.size();
        if (!numObj) return;
        
        
        // update before/after
        Dynacoe::Vector before, after;
        Entity * host;
        for(uint32_t i = 0; i < numObj; ++i) {
            host = objects[i]->GetHost();
            if (!host) {
                objects.erase(objects.begin()+i);
                numObj--;
                i--;
                continue;
            }

            objects[i]->collider.UpdateTransition(
                 objects[i]->GetNextPosition()
            );
        }
        

        ////////////////////////////////
        // grouping static spatial indexing
        ////////////////////////////////

        CollisionGroup * current;
        CollisionGroup * other;

        float spaceX  =  FLT_MAX, spaceX2  = -FLT_MAX,
              spaceY  =  FLT_MAX, spaceY2  = -FLT_MAX;
        float x, y, w, h;
        for(uint32_t i = 0; i < numObj; ++i) {
            auto b = objects[i]->collider.GetMomentBounds();

            if (b.x < spaceX) spaceX = b.x;
            if (b.y < spaceY) spaceY = b.y;
            if (b.x+b.width > spaceX2) spaceX2 = b.x+b.width;
            if (b.y+b.height > spaceY2) spaceY2 = b.y+b.height;
        }
        float spaceW = spaceX2 - spaceX;
        float spaceH = spaceY2 - spaceY;





        for(uint32_t x = 0; x < (int)(Object2D::Group::ID_Z)+1; ++x) {
            current = groups[x];
            current->ReEvaluate(spaceX, spaceY, spaceW, spaceH);
        }


        for(uint32_t x = 0; x < (int)(Object2D::Group::ID_Z)+1; ++x) {
            current = groups[x];
            for(uint32_t y = x; y < (int)(Object2D::Group::ID_Z)+1; ++y) {
                other = groups[y];
                if (!groupInteract[x+y*((int)(Object2D::Group::ID_Z)+1)]) continue;
                
                current->CollideWith(*other);
            }
        }




        /*
        ////////////////////////////////
        // uniform spatial indexing method
        ////////////////////////////////
        
        // setup tracking of who collided with who this iteration.
        // tracked with just an n^2 byte flag
        static uint8_t * collided = nullptr;
        static uint32_t collidedSize = 0;
        if (!collided) {
            collidedSize = 1024;
            collided = new uint8_t[collidedSize];
        }
        
        
        if (collidedSize <= numObj*numObj) {
            delete[] collided;
            collidedSize = numObj*numObj;
            collided = new uint8_t[collidedSize];
        }
        memset(collided, 0, collidedSize);
        
        
        
        float spaceX  =  FLT_MAX, spaceX2  = -FLT_MAX,
              spaceY  =  FLT_MAX, spaceY2  = -FLT_MAX;
        float x, y, w, h;
        for(uint32_t i = 0; i < numObj; ++i) {
            auto b = objects[i]->collider.GetMomentBounds();

            if (b.x < spaceX) spaceX = b.x;
            if (b.y < spaceY) spaceY = b.y;
            if (b.x+b.width > spaceX2) spaceX2 = b.x+b.width;
            if (b.y+b.height > spaceY2) spaceY2 = b.y+b.height;
        }
        float spaceW = spaceX2 - spaceX;
        float spaceH = spaceY2 - spaceY;


        if (map) {
            delete map;
            map = nullptr;
        }
        // no viable collision detection can occur, so just update the objects and drop out.
        if (spaceW == 0.f || spaceH == 0.f) {
            for(uint32_t i = 0; i < numObj; ++i) {
                objects[i]->Update();
            }
            return;
        }
        map = new SpatialMap(
            spaceX,
            spaceY,
            spaceW,
            spaceH,
            numObj
        );
        
        
        
        for(uint32_t i = 0; i < numObj; ++i) {
            map->Insert(objects[i]->collider.GetMomentBounds(), i);
        }
        
        
        
        Object2D * current;
        Object2D * other;
        uint32_t count = 0;
        uint32_t countFalse = 0;


        // list of visited this iteration
        static uint8_t * shortlist = nullptr;
        static uint32_t shortListSize = 1024;
        if (!shortlist) {
            shortlist = new uint8_t[1024];            
        }
        
        if (shortListSize <= numObj) {
            shortListSize = numObj;
            delete[] shortlist;
            shortlist = new uint8_t[numObj];
        }
        memset(shortlist, 0, numObj);

        for(uint32_t i = 0; i < numObj; ++i) {
            // if we didnt use it last iter, dont bother clearing it out!
            if (setObjs.size()) {
                memset(shortlist, 0, numObj);
                setObjs.clear();
            }
            
            // get all collisions
            map->QueryFast(objects[i]->collider.GetMomentBounds(), shortlist, setObjs);            
            current = objects[i];
            

            // process each detected collision
            for(uint32_t n = 0; n != setObjs.size(); ++n) {                
                other = objects[setObjs[n]];
                
                if (other == current) {
                    continue;
                }

                // no repeats! If we're already collided, skip very overlap check
                if (collided[i*numObj + setObjs[n]]) continue;
                collided[i*numObj + setObjs[n]] = true;                


                if (!current->collider.GetMomentBounds().Overlaps(other->collider.GetMomentBounds())) {
                    continue;
                }

                if (current->collider.CollidesWith(other->collider)) {               
                    current->EmitEvent("on-collide", other  ->GetHostID(), {});
                      other->EmitEvent("on-collide", current->GetHostID(), {});
                    current->collider.lastCollided = other  ->GetHostID();
                      other->collider.lastCollided = current->GetHostID();
                } else {
                    countFalse++;
                }
                count++;

            }
        }
        */

        /*
        ////////////////////////////////
        // QTree method
        ////////////////////////////////
        // first, get working bounds
        float spaceX  =  FLT_MAX, spaceY  =  FLT_MAX,
              spaceX2 = -FLT_MAX, spaceY2 = -FLT_MAX;
        float x, y, w, h;
        for(uint32_t i = 0; i < objects.size(); ++i) {
            objects[i]->collider.GetMomentBounds(
                x, y, w, h
            );

            if (x < spaceX) spaceX = x;
            if (y < spaceY) spaceY = y;
            if (x+w > spaceX2) spaceX2 = x+w;
            if (y+h > spaceY2) spaceY2 = y+h;
        }

          
        
        // remove existing tree and replace it with the new bounding space
        if (tree) delete tree;
        tree = new QTree(
            QTree::QTreeBox(
                spaceX,
                spaceY,
                spaceX2 - spaceX,
                spaceY2 - spaceY
            )
        );
        
        
        // insert all objects into the tree

        for(uint32_t i = 0; i < objects.size(); ++i) {
            objects[i]->collider.GetMomentBounds(
                x, y, w, h
            );
            tree->Insert(
                {
                    objects[i], 
                    {
                        x, y, w, h
                    }
                }
            );
        }


        // retrieve all possible-intersecting obejcts for each obejct
        Object2D * current;
        Object2D * other;
        std::vector<Object2D *> hits;
        uint32_t count = 0;
        for(uint32_t i = 0; i < objects.size(); ++i) {
            objects[i]->collider.GetMomentBounds(
                x, y, w, h
            );
            tree->QueryBound(
                {
                    x, y, w, h
                },
                hits
            );
            
            for(uint32_t n = 0; n < hits.size(); ++n) {
                if (hits[n] == objects[i]) {
                    count--;
                    continue;
                }
                other = hits[n];
                current = objects[i];
                if (current->collider.CollidesWith(other->collider)) {               
                    current->EmitEvent("on-collide", other  ->GetHostID(), {});
                      other->EmitEvent("on-collide", current->GetHostID(), {});
                    current->collider.lastCollided = other  ->GetHostID();
                      other->collider.lastCollided = current->GetHostID();
                }
            }
            count += hits.size();
            hits.clear();
        }
        std::cout << count / (float)(objects.size()*objects.size()) << "\n";
        */
        
        
        
        /*  
        //////////////////////////////////////
        // basic n^2
        //////////////////////////////////////
        std::vector<Object2D*> space = objects;
        Object2D * current;
        Object2D * other;
        for(uint32_t n = 0; n < space.size(); ++n) {
            current = space[n];
            for(uint32_t i = n+1; i < space.size(); ++i) {
                other = space[i];
 
                if (current->collider.CollidesWith(other->collider)) {               
                    current->EmitEvent("on-collide", other  ->GetHostID(), {});
                      other->EmitEvent("on-collide", current->GetHostID(), {});
                    current->collider.lastCollided = other  ->GetHostID();
                      other->collider.lastCollided = current->GetHostID();
                }
            }
        }
        */
        
        // apply new positions
        for(uint32_t i = 0; i < numObj; ++i) {
            objects[i]->Update();
        }
    }
  private:
    std::vector<Object2D*> objects;
};
