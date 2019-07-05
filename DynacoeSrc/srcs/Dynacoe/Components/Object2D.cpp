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

#include <cmath>
#include <set>
#include <map>





using namespace Dynacoe;
using namespace std;


#include "Object2D_CollisionManager.hpp"
#include "Object2D_Collider.hpp"

static Entity::ID manager;
   




Object2D::Object2D() : Component() {
    speedX = 0.;
    speedY = 0.;
    frictionX = 0.;
    frictionY = 0.;

    SetTag("Object2D");

    InstallEvent("on-move");
    InstallEvent("on-collide");
    InstallEvent("on-moved");

    static bool inited = false;
    if (!inited) {
        // default: all groups collide with each other
        memset(
            groupInteract, 
            1, 
            (((int)Object2D::Group::ID_Z)+1)*(((int)Object2D::Group::ID_Z)+1)
        );
        inited = true;
    }


    group = nullptr;
    SetGroup(Group::ID_A);
}

Object2D::~Object2D() {
    manager.IdentifyAs<CollisionManager>()->UnregisterObject2D(this);
    group->Unregister(this);
    
}

void Object2D::OnAttach() {
    if (!manager.Valid()) {
        manager =  Entity::Create<CollisionManager>();
        Engine::AttachManager(manager, true);
    }
    manager.IdentifyAs<CollisionManager>()->RegisterObject2D(this);
}



void Object2D::Update() {
    if (!GetHost()->IsStepping()) return;
    Transform * node = &GetHost()->Node();
    

    Vector delta = GetNextPosition() - GetHost()->GetGlobalTransform().Transform({});
    // using the "last" model, we include manual translations as part of 
    // normal collisions.
    if (delta.Length() > .000001 && EmitEvent("on-move")) {
        node->Position() += delta;
        GetHost()->CheckUpdate();
    }
    last = node->GetPosition();

    speedX *= (1.0 - frictionX);
    speedY *= (1.0 - frictionY);
    

}




void Object2D::AddVelocity(float factor, float direction) {
    speedX += factor * (cos((Math::Pi() / 180.f) * direction));
    speedY += factor * (sin((Math::Pi() / 180.f) * direction));
}

void Object2D::AddVelocityTowards(float factor, const Dynacoe::Vector & p, float directionOffset) {
    Transform * n = &GetHost()->Node();
    Vector delta;    
    Vector src = GetHost()->GetGlobalTransform().Transform({});
    
    delta.x = p.x - src.x;
    delta.y = p.y - src.y;
    AddVelocity(factor, delta.RotationZ() + directionOffset);    
}


void Object2D::SetVelocity(float factor, float direction) {
    speedX = factor * (cos((Math::Pi() / 180.f) * direction));
    speedY = factor * (sin((Math::Pi() / 180.f) * direction));
}

void Object2D::SetVelocityTowards(float factor, const Dynacoe::Vector & p, float directionOffset) {
    Vector delta;
    Vector src = GetHost()->GetGlobalTransform().Transform({});
    delta.x = p.x - src.x;
    delta.y = p.y - src.y;
    SetVelocity(factor, delta.RotationZ()+directionOffset); 

}


void Object2D::SetFrictionX(float amt) {
    frictionX = amt;
}

void Object2D::SetFrictionY(float amt) {
    frictionY = amt;
}

float Object2D::GetDirection() {
    return Vector(speedX, speedY).RotationZ();
}

void Object2D::Halt() {
    speedX = 0;
    speedY = 0;
}

void Object2D::ResetMotion() {
    GetHost()->CheckUpdate();
    last = GetHost()->Node().GetPosition();
    collider.UpdateTransition(last);
    collider.UpdateTransition(last);
    
}



float Object2D::GetVelocityX() const { return speedX; }
float Object2D::GetVelocityY() const { return speedY; }

void Object2D::SetVelocityX(float x) { speedX = x; }
void Object2D::SetVelocityY(float y) { speedY = y; }

float Object2D::GetSpeed() { return Vector(speedX, speedY).Length(); }
void Object2D::SetSpeed(float speed) {
    Vector p(speedX, speedY);
    p.SetToNormalize();
    p.x *= speed;
    p.y *= speed;
    speedX = p.x;
    speedY = p.y;
}


Vector Object2D::GetNextPosition() {
    Vector newPos;
    
    Vector p = GetHost()->GetGlobalTransform().Transform({});


    newPos(p.x + speedX*(1.0 - frictionX),
           p.y + speedY*(1.0 - frictionY));

    return newPos;
}

const Vector & Object2D::GetLastPosition() const {
    return last; 
} 


void Object2D::SetGroup(Group id) {

    if (!groups[(int)id]) {
        groups[(int)id] = new CollisionGroup((int)id);
    
    }
    
    if (group) group->Unregister(this);
    group = groups[(int)id];
    group->Register(this);
}

Object2D::Group Object2D::GetGroup() const {
    return (Object2D::Group)group->GetID();
}

void Object2D::EnableGroupInteraction(Object2D::Group a, Object2D::Group b) {
    groupInteract[((int)a) + (((int)b)*(((int)Object2D::Group::ID_Z)+1))] = true;
    groupInteract[((int)b) + (((int)a)*(((int)Object2D::Group::ID_Z)+1))] = true;
}

void Object2D::DisableGroupInteraction(Object2D::Group a, Object2D::Group b) {
    groupInteract[((int)a) + (((int)b)*(((int)Object2D::Group::ID_Z)+1))] = false;
    groupInteract[((int)b) + (((int)a)*(((int)Object2D::Group::ID_Z)+1))] = false;
}




std::string Object2D::GetInfo() {
    return (Chain() << "Position :" << GetHost()->GetGlobalTransform().Transform({}) << "\n" 
                    << "Speed    : (" << GetVelocityX() << ", " << GetVelocityY() << ") [" << GetSpeed() << "]\n"
                    << "Direction: " << GetDirection() << "\n"
                    << "Collided : " << 
                    (collider.lastCollided.Valid() ?
                        Chain() << "(" << collider.lastCollided.Identify()->GetName() << ") - " << collider.lastCollided.String() << "\n"
                    :
                        Chain() << "\n")
            );

}
