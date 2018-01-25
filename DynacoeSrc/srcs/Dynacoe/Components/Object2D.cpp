/*

Copyright (c) 2018, Johnathan Corkery. (jcorkery@umich.edu)
All rights reserved.

This file is part of the Dynacoe project (https://github.com/jcorks/Dynacoe)
Dynacoe was released under the MIT License, as detailed below.



Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished 
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.



*/

#include <Dynacoe/Components/Object2D.h>


#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Entity.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Components/Node.h>

#include <cmath>
#include <set>
#include <map>





using namespace Dynacoe;
using namespace std;

//void Object2D::NullCollision(Entity::ID, Entity::ID) {}
DynacoeEvent(Object2D::NullCollision) {}

// private class that manages all collisions
class Dynacoe::CollisionManager : public Entity {
  public:
    CollisionManager() : Entity() {
        SetName("Object2DManager");
    }


    // generates a group ID to determine what processing
    // group an Object2D belongs to
    // TODO: need actual pruning. Here it just groups based on world alone, 
    // which is an inherent stipulation Perhaps just a simple proximity divition based on a "large" scale?
    Entity::ID GenerateGroupID(Object2D * obj) {

        // Here, the group criterion can be further refined
        // to minimize groups
        if (obj->GetHost() && obj->GetHost()->HasParent() && obj->colliders.size()) {
            return obj->GetHost()->GetParent().GetID();
        }
        return Entity::ID();
    }



    // Processes a collision
    void ProcessCollision(Object2D * a, Object2D * b) {
        if (a == b) return;
        if (!a->collisionActive) return;
        Entity * aHost = a->GetHost();
        Entity * bHost = b->GetHost();
        // TODO: collision groups
        
        if (!aHost || !bHost) return;
        
        Node * aNode = &aHost->node;
        Node * bNode = &bHost->node;

        // perhaps we could still allow this depending?
        if (!aNode || !bNode) return;

        
        Entity * aWorld = &aHost->GetParent();
        Entity * bWorld = &bHost->GetParent();

        if (!aWorld || !bWorld) return;
        

        // simple, last-minute pruning:
        // if both colliders are byond each's collider's ranges, leave.
        // TODO: / global
        float dist = aNode->global.position
           .Distance(bNode->global.position);
        if (dist > a->greatestColliderSpan && dist > b->greatestColliderSpan) return;

        for(int n = 0; n < a->colliders.size(); ++n) {
            for(int j = 0; j < b->colliders.size(); ++j) {

                // worldly checks...
                
                if (&aHost->GetParent() != &bHost->GetParent() || !aHost->HasParent()) {
                    return;
                }

                
                
                if (Object2D::isCollided(a->colliders[n], b->colliders[j],
                                         aNode->global.position, bNode->global.position)
                               ||
                    a->willCollide(b)
                               ||
                    b->willCollide(a))
                               {
                     a->isCollidedState = true;
                     b->isCollidedState = true;
                     a->EmitEvent("on-collide", bHost->GetID(), {});
                     b->EmitEvent("on-collide", aHost->GetID(), {});
                    //(a->colliders[n]->callback)(aHost->GetID(), bHost->GetID());
                    //(b->colliders[n]->callback)(bHost->GetID(), aHost->GetID());

                }
            }
        }
    }    



    void AddObject(Object2D * obj) {
        objs.insert(obj);
    }

    void RemoveObject(Object2D * obj) {
        auto iter = objs.find(obj);
        if (iter != objs.end())
            objs.erase(iter);
    }

