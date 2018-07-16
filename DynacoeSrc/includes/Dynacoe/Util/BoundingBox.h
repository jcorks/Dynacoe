#ifndef H_DYNACOE_BOUNDING_BOX
#define H_DYNACOE_BOUNDING_BOX


namespace Dynacoe {
class BoundingBox {
  public:
    BoundingBox() :
        x(0.f),
        y(0.f),
        width(0.f),
        height(0.f){}
    BoundingBox(float x_, float y_, float w_, float h_) :
        x(x_),
        y(y_),
        width(w_),
        height(h_)
    {}
        
        
    bool ContainsPoint(const Dynacoe::Vector & a) const {
        return (a.x >= x &&
                a.x <= x+width &&
                a.y >= y &&
                a.y <= y+width);
    }
    
    bool ContainsBox(const BoundingBox & a) const {
        return (a.x >= x &&
                a.x+a.width <= x+width &&
                a.y >= y &&
                a.y+a.height <= y+width);        
    }
    
    
    bool Overlaps(const BoundingBox & other) const {
        return 
            x < other.x+other.width  && x+width > other.x &&
            y+other.height > other.y && y < other.y+other.height;
        
    }
    
    
    float x;
    float y;
    float width;
    float height;
};

}


#endif