/*

+-----+-----+
|  II |  I  |
|     |     |
+-----+-----+
| III |  IV |
|     |     |
+-----------+




*/

class QTree {
  public:    
    static const int bucket_size_c = 4;
    static const int bucket_minimum_span = 4;

    
    
    struct QTreeData {
        QTreeData(){}
        QTreeData(Object2D * obj_, const BoundingBox & bounds_) {
            object = obj_;
            bounds = bounds_;
        }
        Object2D * object;
        BoundingBox bounds; // smear bounds likely    
    };

    QTree(const BoundingBox & bound_) {
        Q1 = nullptr;
        Q2 = nullptr;
        Q3 = nullptr;
        Q4 = nullptr;
        
        size = 0;
        bounds = bound_;
    }
    
    ~QTree() {
        if (Q1) {
            delete Q1;
            delete Q2;
            delete Q3;
            delete Q4;            
        }        
    }
    
    QTree * Q1;
    QTree * Q2;
    QTree * Q3;
    QTree * Q4;


    BoundingBox bounds;
    std::vector<QTreeData> reference;
    uint32_t size;
    
    void Insert(const QTreeData & ref) {
        // if already subdivided
        if (Q1) {
            // get fitting index
            QTree * best = GetBestIndex(ref.bounds);

            // insert into proper child (pass it along)
            best->Insert(ref);
            // return 
            return;
        }
            
        // add as child 
        reference.push_back(ref);
        
        // if weve reached maximum child cound
        if (reference.size() >= bucket_size_c &&
            bounds.width > bucket_minimum_span && 
            bounds.height > bucket_minimum_span) {
            Subdivide();
        }

    }
    
    
    void QueryBound(const BoundingBox & other, std::vector<Object2D *> & objects) {
        if (!bounds.Overlaps(other)) return;
                
        // if subdivided
        if (Q1) {
            // get best quadrant 
            QTree * best = GetBestIndex(other);
            // if the quary is completely within the index
            if (best->bounds.ContainsBox(other)) {
                // add quadrants children            
                best->QueryBound(other, objects);
            } else {
                // we have an overlapping reqion. bleh 
                // for each quadrant that the region touches
                // query subregion
                Q1->QueryBound(other, objects);
                Q2->QueryBound(other, objects);
                Q3->QueryBound(other, objects);
                Q4->QueryBound(other, objects);                
            }
        } else {
            // ADD THIS NODES' objects
            for(uint32_t i = 0; i < reference.size(); ++i) {
                objects.push_back(
                    reference[i].object
                );
            }
        }
    }
    
  private:
    // Use the topleft point of the bounds
    // to determine which quadrant is best for 
    // the given box
    QTree * GetBestIndex(const BoundingBox & box) const {
        int left  = box.x > bounds.x + bounds.width/2.f;
        int top   = box.y > bounds.y + bounds.height/2.f ? 2 : 0;

        int which = (top & 0b10) | (left & 0b01);
        QTree * quadrants[] = {
            Q4,
            Q3,
            Q1,
            Q2
        };
        return quadrants[which];
    }
    
    
    // creates the 4 quadrants and donates the children to those 
    // new quadrants
    void Subdivide() {
        Q1 = new QTree(BoundingBox(bounds.x+bounds.width/2, bounds.y, bounds.width/2, bounds.height/2));
        Q2 = new QTree(BoundingBox(bounds.x               , bounds.y, bounds.width/2, bounds.height/2));
        Q3 = new QTree(BoundingBox(bounds.x               , bounds.y + bounds.height/2, bounds.width/2, bounds.height/2));
        Q4 = new QTree(BoundingBox(bounds.x+bounds.width/2, bounds.y + bounds.height/2, bounds.width/2, bounds.height/2));

        uint32_t oldSize = reference.size();

        
        for(uint32_t i = 0; i < oldSize; ++i) {
            Insert(reference[i]);
        }
        reference.clear();
        

    }
    

};