    void OnPreStep() {
        // first group based on world (this will also filter out
        //  un-attached object2Ds which arent updated)
        std::map<Entity::ID, std::vector<Object2D *>> objsGrouped;
        
        Object2D * obj;
        auto setIter = objs.begin();
        while(setIter != objs.end()) {
            obj = *setIter++;
            Entity::ID groupID = GenerateGroupID(obj);
            if (groupID.Valid()) objsGrouped[groupID].push_back(obj);
        }

        // now that we only have valid objects
        // process all collisions for each group.

        std::vector<Object2D*> * group;
        Object2D * thisCollider;
        Object2D * otherCollider;
        auto groupIter = objsGrouped.begin();
        while(groupIter != objsGrouped.end()) {
            group = &groupIter->second;
            for(int n = 0; n < group->size(); ++n) {
                thisCollider = (*group)[n];
                thisCollider->isCollidedState = false;
                for(int k = 0; k < group->size(); ++k) {
                    otherCollider = (*group)[k];
                    ProcessCollision(thisCollider, otherCollider);
                }
            }
            groupIter++;
        }


        // finally, update all objects (that remain)
        setIter = objs.begin();
        while(setIter != objs.end()) {
            (*setIter++)->Update();
        }
    }

    void DrawColliders(const Color & color) {
        Shape2D colliderVisual;
        colliderVisual.color = color;
        Object2D * obj;
        Object2D::ContactPoly * poly;
        Vector offset;
        auto setIter = objs.begin();
        std::vector<Vector> iterPts;
        Node * hNode;

        // dump all collider points into a vector.
        // form triangles about the center
        while(setIter != objs.end()) {
            obj = *setIter++;
            if (!obj->GetHost()) return;
            hNode = &obj->GetHost()->node;

            offset = hNode->global.position;
            for(int i = 0; i < obj->colliders.size(); ++i) {
                poly = obj->colliders[i];            
                for(int n = 0; n < poly->numPts; ++n) {
                    iterPts.push_back(poly->pts[n]+offset);
                    if (n != poly->numPts -1) {
                        iterPts.push_back(poly->pts[n+1]+offset);
                    } else {
                        iterPts.push_back(poly->pts[0]+offset);                        
                    }
                    iterPts.push_back(offset);

                }
            }
            
        }
        
        colliderVisual.FormTriangles(iterPts);
        Graphics::Draw(colliderVisual);
        
    }

  private:
    std::set<Object2D*> objs;
};






CollisionManager *              Object2D::collisionManager;





   


Object2D::Object2D() : Component("Object2D") {
    speedX = 0.;
    speedY = 0.;
    frictionX = 0.;
    frictionY = 0.;
    greatestColliderSpan = 0;

    collisionActive = true;
    isCollidedState = false;
    
    InstallEvent("on-move");
    InstallEvent("on-collide");
}

Object2D::~Object2D() {
    collisionManager->RemoveObject(this);
    for(uint32_t i = 0; i < colliders.size(); ++i) {
        delete colliders[i];
    }
    // TODO: cleanup colliders
}

void Object2D::OnAttach() {
    if (!collisionManager) {
        collisionManager = Entity::Create<CollisionManager>().IdentifyAs<CollisionManager>();
        Engine::AttachManager(collisionManager->GetID());
    }
    collisionManager->AddObject(this);
}



void Object2D::OnStep() {

    // per-instance implementation
    /*
    runCollisions();

    Update();
    */

}

void Object2D::Update() {
    if (!GetHost()) return;
    if (!GetHost()->IsStepping()) return;
    Node * node = &GetHost()->node;
    if (!node) return;
    
    EmitEvent("on-move");
        
    node->local.position += GetNextPosition() - node->global.position;
    speedX *= (1.0 - frictionX);
    speedY *= (1.0 - frictionY);

}




void Object2D::AddVelocity(double factor, double direction) {
    speedX += factor * (cos((Math::Pi() / 180.f) * direction));
    speedY += factor * (sin((Math::Pi() / 180.f) * direction));
}

void Object2D::AddVelocityTowards(double factor, const Dynacoe::Vector & p) {
    if (!GetHost()) return;
    Node * n = &GetHost()->node;
    Vector delta;    
    Vector src = n->global.position;
    
    delta.x = p.x - src.x;
    delta.y = p.y - src.y;
    AddVelocity(factor, delta.RotationZ());    
}

void Object2D::SetVelocity(double factor, double direction) {
    speedX = factor * (cos((Math::Pi() / 180.f) * direction));
    speedY = factor * (sin((Math::Pi() / 180.f) * direction));
}

