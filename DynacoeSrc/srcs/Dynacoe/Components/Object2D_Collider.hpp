
Object2D::Collider::Line::Line(const Dynacoe::Vector & a_, const Dynacoe::Vector & b_) {
    a = a_;
    b = b_;
}


bool Object2D::Collider::Line::Intersects(const Line & other) const {
    // "LeMothe's Method"
    float s1_x = b.x - a.x;
    float s1_y = b.y - a.y;
    float s1_z = b.z - a.z;
    
    
    float s2_x = other.b.x - other.a.x;
    float s2_y = other.b.y - other.a.y;
    float s2_z = other.b.z - other.a.z;

    float s = (- s1_y * (a.x - other.a.x) + s1_x * (a.y - other.a.y)) / (-s2_x * s1_y + s1_x * s2_y);
    float t = (  s2_x * (a.y - other.a.y) - s2_y * (a.x - other.a.x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t>= 0 && t <= 1) {
        return 1;
    }
    return 0;
}

bool Object2D::Collider::WillContainPoint(const Dynacoe::Vector & p) const {
    bool temp = false;
    uint32_t realStart = stationary ? 0 : smear.size()/3;
    uint32_t realCount = stationary ? smear.size() : realStart*2;
    uint32_t i, n;

    struct int2{
        int2(){};
        int2(const Dynacoe::Vector & a) {
            x = round(a.x);
            y = round(a.y);
        }
        int x;
        int y;
    };
    
    int2 thisP;
    int2 nextP;

    int x = round(p.x);
    int y = round(p.y);

    bool in = false;
    // Jordan curve theorem, based on PNPOLY (WRF)
    for(i = realStart, n = realCount - 1; i < realCount; n = i++) { // flow control for wrapping behavior. i is current poly, n is next
        thisP = int2(smear[i].a);
        nextP = int2(smear[n].a);

        if (
                ((thisP.y > y) != 
                 (nextP.y > y))            
            &&
            
                (x < (nextP.x - thisP.x) * (y - thisP.y) / (nextP.y - thisP.y) + thisP.x)
            
            ) {
            in = !in;
        }
    }
    return in;
}





Object2D::Collider::Collider(const std::vector<Dynacoe::Vector> & pts) {
    SetFromPoints(pts);
}


Object2D::Collider::Collider(float radius, uint32_t numPts) {
    std::vector<Dynacoe::Vector> inpts(numPts);
    double RADIAN_MAX = 3.14159265359*2;
    for(uint32_t i = 0; i < numPts; ++i) {
        inpts[i].x = cos(i / RADIAN_MAX) * radius;
        inpts[i].y = sin(i / RADIAN_MAX) * radius;
    }
    
    SetFromPoints(inpts);
}


Object2D::Collider::Collider(const std::vector<Object2D::Collider> & colliders) {
    std::vector<Dynacoe::Vector> pts;    
    for(uint32_t i = 0; i < colliders.size(); ++i) {
        uint32_t localSize = colliders[i].staticPoints.size();
        if (!localSize) continue;

        for(uint32_t n = 0; n < localSize; ++n) {
            pts.push_back(colliders[i].staticPoints[n].a);
        }
        pts.push_back(colliders[i].staticPoints[0].a);
    }
    
    SetFromPoints(pts);
}


void Object2D::Collider::SetFromPoints(const std::vector<Dynacoe::Vector> & pts) {
    if (!pts.size() || pts.size() == 1) return;

    for(uint32_t i = 0; i < pts.size()-1; ++i) {
        staticPoints.push_back(
            {pts[i], pts[i+1]}
        );
    }
    staticPoints.push_back({pts[pts.size()-1], pts[0]});

    float minX, minY, maxX, maxY;
    minX = pts[0].x;
    maxX = pts[0].x;
    minY = pts[0].y;
    maxY = pts[0].y;

    for(uint32_t i = 1; i < pts.size(); ++i) {
        if (minX > pts[i].x) minX = pts[i].x;
        if (maxX < pts[i].x) maxX = pts[i].x;
        if (minY > pts[i].y) minY = pts[i].y;
        if (maxY < pts[i].y) maxY = pts[i].y;
    }
    
    boundingBox = {
        {{minX, minY}, {maxX, minY}},
        {{maxX, minY}, {maxX, maxY}},
        {{maxX, maxY}, {minX, maxY}},
        {{minX, maxY}, {minX, minY}}    
    };
    
    stationary = false;
    isSet = false;
    
}



/*
std::vector<Object2D::Collider::Line> Object2D::Collider::GetSmearBoundingBox(const Dynacoe::Vector & before, const Dynacoe::Vector & after) const {
    float minX = before.x < after.x ? boundBox[0].x + before.x : boundBox[0].x + after.x;
    float maxX = before.x > after.x ? boundBox[1].x + before.x : boundBox[1].x + after.x;
    float minY = before.y < after.y ? boundBox[1].y + before.y : boundBox[1].y + after.y;
    float maxY = before.y > after.y ? boundBox[2].y + before.y : boundBox[2].y + after.y;
    return {
        {{minX, minY}, {maxX, minY}},
        {{maxX, minY}, {maxX, maxY}},
        {{maxX, maxY}, {minX, maxY}},
        {{minX, maxY}, {minX, minY}}        
    };
}
*/

void Object2D::Collider::UpdateTransition(const Dynacoe::Vector & after) {
    if (!staticPoints.size()) return;
    
    if (!isSet) {
        oldPosition = after;
        isSet = true;
    }
    Dynacoe::Vector before = oldPosition;
    oldPosition = after;
    
    if (before.Distance(after) < .000001) {
        // if we were already stationary, neither the smear bounds nor the static points have changed!
        if (stationary) {
            return;
        }

        smear.clear();
        // procs shorthand smear and bounds
        // only the "after" smears are taken
        // (the smears are also 1/3 the size when stationary since it doesnt include the
        // union of the movement smear and old polygon smear)
        for(uint32_t i = 0; i < staticPoints.size(); ++i) {
            smear.push_back({
                staticPoints[i].a + after,
                staticPoints[i].b + after
            });
        }
        
        smearBounds.x = smear[0].a.x;
        smearBounds.y = smear[0].a.y;
        float smearX2 = smearBounds.x;
        float smearY2 = smearBounds.y;
        Dynacoe::Vector afterWorld;
    
        for(uint32_t i = 0; i < smear.size(); ++i) {
            afterWorld  = smear[i].a;
            if (afterWorld.x < smearBounds.x) smearBounds.x = afterWorld.x;
            if (afterWorld.y < smearBounds.y) smearBounds.y = afterWorld.y;
            if (afterWorld.x > smearX2) smearX2 = afterWorld.x;
            if (afterWorld.y > smearY2) smearY2 = afterWorld.y;    
        }
        
        smearBounds.width = smearX2 - smearBounds.x;
        smearBounds.height = smearY2 - smearBounds.y;
        
        
        
        stationary = true;        
        return;
    } else {
        stationary = false;
        smear.clear();

    }
    
    for(uint32_t i = 0; i < staticPoints.size(); ++i) {
        smear.push_back({
            staticPoints[i].a + before,
            staticPoints[i].b + before
        });
    }

    for(uint32_t i = 0; i < staticPoints.size(); ++i) {
        smear.push_back({
            staticPoints[i].a + after,
            staticPoints[i].b + after
        });
    }

    smearBounds.x = smear[0].a.x;
    smearBounds.y = smear[0].a.y;
    float smearX2 = smearBounds.x;
    float smearY2 = smearBounds.y;
    Dynacoe::Vector beforeWorld;
    Dynacoe::Vector afterWorld;

    for(uint32_t i = 0; i < staticPoints.size(); ++i) {
        beforeWorld = staticPoints[i].a + before;
        afterWorld  = staticPoints[i].a + after;
        smear.push_back({
            beforeWorld,
            afterWorld,
        });
        if (beforeWorld.x < smearBounds.x) smearBounds.x = beforeWorld.x;
        if (beforeWorld.y < smearBounds.y) smearBounds.y = beforeWorld.y;
        if (afterWorld.x < smearBounds.x) smearBounds.x = afterWorld.x;
        if (afterWorld.y < smearBounds.y) smearBounds.y = afterWorld.y;

        if (beforeWorld.x > smearX2) smearX2 = beforeWorld.x;
        if (beforeWorld.y > smearY2) smearY2 = beforeWorld.y;
        if (afterWorld.x > smearX2) smearX2 = afterWorld.x;
        if (afterWorld.y > smearY2) smearY2 = afterWorld.y;

    }
    
    smearBounds.width = smearX2 - smearBounds.x;
    smearBounds.height = smearY2 - smearBounds.y;
    
    
    
}



// collision detection is counted under 2 criteria:
// - Velocty smear intersection
// - The new positional points are contained within the others new polygonal structure
//
bool Object2D::Collider::CollidesWith(const Object2D::Collider & other) const {
    
    
    
    for(uint32_t i = 0; i < smear.size(); ++i) {
        for(uint32_t n = 0; n < other.smear.size(); ++n) {
            if (smear[i].Intersects(other.smear[n])) {
                return true;
            }          
        }
    }
    
    uint32_t size = stationary ? smear.size() : (smear.size()/3)*2;
    for(uint32_t i = stationary ? 0 : smear.size()/3; i < size; ++i) {
        if (other.WillContainPoint(
            smear[i].a
        )) return true;
    }
    
    
    size = other.stationary ? other.smear.size() : (other.smear.size()/3)*2;
    for(uint32_t i = other.stationary ? 0 : other.smear.size()/3; i < size; ++i) {
        if (WillContainPoint(
            other.smear[i].a
        )) return true;
    }

    
    return false;
}
