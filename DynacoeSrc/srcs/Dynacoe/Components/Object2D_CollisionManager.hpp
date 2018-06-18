
class CollisionManager : public Dynacoe::Entity {
  public:
      
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

            objects[i]->UpdateTransition(
                 host->node.global.position,
                 before+objects[i]->GetNextPosition()
            )
        }
        
        
        
        
        
        
        
        for(uint32_t i = 0; i < objects.size(); ++i) {
            objects[i]->Update();
        }
    }
  private:
    std::vector<Object2D*> objects;
}