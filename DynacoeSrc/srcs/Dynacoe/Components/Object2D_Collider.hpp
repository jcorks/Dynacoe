Object2D::Collider::Line::Line(const Dynacoe::Vector & a_, const Dynacoe::Vector & b_) {
    a = a_;
    b = b_;
}

bool Object2D::Collider::Line::Intersects(const Line & other) const {
    // "LeMothe's Method"
    Vector s1 = b - a;
    Vector s2 = other.b - other.a;

    float s = (- s1.y * (a.x - other.a.x) + s1.x * (a.y - other.a.y)) / (-s2.x * s1.y + s1.x * s2.y);
    float t = (  s2.x * (a.y - other.a.y) - s2.y * (a.x - other.a.x)) / (-s2.x * s1.y + s1.x * s2.y);

    if (s >= 0 && s <= 1 && t>= 0 && t <= 1) {
        return 1;
    }
    return 0;
}





Object2D::Collider::Collider(const std:vector<Dynacoe::Vector> & pts) {
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
}




Object2D::Collider::Collider(float radius, uint32_t numPts) {
    std::vector<Dynacoe::Vector> pts(numPts);
    double RADIAN_MAX = M_PI*2;
    for(uint32_t i = 0; i < numPts; ++i) {
        pts[i].x = cos(i / RADIAN_MAX) * radius;
        pts[i].y = sin(i / RADIAN_MAX) * radius;
    }
    
    Collider(pts);
}


Object2D::Collider::Collider(const std:vector<const Object2D::Collider &> & colliders) {
    std::vector<Dynacoe::Vector> pts;
    uint32_t totalSize = 0;
    for(uint32_t i = colliders.size(); ++i) {
        totalSize += colliders[i].size();
    }
    pts.resize(totalSize);
    
    totalSize = 0;
    for(uint32_t i = colliders.size(); ++i) {
        memcpy(
            &pts[totalSize], 
            &colliders.staticPoints[0], 
            colliders.staticPoints.size()*sizeof(Collider::Line)
        );
        totalSize += colliders[i].size();
    }
    
    Collider(pts);
}



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

std::vector<Object2D::Collider::Line> Object2D::Collider::UpdateTransition(const Dynacoe::Vector & before, const Dynacoe::Vector & after) {
    out.resize(staticPoints.size()*3);
    for(uint32_t i = 0; i < staticPoints.size(); ++i) {
        out.push_back({
            staticPoints[i].a + before,
            staticPoints[i].b + before
        });

        out.push_back({
            staticPoints[i].a + after,
            staticPoints[i].b + after
        });
        
        out.push_back({
            staticPoints[i].a + before,
            staticPoints[i].a + after,
        })
    }
    
    return out;
}


bool Collider::CollidesWith(const Collider & other, const Dynacoe::Vector & before, const Dynacoe::Vector & after) const {
    auto thisSmear  =       GetSmear(before, after);
    auto otherSmear = other.GetSmear(before, after);
    
    for(uint32_t i = 0; i < thisSmear.size(); ++i) {
        for(uint32_t n = 0; n < otherSmear.size(); ++i) {
            if (thisSmear[i].Intersects(otherSmear[n])) {
                return true;
            }          
        }
    }
    return false;
}
