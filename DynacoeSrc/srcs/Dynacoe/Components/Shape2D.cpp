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


#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Image.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Modules/Graphics.h>
#include <cmath>
#include <cstring>

using namespace Dynacoe;
using std::vector;



static float quadUV[] =  {
    0.f, 0.f,
    0.f, 1.f,
    1.f, 1.f,

    0.f, 0.f,
    1.f, 1.f,
    1.f, 0.f
};



Shape2D::Shape2D() : Render2D("Shape2D") {
    idFrame = 0;
    realColor = "white";
    color = "white";
    forcedWidth = -1;
    forcedHeight = -1;
    currentTexture = -1;
}


void Shape2D::FormRectangle(
    float w, float h
) {

    id = AssetID();

    vector<Renderer::Vertex2D> vertices;
    vertices.resize(6);

    float rgba[4];
    rgba[0] = color.r;
    rgba[1] = color.g;
    rgba[2] = color.b;
    rgba[3] = color.a;

    vertices[0] = {0, 0,     rgba[0], rgba[1], rgba[2], rgba[3]};
    vertices[1] = {0, h,     rgba[0], rgba[1], rgba[2], rgba[3]};
    vertices[2] = {w, h,     rgba[0], rgba[1], rgba[2], rgba[3]};
    vertices[3] = {0, 0,     rgba[0], rgba[1], rgba[2], rgba[3]};
    vertices[4] = {w, h,     rgba[0], rgba[1], rgba[2], rgba[3]};
    vertices[5] = {w, 0,     rgba[0], rgba[1], rgba[2], rgba[3]};

    SetPolygon(Renderer::Polygon::Triangle);
    SetVertices(vertices);

};


void Shape2D::FormImage(AssetID id_, float fw, float fh) {
    Image * im;
    if (!(im = &Assets::Get<Image>(id_))) {
        Console::Info()  << "Aspect2D::FormImage : invalid image id given" << Console::End;
        return;
    }
    forcedWidth = fw;
    forcedHeight = fh;
    vector<Renderer::Vertex2D> vertices;
    vertices.resize(6);

    float rgba[4];
    rgba[0] = color.r;
    rgba[1] = color.g;
    rgba[2] = color.b;
    rgba[3] = color.a;

    //float texID = (idFrame < 0 ? im->getNextFrame() : im->getFrame(idFrame));
    id = id_;
    float tex = im->CurrentFrame().GetHandle();
    float w = im->CurrentFrame().Width();
    float h = im->CurrentFrame().Height();
    if (forcedWidth > 0.f) w = forcedWidth;
    if (forcedHeight > 0.f) h = forcedHeight;
    currentTexture = tex;

    vertices[0] = {0, 0,     rgba[0], rgba[1], rgba[2], rgba[3],  tex, 0, 0};
    vertices[1] = {0, h,     rgba[0], rgba[1], rgba[2], rgba[3],  tex, 0, 1};
    vertices[2] = {w, h,     rgba[0], rgba[1], rgba[2], rgba[3],  tex, 1, 1};
    vertices[3] = {0, 0,     rgba[0], rgba[1], rgba[2], rgba[3],  tex, 0, 0};
    vertices[4] = {w, h,     rgba[0], rgba[1], rgba[2], rgba[3],  tex, 1, 1};
    vertices[5] = {w, 0,     rgba[0], rgba[1], rgba[2], rgba[3],  tex, 1, 0};


    SetVertices(vertices);
    SetPolygon(Renderer::Polygon::Triangle);
    idFrame = -1;

}


void Shape2D::FormImageFrame(AssetID id_, int frame, float forcedWidth, float forcedHeight) {
    if (!id_.Valid()) return;
    FormImage(id_, forcedWidth, forcedHeight);
    idFrame = frame;

    Image & im = Assets::Get<Image>(id_);
    SetFrameActive(im.frames[frame%im.frames.size()]);
}



