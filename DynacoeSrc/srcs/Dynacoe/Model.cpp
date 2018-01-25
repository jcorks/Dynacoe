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


#include <Dynacoe/Model.h>
#include <Dynacoe/Components/RenderMesh.h>
#include <Dynacoe/Material.h>
#include <Dynacoe/Components/Node.h>

using namespace Dynacoe;

Entity::ID Model::Create() {
    Entity * out = Entity::CreateReference<Entity>();
    out->SetName(GetAssetName());
    for(uint32_t i = 0; i < meshes.size(); ++i) {
        Entity * ent = Entity::CreateReference<Entity>();
        ent->SetName(Chain() << "model-node-" << i);
        RenderMesh * copyMesh = new RenderMesh();        
        copyMesh->AddMesh(meshes[i]->MakeShallowCopy());
        copyMesh->Material() = *materials[i];
        ent->AddComponent(copyMesh);
        out->Attach(ent->GetID());
    }
    return out->GetID();
}

Model::~Model() {
    for(uint32_t i = 0; i < meshes.size(); ++i) {
        delete meshes[i];
        delete materials[i];
    }
    
}


void Model::AddSection() {
    Mesh * newM = new Mesh();
    Material * newMat = new Material();
    meshes.push_back(newM);
    materials.push_back(newMat);
}


uint32_t Model::GetSectionCount() const {
    return meshes.size();
}

Mesh & Model::SectionMesh(uint32_t i) {
    static Mesh error;
    if (i >= GetSectionCount())
        return error;
        
    return *meshes[i];
}


Material & Model::SectionMaterial(uint32_t i) {
    static Material error;
    if (i >= GetSectionCount())
        return error;
        
    return *materials[i];
}

