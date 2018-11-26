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


#include <Dynacoe/Mesh.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Util/Iobuffer.h>
#include <Dynacoe/Modules/Console.h>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <cassert>







float quadPrimitive_positions[] = {
    -0.5f, -0.5f, 0.f,      0.f, 0.f, 1.f,      0.f, 0.f,   0.f, 0.f, 0.f, 0.f,
     0.5f, -0.5f, 0.f,      0.f, 0.f, 1.f,      1.f, 0.f,   0.f, 0.f, 0.f, 0.f,
     0.5f,  0.5f, 0.f,      0.f, 0.f, 1.f,      1.f, 1.f,   0.f, 0.f, 0.f, 0.f,
    -0.5f,  0.5f, 0.f,      0.f, 0.f, 1.f,      0.f, 1.f,   0.f, 0.f, 0.f, 0.f,
};

uint32_t quadPrimitive_indexList[] = {
    0, 1, 2,    0, 2, 3
};









float triPrimitive_positions[] = {
    -0.5f, -0.5f, 0.f,      0.f, 0.f, 1.f,      0.f, 0.f,   0.f, 0.f, 0.f, 0.f,
      0.f,  0.5f, 0.f,      0.f, 0.f, 1.f,      0.f, 1.f,   0.f, 0.f, 0.f, 0.f,
     0.5f, -0.5f, 0.f,      0.f, 0.f, 1.f,      1.f, 0.f,   0.f, 0.f, 0.f, 0.f,

};

uint32_t triPrimitive_indexList[] = {
    0, 2, 1
};





float pyramidPrimitive_positions[] = {
    -0.5f, -0.5f, -0.5f,      0.f, 0.f, 1.f,      0.f, 1.f,   0.f, 0.f, 0.f, 0.f,
     0.5f, -0.5f, -0.5f,      0.f, 0.f, 1.f,      1.f, 1.f,   0.f, 0.f, 0.f, 0.f,
     0.5f, -0.5f,  0.5f,      0.f, 0.f, 1.f,      1.f, 0.f,   0.f, 0.f, 0.f, 0.f,
    -0.5f, -0.5f,  0.5f,      0.f, 0.f, 1.f,      0.f, 1.f,   0.f, 0.f, 0.f, 0.f,
     0.0f,  0.5f,  0.0f,      0.f, 0.f, 1.f,      1.f, 0.f,   0.f, 0.f, 0.f, 0.f,
};


uint32_t pyramidPrimitive_indexList[] = {
    0, 1, 4,
    1, 2, 4,
    2, 3, 4,
    3, 0, 4,
    
    0, 1, 2,    0, 2, 3

};




using namespace Dynacoe;
using std::vector;
using Dynacoe::Vector;


// retrieves the offset into a vertex entry that the attribute exists 
// and the number of floats the attribute occupies.
static void VertexAttribSizes(Mesh::VertexAttribute attrib, uint8_t & offset, uint8_t & numFloats) {
    switch(attrib) {
        case Mesh::VertexAttribute::Position: offset = 0; numFloats = 3; break;
        case Mesh::VertexAttribute::Normal:   offset = 3; numFloats = 3; break;
        case Mesh::VertexAttribute::UV:       offset = 6; numFloats = 2; break;
        case Mesh::VertexAttribute::UserData: offset = 8; numFloats = 3; break;
    }

}



Mesh::~Mesh() {
    if (vertices.Valid() && !isShallow)
        Graphics::GetRenderer()->RemoveBuffer(vertices);
}

Mesh::Mesh() { numElts = 0; isShallow = false;}

Mesh::Mesh(const Mesh & other) {
    isShallow = false;
    *this = other;
}

Mesh & Mesh::operator=(const Mesh & other) {
    if (isShallow) {
        vertices = other.vertices;
    } else {
        if (vertices.Valid())
            Graphics::GetRenderer()->RemoveBuffer(vertices);
            
        if (other.vertices != RenderBufferID()) {
            vertices = Graphics::GetRenderer()->AddBuffer(nullptr, other.numElts*12);
            float * copyData = new float[other.numElts*12];
            Graphics::GetRenderer()->ReadBuffer(other.vertices, copyData, 0, other.numElts*12);
            Graphics::GetRenderer()->UpdateBuffer(vertices, copyData, 0, other.numElts*12);
            delete[] copyData;
        }
    }
    numElts = other.numElts;
    objs = other.objs;
    return *this;
}


