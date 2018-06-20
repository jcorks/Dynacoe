

class QTree {
  public:    
    const int bucket_size_c = 1;
        
    class QTreeBox {
      public:
        QTreeBox(const Dynacoe::Vector & a) {
            centerLen = a;
        }
        
        bool ContainsPoint(const Vector & pt) const {
            return (pt.x >= centerLen.x - centerLen.z && pt.x < centerLen.x + centerLen.z) &&
                   (pt.y >= centerLen.y - centerLen.z && pt.y < centerLen.y + centerLen.z);
        }
        
        
        bool Overlaps(const QTreeBox & other) const {
            return ContainsPoint(other.centerLen.x - other.centerLen.z) ||
                   ContainsPoint(other.centerLen.x + other.centerLen.z) ||
                   ContainsPoint(other.centerLen.y - other.centerLen.z) ||
                   ContainsPoint(other.centerLen.y + other.centerLen.z);
        }
        
        bool Overlaps(const Dynacoe::Vector & q0,
                      const Dynacoe::Vector & q1
                      const Dynacoe::Vector & q2
                      const Dynacoe::Vector & q3) {
              
        }
        Dynacoe::Vector centerLen;

    };

    QTreeBox(const QTreeBox & bound) {
        Q1 = nullptr;
        Q2 = nullptr;
        Q3 = nullptr;
        Q4 = nullptr;
        
        size = 0;
    }
    QTree * Q1;
    QTree * Q2;
    QTree * Q3;
    QTree * Q4;


    QTreeBox bounds;
    Object2D * reference[bucket_size_c];
    uint32_t size;
    
    bool InsertPoint(const Dynacoe::Vector & position, Object2D * reference) {
        if (!ContainsPoint(position)) return false;
        
        if (size < bucket_size_c) {
            reference[size++];
        }
        
        if (!Q1) {
            Q1 = new QTree({bounds.centerLen.x-bounds.centerLen.z/2, bounds.centerLen.y-bounds.centerLen.z/2, bounds.centerLen.z/2});
            Q2 = new QTree({bounds.centerLen.x+bounds.centerLen.z/2, bounds.centerLen.y-bounds.centerLen.z/2, bounds.centerLen.z/2});
            Q3 = new QTree({bounds.centerLen.x+bounds.centerLen.z/2, bounds.centerLen.y+bounds.centerLen.z/2, bounds.centerLen.z/2});
            Q4 = new QTree({bounds.centerLen.x-bounds.centerLen.z/2, bounds.centerLen.y+bounds.centerLen.z/2, bounds.centerLen.z/2});                
        }
        
        if (Q1->insert(p)) return true;
        if (Q2->insert(p)) return true;
        if (Q3->insert(p)) return true;
        if (Q4->insert(p)) return true;

        return false;
    }
    
    bool InsertPoint(const Dynacoe::Vector & position, Object2D * reference) {
        if (!ContainsPoint(position)) return false;
        
        if (size < bucket_size_c) {
            reference[size++];
        }
        
        if (!Q1) {
            Q1 = new QTree({bounds.centerLen.x-bounds.centerLen.z/2, bounds.centerLen.y-bounds.centerLen.z/2, bounds.centerLen.z/2});
            Q2 = new QTree({bounds.centerLen.x+bounds.centerLen.z/2, bounds.centerLen.y-bounds.centerLen.z/2, bounds.centerLen.z/2});
            Q3 = new QTree({bounds.centerLen.x+bounds.centerLen.z/2, bounds.centerLen.y+bounds.centerLen.z/2, bounds.centerLen.z/2});
            Q4 = new QTree({bounds.centerLen.x-bounds.centerLen.z/2, bounds.centerLen.y+bounds.centerLen.z/2, bounds.centerLen.z/2});                
        }
        
        if (Q1->insert(p)) return true;
        if (Q2->insert(p)) return true;
        if (Q3->insert(p)) return true;
        if (Q4->insert(p)) return true;

        return false;
    }
    
    void GetOverlapped(const QTreeBox & bounds, std::vector<Object2D*> & hits) {
        
        
    }
};