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


using namespace Dynacoe;

Render2D::Render2D(const std::string & n) : Component(n){
    absolute = false;
    mode = RenderMode::Normal;
    polygon = Renderer::Polygon::Triangle;
}

void Render2D::SetVertices(const std::vector<Renderer::DynamicVertex> & v) {
    vertices = v;
}

std::vector<Renderer::DynamicVertex> & Render2D::ChangeVertices() {
    return vertices;
}

const std::vector<Renderer::DynamicVertex> & Render2D::GetVertices() {
    return vertices;
}

Renderer::Polygon Render2D::GetPolygon() {
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