void Object2D::SetVelocityTowards(double factor, const Dynacoe::Vector & p) {
    if (!GetHost()) return;
    Node * n = &GetHost()->node;
    Vector delta;
    Vector src = n->global.position;
    delta.x = p.x - src.x;
    delta.y = p.y - src.y;
    SetVelocity(factor, delta.RotationZ()); 

}
void Object2D::SetFrictionX(double amt) {
    frictionX = amt;
}

void Object2D::SetFrictionY(double amt) {
    frictionY = amt;
}

double Object2D::GetDirection() {
    //return (180 / Math::Pi()) * atan(speedY / (speedX + .0000001));
    return Vector(speedX, speedY).RotationZ();
}

void Object2D::Halt() {
    speedX = 0;
    speedY = 0;
}



double Object2D::GetVelocityX() { return speedX; }
double Object2D::GetVelocityY() { return speedY; }

double Object2D::GetSpeed() { return Vector(speedX, speedY).Length(); }
void Object2D::SetSpeed(double speed) {
    Vector p(speedX, speedY);
    p.SetToNormalize();
    p.x *= speed;
    p.y *= speed;
    speedX = p.x;
    speedY = p.y;
}


Vector Object2D::GetNextPosition() {
    if (!GetHost()) return Vector();
    Node * n = &GetHost()->node;
    Vector newPos;
    
    Vector p = n->global.position;


    newPos(p.x + speedX*(1.0 - frictionX),
           p.y + speedY*(1.0 - frictionY));

    return newPos;
}


void Object2D::DrawColliders(const Color & c) {
   collisionManager->DrawColliders(c);
}




/* collisions */



Object2D::ContactID Object2D::AddContactBox(const Dynacoe::Vector & offset,
                           int w, int h) {

    vector<Vector> pts;
    pts.push_back(offset);
    pts.push_back(offset + Vector(w, 0.f));
    pts.push_back(offset + Vector(w, h));
    pts.push_back(offset + Vector(0.f, h));
    return AddContactPolygon(pts);
}

Object2D::ContactID Object2D::AddContactPolygon(const vector<Vector> & pts) {

    ContactPoly * c = new ContactPoly(pts.size());
    float greatestLocalSpan = 0.f;
    for(size_t i = 0; i < pts.size(); ++i) {
        c->pts[i] = pts[i];
        if (pts[i].Length()  > greatestLocalSpan)
            greatestLocalSpan = pts[i].Length();
    }

    if (greatestLocalSpan > greatestColliderSpan)
        greatestColliderSpan = greatestLocalSpan;

    colliders.push_back(c);
    ContactID id;
    id.index = colliders.size() - 1;
    return id;
}

// Aternate:
// Save all collisions in a list with distance
// execute callbacks ordered by distance
void Object2D::runCollisions() {
    if (!collisionActive || !GetHost()->HasParent()) return;
    Entity & world = GetHost()->GetParent();
    Node * node = &GetHost()->node;
    // Search through all entities
    auto worldChildren = world.GetChildren();
    for(int i = 0; i < worldChildren.size(); ++i) {
        Entity * e = worldChildren[i].Identify();
        if (e == GetHost() || &world != &e->GetParent() || &world != &GetHost()->GetParent()) continue;
        Object2D * otherCollider = (e->QueryComponent<Object2D>());
        Node * otherNode = &e->node;
        // if the entity has a collider...
        if (otherCollider) {
            // Check if any of the other collider's contact boxes hit any of our contact boxes
            // if so, run both callbacks

            for(int n = 0; n < colliders.size(); ++n) {
                for(int j = 0; j < otherCollider->colliders.size(); ++j) {
                    if (&world != &e->GetParent() || &world != &GetHost()->GetParent()) {
                        j = otherCollider->colliders.size();
                        n = colliders.size();
                        if (&world != &GetHost()->GetParent())
                            i = worldChildren.size();
                        continue;
                    }
                    if (isCollided(colliders[n], otherCollider->colliders[j],
                                   node->global.position, otherNode->global.position)
                                   ||
                        willCollide(otherCollider)
                                   ||
                        otherCollider->willCollide(this))
                                   {
                                       EmitEvent("on-collide", e->GetID(), {});
                        otherCollider->EmitEvent("on-collide", GetHostID(), {});
                        //(colliders[n]->               callback)(GetHost()->GetID(), e->GetID());
                        //(otherCollider->colliders[n]->callback)(e->GetID(), GetHost()->GetID());

                    }
                }
            }
        }
    }
}

