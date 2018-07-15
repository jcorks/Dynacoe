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

#include <Dynacoe/Components/Render2D.h>
#include <Dynacoe/Modules/Graphics.h>

using namespace Dynacoe;

static DynacoeEvent(Render2D_UpdateRendererObject) {
    Render2D * r2d = (Render2D*)functionData;

    TransformMatrix m = r2d->node.GetGlobalTransform();
    m.ReverseMajority();
    Renderer::Render2DObjectParameters obj = *(Renderer::Render2DObjectParameters*)m.GetData();
    Graphics::GetRenderer()->Set2DObjectParameters(
        r2d->GetObjectID(),
        obj
    );
    return true;
}
Render2D::Render2D(const std::string & n) : Render2D(n, new Node){}
Render2D::Render2D(const std::string & n, Node * nInst) : Component(n), node(*nInst){
    absolute = false;
    mode = RenderMode::Normal;
    polygon = Renderer::Polygon::Triangle;
    objectID = Graphics::GetRenderer()->Add2DObject();
    node.InstallHook("on-update", Render2D_UpdateRendererObject, this);
}

Render2D::~Render2D() {
    node.UninstallHook("on-update", Render2D_UpdateRendererObject);
    Graphics::GetRenderer()->Remove2DObject(objectID);
}

void Render2D::SetVertices(const std::vector<Renderer::Vertex2D> & v) {
    while (vertexSrc.size() < v.size()) {
        vertexSrc.push_back(Graphics::GetRenderer()->Add2DVertex());
    }
    
    while (vertexSrc.size() > v.size()) {
        uint32_t t = vertexSrc[vertexSrc.size()-1];
        Graphics::GetRenderer()->Remove2DVertex(t);
        vertexSrc.erase(vertexSrc.begin()+(vertexSrc.size()-1));
    }


    for(uint32_t i = 0; i < v.size(); ++i) {
        Renderer::Vertex2D vertex = v[i];
        vertex.object = (float)objectID;
        Graphics::GetRenderer()->Set2DVertex(vertexSrc[i], vertex);
    }
}


std::vector<Renderer::Vertex2D> Render2D::GetVertices() const {
    std::vector<Renderer::Vertex2D> out;
    for(uint32_t i = 0; i < vertexSrc.size(); ++i) {
        out.push_back(Graphics::GetRenderer()->Get2DVertex(vertexSrc[i]));
    }
    return out;
}

uint32_t Render2D::GetObjectID() const {
    return objectID;
}

const std::vector<uint32_t> & Render2D::GetVertexIDs() const {
    return vertexSrc;
}

Renderer::Polygon Render2D::GetPolygon() const {
    return polygon;
}

void Render2D::SetPolygon(Renderer::Polygon p) {
    polygon = p;
}

void Render2D::OnAttach() {
    node.SetManualParent(GetHost()->QueryComponent<Node>());
    node.OnAttach();
}

void Render2D::OnStep() {
    node.Step();
}
