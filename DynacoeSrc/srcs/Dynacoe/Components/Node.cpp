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


#include <Dynacoe/Components/Node.h>
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

static Table<Node*> * instance = nullptr;




Node::Node() : Component("Node") {
    if (!instance) instance = new Table<Node*>();
    id = instance->Insert(this);
    localTransformValid = false;
    parentTransformValid = false;
    parentNode = nullptr;
    overrideParent = nullptr;
    manualChildren = nullptr;
}


std::string Node::GetInfo() {
    Chain info;
    info = Chain() << "Local:\n"
                   << "position :" << local.position << "\n"
                   << "scale    :" << local.scale << "\n"
                   << "rotation :" << local.rotation << "\n\n"
                   << "Global: \n"
                   << "position :" << global.position << "\n"
                   << "scale    :" << global.scale << "\n"
                   << "rotation :" << global.rotation << "\n\n";

    return info;

}




Node::~Node() {
    if (overrideParent) {
        assert(overrideParent->manualChildren);
        overrideParent->manualChildren->erase(overrideParent->manualChildren->find(this));
    }

    instance->Remove(id);
    if (manualChildren) {
        delete manualChildren;
    }
}





//// family functions






const uint8_t * Node::Transform::GetState() {
    return (const uint8_t *)&position;
}

TransformMatrix Node::Transform::Compute() {
    TransformMatrix localTransform;


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
    return localTransform;
}





// grouping Aspects: if not volatile, shallow copy of vertex reference id
// else, deep copy (get buffer data, copy it into new buffer). The only time
// a RenderObject is volatile is if the sourceAspect's vertex Data
// was formed by Define()










void Node::updateTransform() {


    // check local transform based on user-given public attribute
    localTransformValid = transformLastState == local;


    if (localTransformValid && parentTransformValid) {
        return;
    }



    if (!localTransformValid) {
        transformLastState = local;
        localTransformValid = true;
        localTransform = local.Compute();
    }


    if (!parentTransformValid) {
        // Instead of going directly to the host, we
        // keep track of a "parentNode" reference to allow for non-entities to
        // to keep track of nodes and act as leaves in nodal hierarchies
        UpdateParentReference();
    }




    // since one of them changed, we need to update the global transform
    if (parentNode) {
        TransformMatrix parent = parentNode->globalTransform;
        globalTransform = parent * localTransform;
    } else {
        globalTransform = localTransform;
    }
    Entity * host = GetHost();



    global.position = globalTransform.Transform({0, 0, 0});

    Vector rotationBaseZ = Vector(1, 0, 0);

    rotationBaseZ = globalTransform.Transform(rotationBaseZ);
    Vector rotationBaseY = Vector(0, 0, 1);
    rotationBaseY = globalTransform.Transform(rotationBaseY);

    Vector rotationBaseX = Vector(0, 1, 0);
    rotationBaseX = globalTransform.Transform(rotationBaseX);

    rotationBaseX = rotationBaseX - global.position;
    rotationBaseY = rotationBaseY - global.position;
    rotationBaseZ = rotationBaseZ - global.position;

    global.rotation.x = rotationBaseX.RotationX();
    global.rotation.y = rotationBaseY.RotationY();
    global.rotation.z = rotationBaseZ.RotationZ();







    // update all children since the parent changed.
    Node * asp;
    //globalTransform.ReverseMajority();
    if (host) {
        auto children = host->GetChildren();
        for(uint32_t i = 0; i < children.size(); ++i) {
            asp = children[i].Identify()->QueryComponent<Node>();
            if (!asp) continue;
            asp->parentTransformValid = false;
            asp->updateTransform();
        }
    }


    if (manualChildren) {
        for(auto i = manualChildren->begin(); i != manualChildren->end(); ++i) {
            asp = (*i);
            asp->parentTransformValid = false;
            asp->updateTransform();
        }
    }


    parentTransformValid = true;
    localTransformValid = true;
    OnTransformUpdate();

}

void Node::UpdateParentReference() {
    if (!overrideParent) {
        Entity * host = GetHost();
        if (host && host->HasParent()) {
            parentNode = host->GetParent().QueryComponent<Node>();
        }
    } else {
        parentNode = overrideParent;
    }
}

const TransformMatrix & Node::GetGlobalTransform() {
    return globalTransform;
}


void Node::UpdateModelTransforms(RenderBufferID modelTransform) {
    normalTransform = Graphics::GetCamera3D().QueryComponent<Node>()->GetGlobalTransform() * globalTransform;
    normalTransform.Inverse();
    //normalTransform.Transpose();
    //normalTransform.ReverseMajority();

    TransformMatrix m = globalTransform;
    m.ReverseMajority();
    Graphics::GetRenderer()->UpdateBuffer(modelTransform, m.GetData(), 0, 16);
    Graphics::GetRenderer()->UpdateBuffer(modelTransform, normalTransform.GetData(), 16, 16);
}

void Node::OnStep() {
    updateTransform();
}

void Node::SetManualParent(Node * node) {
    if (overrideParent) {
        assert(overrideParent->manualChildren);
        overrideParent->manualChildren->erase(overrideParent->manualChildren->find(this));
    }
    overrideParent = node;
    if (!overrideParent->manualChildren) {
        overrideParent->manualChildren = new std::set<Node*>();
    }
    node->manualChildren->insert(this);
}

void Node::OnAttach() {
    parentTransformValid = false;
    updateTransform();
}