bool Object2D::isCollided(const ContactPoly * c1, const ContactPoly * c2, const Vector & selfOrigin, const Vector & otherOrigin) {
    for(int i = 0; i < c1->numPts - 1; ++i) {
        for(int n = 0; n < c2->numPts - 1; ++n) {
            if (areIntersected(c1->pts[i] + selfOrigin,  c1->pts[i+1] + selfOrigin,
                               c2->pts[n] + otherOrigin, c2->pts[n+1] + otherOrigin)) {
                return true;
            }
        }
    }
    return false;
}

bool Object2D::willCollide(Object2D * other) {
    // Check for intersection of all lines L with
    // all contact polygons of this, where L is defined as:
    //      A line segment of 2 points where the first point
    //      is a ContactPolygon point of the other's Object2D instance
    //      offset by the other entity's position and the second point is
    //      that ContactPolygon point offset by its projected
    //      position based on its velocity

    // This form of collision detection is only one-way; it only
    // accounts for the other object motion compared to the current objects
    // stationary contact points.

    Node * otherNode = GetHostID().Query<Node>();

    ContactPoly * otherC;
    Vector now =  otherNode->global.position;
    Vector next = other->GetNextPosition();

    for(size_t i = 0; i < other->colliders.size(); ++i) {
        otherC = other->colliders[i];
        for(size_t p = 0; p < otherC->numPts; ++p) {
            if (IsLineCollided(otherC->pts[p] + now, otherC->pts[p] + next))
                return true;
        }
    }
    return false;
}


bool Object2D::IsVectorCollided(const Dynacoe::Vector & p) {
    /*
    Vector guess = p - GetHost()->pos;
    ContactBox b;
    for(int i = 0; i < colliders.size(); ++i) {
        b = colliders[i];
        if (Params::isWithinBounds(guess, b.offset,
                                      b.w, b.h)) {
            return true;
        }
    }
    */
    return false;
}


bool Object2D::IsLineCollided(const Vector & p1_, const Vector & p2_) {

    ContactPoly p(2);
    p.pts[0] = p1_;
    p.pts[1] = p2_;
    
    Node * n = GetHostID().Query<Node>();

    for(size_t i = 0; i < colliders.size(); ++i) {
        if (isCollided(colliders[i], &p, n->global.position, Vector()))
            return true;
    }
    return false;

}

bool Object2D::areIntersected(const Vector & p1, const Vector & p2,
                                  const Vector & p3, const Vector & p4) {

    // "LeMothe's Method"
    Vector s1 = p2 - p1;
    Vector s2 = p4 - p3;

    float s = (- s1.y * (p1.x - p3.x) + s1.x * (p1.y - p3.y)) / (-s2.x * s1.y + s1.x * s2.y);
    float t = (  s2.x * (p1.y - p3.y) - s2.y * (p1.x - p3.x)) / (-s2.x * s1.y + s1.x * s2.y);

    if (s >= 0 && s <= 1 && t>= 0 && t <= 1) {
        return 1;
    }
    return 0;
}



std::string Object2D::GetInfo() {
    Node * n = GetHostID().Query<Node>();
    return (Chain() << "Position :" << (n ? n->global.position : Vector()) << "\n" 
                    << "Speed    : (" << GetVelocityX() << ", " << GetVelocityY() << ") [" << GetSpeed() << "]\n"
                    << "Direction: " << GetDirection() << "\n"
                    << "Collided?: " << (isCollidedState ? "Yes" : "Nope") << "\n"
                    << "SeekCollisions? "<< (collisionActive ? "Yes" : "Nope"));

}