void Shape2D::FormCircle(float radius, int numIterations) {
    vector<Vector> pts;
    Vector curPt;
    id = AssetID();

    for(int i = 0; i < numIterations; ++i) {
        curPt.x = cos(2*Math::Pi() * (i / (float) numIterations)) * radius;
        curPt.y = sin(2*Math::Pi() * (i / (float) numIterations)) * radius;
        pts.push_back(curPt);
        curPt.x = cos(2*Math::Pi() * (((i+1)%numIterations) / (float) numIterations)) * radius;
        curPt.y = sin(2*Math::Pi() * (((i+1)%numIterations) / (float) numIterations)) * radius;
        pts.push_back(curPt);
        pts.push_back(Vector());
    }
    FormTriangles(pts);
}


void Shape2D::FormTriangles(vector<Vector> & pts) {
    std::vector<Renderer::Vertex2D> vertices;
    vertices.resize(pts.size());

    float rgba[4];
    rgba[0] = color.r;
    rgba[1] = color.g;
    rgba[2] = color.b;
    rgba[3] = color.a;


    for(uint32_t i = 0; i < vertices.size(); ++i) {
        vertices[i] = {
            pts[i].x, pts[i].y,
            rgba[0], rgba[1], rgba[2], rgba[3]
        };
    }
    SetPolygon(Renderer::Polygon::Triangle);
    SetVertices(vertices);
}

void Shape2D::FormLines(const vector<Vector> & pts) {
    std::vector<Renderer::Vertex2D> vertices;
    vertices.resize(pts.size());
    id = AssetID();

    float rgba[4];
    rgba[0] = color.r;
    rgba[1] = color.g;
    rgba[2] = color.b;
    rgba[3] = color.a;


    for(uint32_t i = 0; i < vertices.size(); ++i) {
        vertices[i] = {
            pts[i].x, pts[i].y,
            rgba[0], rgba[1], rgba[2], rgba[3]
        };
    }
    SetPolygon(Renderer::Polygon::Line);
    SetVertices(vertices);
}






void Shape2D::OnDraw() {
    // TODO: there... needs to be a better way
    if (!(realColor == color)) {
        std::vector<Renderer::Vertex2D> v = GetVertices();
        for(uint32_t i = 0; i < v.size(); ++i) {
            v[i].r = color.r;
            v[i].g = color.g;
            v[i].b = color.b;
            v[i].a = color.a;
        }
        realColor = color;
        SetVertices(v);
    }

    if (id.Valid() && idFrame != -1) {
        Image & im = Assets::Get<Image>(id);
        SetFrameActive(im.CurrentFrame());
    }

    //TODO: iterate through frames
    Graphics::Draw(*this);
}


std::string Shape2D::GetInfo() {
    return (Chain() <<
        "Color: " << color.ToString().c_str() << "\n" <<
        "Vertices: " << (int)GetVertexIDs().size() << "\n" <<
        node.GetInfo()
    );
}


void Shape2D::SetFrameActive(const Image::Frame & frameRef) {
    float tex = frameRef.GetHandle();
    if (currentTexture == tex) return;
    if (GetVertexIDs().size() < 6) return;
    currentTexture = tex;
    
    vector<Renderer::Vertex2D> vertices = GetVertices();
    vertices[0].useTex = tex;
    vertices[1].useTex = tex;
    vertices[2].useTex = tex;

    vertices[3].useTex = tex;
    vertices[4].useTex = tex;
    vertices[5].useTex = tex;


    float w = frameRef.Width();
    float h = frameRef.Height();
    if (forcedWidth > 0.f) w = forcedWidth;
    if (forcedHeight > 0.f) h = forcedHeight;

    vertices[1].y = h;
    vertices[2].x = w;
    vertices[2].y = h;
    vertices[4].x = w;
    vertices[4].y = h;
    vertices[5].x = w;
    
    
    SetVertices(vertices);
}
