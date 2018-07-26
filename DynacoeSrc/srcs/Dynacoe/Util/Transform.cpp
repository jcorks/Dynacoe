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


#include <Dynacoe/Util/Transform.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Mesh.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <cstring>
#include <cassert>



/*
    Notes on implementation:


    Aspects never vertex lists, index lists, nor materials. They are either static (primitive
    meshes)  or given references from Mesh assets

 */

using namespace Dynacoe;



Transform::Transform() {
    reverse = false;
    scale = {1, 1, 1};
    needsUpdate = true;
}







Transform::~Transform() {
    uint32_t i = 0; 
    for(; i < callbacks.size(); ++i) {
        if (callbacks[i]->OwnedByTransform()) {
            delete callbacks[i];
        }
    }

}




const Vector & Transform::GetRotation() const {
    return rotation;
}

const Vector & Transform::GetPosition() const {
    return position;
}

const Vector & Transform::GetScale() const {
    return scale;
}


Vector & Transform::Rotation() {
    needsUpdate = true;
    return rotation;
}

Vector & Transform::Position() {
    needsUpdate = true;
    return position;
}

Vector & Transform::Scale() {
    needsUpdate = true;
    return scale;
}

void Transform::SetReverseTranslation(bool d) {
    reverse = d;
    needsUpdate = true;
}


const TransformMatrix & Transform::GetMatrix() {
    if (needsUpdate) {
        ComputeLocal();
        
        for(uint32_t i = 0; i < callbacks.size(); ++i) {
            (*callbacks[i])(this);
        }
    }
    return localTransform;
}

bool Transform::NeedsUpdate() const {
    return needsUpdate;
}


void Transform::AddTransformCallback(OnTransformUpdate * cb) {
    for(uint32_t i = 0; i < callbacks.size(); ++i) {
        if (callbacks[i] == cb) return;
    }
    callbacks.push_back(cb);
}

void Transform::RemoveTransformCallback(OnTransformUpdate * cb) {
    for(uint32_t i = 0; i < callbacks.size(); ++i) {
        if (callbacks[i] == cb) {
            callbacks.erase(callbacks.begin() + i);
            return;
        }
    }
}




void Transform::ComputeLocal() {
    localTransform.SetToIdentity();


    if (position.x != 0.f ||
        position.y != 0.f ||
        position.z != 0.f) {
        localTransform.Translate(reverse ? -1*position.x : position.x,
                                 reverse ? -1*position.y : position.y,
                                 reverse ? -1*position.z : position.z);
    }

    if (rotation.x != 0.f ||
        rotation.y != 0.f ||
        rotation.z != 0.f) {
        localTransform.RotateByAngles(
            rotation.x,
            rotation.y,
            rotation.z
        );
    }


    if (scale.x != 1.f ||
        scale.y != 1.f ||
        scale.z != 1.f) {
        localTransform.Scale(scale.x,
                             scale.y,
                             scale.z);


    }
    needsUpdate = false;

}