void Mesh::SetVertexCount(uint32_t i) {
    numElts = i;
    Graphics::GetRenderer()->RemoveBuffer(vertices);
    vertices = Graphics::GetRenderer()->AddBuffer(nullptr, numElts*12);
}

bool Mesh::IsShallow() {
    return isShallow;
}

void Mesh::MakeUnique() {
    if (!isShallow) return;
    
    Mesh m = *this;
    isShallow = false;
    *this = m;
}


void Mesh::DefineVertices(VertexAttribute attrib, 
    const vector<Vector> & v) {
    if (!v.size()) return;


    if (numElts != v.size()) {
        Console::Warning()  << "[Mesh]: Definition input vertex list data count and the mesh's vertex count do not match. Ignoring definition."<< Console::End();
        return;
    }

    uint8_t offset, numFloats;
    VertexAttribSizes(attrib, offset, numFloats);
    for(uint32_t i = 0; i < numElts; ++i) {
        Graphics::GetRenderer()->UpdateBuffer(vertices, (float*)&v[i], i*12 + offset, numFloats);
    }
}

void Mesh::DefineVerticesState(const std::vector<Renderer::StaticVertex> & vt) {
    SetVertexCount(vt.size());
    Graphics::GetRenderer()->UpdateBuffer(vertices, (float*)&vt[0], 0, vt.size()*12);
}   

Vector Mesh::GetVertex(uint32_t index, VertexAttribute attrib) const{
    uint8_t offset, numFloats;
    if (index >= numElts) return Vector();
    VertexAttribSizes(attrib, offset, numFloats);
    Vector out;
    Graphics::GetRenderer()->ReadBuffer(vertices, (float*)&out, index*12 + offset, numFloats);
    return out;
}

void Mesh::SetVertex(uint32_t index, VertexAttribute attrib, const Vector & in) {
    uint8_t offset, numFloats;
    if (index >= numElts) return;
    VertexAttribSizes(attrib, offset, numFloats);
    Graphics::GetRenderer()->UpdateBuffer(vertices, (float*)&in, index*12 + offset, numFloats);
}




uint32_t Mesh::NumVertices() const {
    return numElts;
}








int Mesh::AddObject() {
    objs.push_back(MeshObject());
    return objs.size()-1;
}

int Mesh::AddObject(const Mesh::MeshObject & obj) {
    objs.push_back(obj);
    return objs.size()-1;
}

Mesh::MeshObject * Mesh::Get(int i) {
    if (i < 0 || i >= objs.size()) return nullptr;
    return &objs[i];
}

void Mesh::RemoveObject(int i) {
    if (i < 0 || i >= objs.size()) return;
    objs.erase(i + objs.begin());
}

int Mesh::NumObjects() {
    return objs.size();
}



void Mesh::PopulateState(StaticState * s, int i) {    
    s->vertices = vertices;
    if (i < 0 || i >= objs.size()) {
        s->indices = nullptr;
    }
    s->indices = &objs[i].faceList;
}


Mesh Mesh::MakeShallowCopy() const {
    Mesh m;
    m.isShallow = true;
    m = *this;
    return m;
}



