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


#include <Dynacoe/Spatial.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Mesh.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <cstring>
#include <cassert>



using namespace Dynacoe;

class Dynacoe::SpatialTransformUpdate : public Transform::OnTransformUpdate {
public:
    Spatial * parent;
    SpatialTransformUpdate(Spatial * p) {
        parent = p;
    }
    void operator()(Transform * source) {
        parent->Invalidate();
    }
    
    bool OwnedByTransform() {return false;}
};

Spatial::Spatial() : node(new Transform) {
    transformOwned = node;
	transformUpdate = new SpatialTransformUpdate(this);
    transformOwned->AddTransformCallback(transformUpdate);
    needsUpdate = true;
    parent = nullptr;
}

void Spatial::Invalidate() {
    needsUpdate = true;
}

Spatial::~Spatial() {
    SetAsParent(nullptr);
    delete transformOwned;
	delete transformUpdate;
}

TransformMatrix & Spatial::GetGlobalTransform() {
    CheckUpdate();
    return global;
}



void Spatial::ReplaceTransform(Transform * t) {
	node->RemoveTransformCallback(transformUpdate);
    if (!t) {
        node = transformOwned;
    } else {
        node = t;
    }
    node->AddTransformCallback(transformUpdate);
}


void Spatial::SetAsParent(Spatial * newParent) {
    if (parent) {
        int64_t ct = parent->children.size();
        for(uint64_t i = 0; i < ct; ++i) {
            if (parent->children[i] == this) {
                parent->children.erase(parent->children.begin() + i);
                break;
            }
        }
        parent = nullptr;
    }
    
    
    
    if (!newParent) return;
    parent = newParent;
    assert(newParent != this);
    newParent->children.push_back(this);
    needsUpdate = true;
    
    //look for parent 
    Spatial * p = parent->parent;
    while(p) {
        p = p->parent;
    }
}


void Spatial::UpdateModelTransforms(RenderBufferID modelTransform) {
    TransformMatrix normalTransform = Graphics::GetCamera3D().GetGlobalTransform() * GetGlobalTransform();
    normalTransform.Inverse();
    //normalTransform.Transpose();
    //normalTransform.ReverseMajority();

    TransformMatrix m = GetGlobalTransform();
    m.ReverseMajority();
    Graphics::GetRenderer()->UpdateBuffer(modelTransform, m.GetData(), 0, 16);
    Graphics::GetRenderer()->UpdateBuffer(modelTransform, normalTransform.GetData(), 16, 16);
}




void Spatial::SendUpdateSignal() {
    std::stack<Spatial*> stack;
    
    Spatial * current;
    stack.push(this);
    while(!stack.empty()) {
        current = stack.top();
        stack.pop();
        current->needsUpdate = true;
        uint32_t ct = current->children.size();
        for(uint32_t i = 0; i < ct; ++i) {
            stack.push(current->children[i]);
        }    
    }
}

void Spatial::CheckUpdate() {
    if (node->NeedsUpdate()) needsUpdate = true;
    if (!needsUpdate) return;
    if (parent) {
        global = parent->GetGlobalTransform() * node->GetMatrix();
    } else {
        global = node->GetMatrix();
    }
    SendUpdateSignal();

    needsUpdate = false;
    OnUpdateTransform();
}


