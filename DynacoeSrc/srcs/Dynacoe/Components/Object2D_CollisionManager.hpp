
class CollisionManager : public Dynacoe::Entity {
  public:
    CollisionManager() {
        SetName("Object2D::CollisionManager");
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
        // update before/after
        Dynacoe::Vector before, after;
        Entity * host;
        for(uint32_t i = 0; i < objects.size(); ++i) {
            host = objects[i]->GetHost();
            if (!host) {
                objects.erase(objects.begin()+i);
            }

            objects[i]->collider.UpdateTransition(
                 host->node.global.position,
                 before+objects[i]->GetNextPosition()
            );
            objects[i]->collider.lastCollided = Entity::ID();
        }
        
        
        
        // pruning?




        // basic n^2 to start off
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
        
        
        // apply new positions
        for(uint32_t i = 0; i < objects.size(); ++i) {
            objects[i]->Update();
        }
    }
  private:
    std::vector<Object2D*> objects;
};