const Mesh & Mesh::Basic_Cube() {
    static Mesh * cube_base = nullptr;
    if (!cube_base) {
        cube_base = new Mesh();
        cube_base->AddObject();
        MeshObject * obj = cube_base->Get(0);
        obj->faceList = {
            0,  1,  2,      2,  3,  1,
            4,  5,  6,      6,  7,  5,
            8,  9,  10,     10, 11, 9,
            12, 13, 14,     14, 15, 13,
            16, 17, 18,     18, 19, 17,
            20, 21, 22,     22, 23, 21
        };
        
        
        cube_base->DefineVerticesState(
            {
                // back face
                {-0.5f, -0.5f, -0.5f,    0, 0, -1.f,     0.f, 0.f,   0.f, 0.f, 0.f, 0.f}, 
                { 0.5f, -0.5f, -0.5f,    0, 0, -1.f,     1.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                {-0.5f,  0.5f, -0.5f,    0, 0, -1.f,     0.f, 1.f,   0.f, 0.f, 0.f, 0.f},
                { 0.5f,  0.5f, -0.5f,    0, 0, -1.f,     1.f, 1.f,   0.f, 0.f, 0.f, 0.f},

                // front face
                {-0.5f, -0.5f, 0.5f,     0, 0,  1.f,     0.f, 0.f,   0.f, 0.f, 0.f, 0.f},         
                { 0.5f, -0.5f, 0.5f,     0, 0,  1.f,     1.f, 0.f,   0.f, 0.f, 0.f, 0.f},    
                {-0.5f,  0.5f, 0.5f,     0, 0,  1.f,     0.f, 1.f,   0.f, 0.f, 0.f, 0.f},
                { 0.5f,  0.5f, 0.5f,     0, 0,  1.f,     1.f, 1.f,   0.f, 0.f, 0.f, 0.f},

                // right face
                {0.5f, -0.5f, -0.5f,    1.f, 0, 0,      0.f, 0.f,   0.f, 0.f, 0.f, 0.f},         
                {0.5f,  0.5f, -0.5f,    1.f, 0, 0,      1.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                {0.5f, -0.5f,  0.5f,    1.f, 0, 0,      0.f, 1.f,   0.f, 0.f, 0.f, 0.f},
                {0.5f,  0.5f,  0.5f,    1.f, 0, 0,      1.f, 1.f,   0.f, 0.f, 0.f, 0.f},

                // left face
                {-0.5f, -0.5f, -0.5f,    -1.f, 0, 0,     0.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                {-0.5f,  0.5f, -0.5f,    -1.f, 0, 0,     1.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                {-0.5f, -0.5f,  0.5f,    -1.f, 0, 0,     0.f, 1.f,   0.f, 0.f, 0.f, 0.f},
                {-0.5f,  0.5f,  0.5f,    -1.f, 0, 0,     1.f, 1.f,   0.f, 0.f, 0.f, 0.f},

                // bottom face
                {-0.5f, -0.5f, -0.5f,    0, -1.f, 0,     0.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                { 0.5f, -0.5f, -0.5f,    0, -1.f, 0,     1.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                {-0.5f, -0.5f,  0.5f,    0, -1.f, 0,     0.f, 1.f,   0.f, 0.f, 0.f, 0.f},
                { 0.5f, -0.5f,  0.5f,    0, -1.f, 0,     1.f, 1.f,   0.f, 0.f, 0.f, 0.f},

                // top face
                {-0.5f,  0.5f, -0.5f,    0, 1.f, 0,      0.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                { 0.5f,  0.5f, -0.5f,    0, 1.f, 0,      1.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                {-0.5f,  0.5f,  0.5f,    0, 1.f, 0,      0.f, 1.f,   0.f, 0.f, 0.f, 0.f},
                { 0.5f,  0.5f,  0.5f,    0, 1.f, 0,      1.f, 1.f,   0.f, 0.f, 0.f, 0.f},

            }
        );
    }
    return *cube_base;
}



const Mesh & Mesh::Basic_Square() {
    static Mesh * square_base = nullptr;
    if (!square_base) {
        square_base = new Mesh();
        square_base->AddObject();
        MeshObject * obj = square_base->Get(0);
        obj->faceList = {
            0, 1, 2,    0, 2, 3
        };
        
        
        square_base->DefineVerticesState(
            {
                {-0.5f, 0.f, -0.5f,      0.f, 1.f, 0.f,      0.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                { 0.5f, 0.f, -0.5f,      0.f, 1.f, 0.f,      1.f, 0.f,   0.f, 0.f, 0.f, 0.f},
                { 0.5f, 0.f,  0.5f,      0.f, 1.f, 0.f,      1.f, 1.f,   0.f, 0.f, 0.f, 0.f},
                {-0.5f, 0.f,  0.5f,      0.f, 1.f, 0.f,      0.f, 1.f,   0.f, 0.f, 0.f, 0.f}
            }
        );
    }
    return *square_base;
}






