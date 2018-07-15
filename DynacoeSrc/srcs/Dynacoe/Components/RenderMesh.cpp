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


#include <Dynacoe/Components/RenderMesh.h>


#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Mesh.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <Dynacoe/Components/Node.h>



/* 
    Notes on implementation:


    Aspects never vertex lists, index lists, nor materials. They are either static (primitive 
    meshes)  or given references from Mesh assets

 */

using namespace Dynacoe;

static bool arePrimitivesAllocd = false;








RenderMesh::RenderMesh() : RenderMesh(new Node){}

RenderMesh::RenderMesh(Node * nInst) : Component("RenderMesh"), node(*nInst){
    Alloc();
    initValues();
}



void RenderMesh::initValues() {
    prim = Renderer::Polygon::Triangle;
}

RenderMesh::~RenderMesh() {
    Clear();
    Graphics::GetRenderer()->RemoveBuffer(modelTransform);
}

RenderMesh & RenderMesh::operator=(const RenderMesh & other) {       
    Clear();
    meshes = other.meshes;
    mat = other.mat;
    framebufferID = other.framebufferID;
    prim = other.prim;

    // deep copies for temp meshes
    for(uint32_t i = 0; i < other.meshes.size(); ++i) {
        Mesh * newMesh = new Mesh();
        *newMesh = *other.meshes[i];
        meshes.push_back(newMesh);
    }
    
    float data[32];
    Graphics::GetRenderer()->ReadBuffer  (other.modelTransform, data, 0, 32);
    Graphics::GetRenderer()->UpdateBuffer(modelTransform,       data, 0, 32);
    
    
    
    
    return *this;
}

RenderMesh::RenderMesh(const RenderMesh & other, Node * n) : Component("RenderMesh"), node(n?*n:*new Node()){
    Alloc();
    initValues(); 
    *this = other;
}

void RenderMesh::Clear() {
    for(uint32_t i = 0; i < meshes.size(); ++i) {
        delete (meshes[i]);
    }
    meshes.clear();
}





void RenderMesh::AddMesh(const Mesh & m) {
    Mesh * newMesh = new Mesh();
    *newMesh = m;
    meshes.push_back(newMesh);
}

void RenderMesh::AddMesh(
    const std::vector<Vector> & vertexPositions,
    const std::vector<uint32_t> & faces,
    const std::vector<Vector> & UVs,
    const std::vector<Vector> & normals,
    const std::vector<Vector> & userData
) {
    Mesh & m = *(new Mesh());
    
    m.DefineVertices(Mesh::VertexAttribute::Position, vertexPositions);

    if (normals.size())  m.DefineVertices(Mesh::VertexAttribute::Normal, vertexPositions);
    if (UVs.size())      m.DefineVertices(Mesh::VertexAttribute::UV, vertexPositions);
    if (userData.size()) m.DefineVertices(Mesh::VertexAttribute::UserData, vertexPositions);
    
    Mesh::MeshObject * mObj = m.Get(m.AddObject());
    mObj->faceList = faces;
    meshes.push_back(&m);
}

Mesh & RenderMesh::GetMesh(uint32_t i) {
    static Mesh error;
    if (i >= meshes.size()) return error;
    return *meshes[i];
}



Dynacoe::Material & RenderMesh::Material() {
    return mat;
}





// called right before Drawing
void RenderMesh::OnTransformUpdate() {
    
}

// TODO: remove the need for graphics-specific logic!
void RenderMesh::OnDraw() {
    Entity * src = GetHost();
    if (src) {
        node.UpdateModelTransforms(modelTransform);
        Graphics::Draw(*this);
    }
}



std::string RenderMesh::GetInfo() {
    Chain info;
    for(uint32_t i = 0; i < meshes.size(); ++i) {
        info << "Mesh " << i;
        for(uint32_t n = 0; n < meshes[i]->NumObjects(); ++n) {
            Mesh::MeshObject * m = meshes[i]->Get(n);
            info << "(" << m->faceList.size() << " faces)";
        }
        info << "\n";
    }
    
    info << mat.Info() << "\n";
    return info;
}

void RenderMesh::RenderSelf(Renderer * renderer) {
    StaticState state;
    state.samplebuffer = nullptr;
    state.modelData = modelTransform;
    
    Camera * c = nullptr;

    
    mat.PopulateState(&state);
    
    for(uint32_t i = 0; i < meshes.size(); ++i) {        
        Mesh * m = meshes[i];
        for(uint32_t n = 0; n < m->NumObjects(); ++n) {
            if (m->NumVertices() && m->NumObjects()) {
                m->PopulateState(&state, n);
                renderer->RenderStatic(&state);
            }
        }
    }
    
}

void RenderMesh::Alloc() {

    // set the Context Data. Context data includes the 
    // local transform of the mesh and the material information.

    TransformMatrix temp;
    temp.SetToIdentity();
    modelTransform = Graphics::GetRenderer()->AddBuffer(nullptr, 32);
    
    OnTransformUpdate();
}




void RenderMesh::SetRenderPrimitive(Renderer::Polygon p) {
    prim = p;
}

Renderer::Polygon RenderMesh::GetRenderPrimitive() {
    return prim;
}


void RenderMesh::OnAttach() {
    node.SetManualParent(GetHost()->QueryComponent<Node>());
}

void RenderMesh::OnStep() {
    node.Step();
}