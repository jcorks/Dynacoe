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

#include <Dynacoe/Camera.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Modules/ViewManager.h>
#include <Dynacoe/Components/Node.h>
#include <Dynacoe/Util/Math.h>
#include <cmath>

using namespace Dynacoe;





Camera * Camera::activeRef = nullptr;



static TransformMatrix Matrix_ProjectionOrthogonal(float, float, float, float, float, float);
static TransformMatrix Matrix_ProjectionPerspective(float, float, float, float);
static TransformMatrix Matrix_ViewLookAt(const Vector &, const Vector &, const Vector &);

void Camera::BindTransformBuffers(RenderBufferID mview, RenderBufferID proj) {
    modelView = mview;
    projection = proj;
}

Camera::Camera() {
    modelView = Graphics::GetRenderer()->AddBuffer(nullptr, 32);
    projection = Graphics::GetRenderer()->AddBuffer(nullptr, 16);
    lastW = lastH = 0;
    fb = (Dynacoe::Framebuffer*)Backend::CreateDefaultFramebuffer();
    autoRefresh = true;
}

void Camera::SetType(Type t) {
    type = t;
    if (t == Type::Perspective3D) {
        projectionMatrix = Matrix_ProjectionPerspective(60, 1, .01, 100);
        TransformMatrix matr = projectionMatrix;
        matr.ReverseMajority();
        Graphics::GetRenderer()->UpdateBuffer(projection, matr.GetData(), 0, 16);
    } else {
        // force normal update. We don't do this for perspective because
        // we only need to set the projection once
        //OnStep();
    }

}




void Camera::Refresh() {
    Framebuffer * old = Graphics::GetRenderer()->GetTarget();
    if (old != fb) {
        Graphics::GetRenderer()->AttachTarget(fb);
        Graphics::GetRenderer()->ClearRenderedData();
        Graphics::GetRenderer()->AttachTarget(old);
    } else {
        Graphics::GetRenderer()->ClearRenderedData();
    }
}

void Camera::SetTarget(const Dynacoe::Vector & pos) {
    if (type == Type::Perspective3D) {
        /*
        transform.ResetView();
        transform.LookAt(node.local.position,
                  pos,
                  Vector(0, 1.f, 0));
        transform.RetrieveViewMatrix(&matr);
        */


        node.Rotation() = Vector();
        Vector p = node.GetGlobalTransform().Transform(node.GetPosition());
        TransformMatrix m = Matrix_ViewLookAt(p,
                  pos,
                  Vector(0, 1.f, 0));
        node.Rotation() = {
            m.Transform(Vector(1, 0, 0)).RotationZ(),
            m.Transform(Vector(0, 0, 1)).RotationY(),
            m.Transform(Vector(0, 1, 0)).RotationX()
        };

    } else {
        node.Position() = pos;
    }
}

Vector Camera::TransformWorldToScreen(const Dynacoe::Vector & p) {
    // 3D world -> homogeneous point -> 2D

    float w = fb->Width();
    float h = fb->Height();

    // combines homogeneous -> normalized device coordinates -> screen coordinates process

    //transform.TransformVectors(pIn, pOut, 1);
    Vector out = node.GetGlobalTransform().Transform(p);
    out = projectionMatrix.Transform(out);

    return Vector(      (out.x)*(w/2.f) + w/2.f,
                  h - ((out.y)*(h/2.f) + h/2.f),
                 0);
}

Framebuffer * Camera::GetFramebuffer() {
    return fb;
}

Vector Camera::TransformScreenToWorld(const Dynacoe::Vector & p, float distance) {
    float pIn[4],
          pOut[4];


    float w = fb->Width();
    float h = fb->Height();

    Vector in = {
        (p.x/w)*2 - 1,
        ((h-p.y)/h)*2 - 1,
        ((p.z+.01f)/100.f),
    };

    TransformMatrix mvInv = node.GetGlobalTransform();
    TransformMatrix pjInv = projectionMatrix;

    mvInv.Inverse();
    pjInv.Inverse();

    in = pjInv.Transform(in);
    in = mvInv.Transform(in);

    return in;
}


void Camera::OnStep() {
    TransformMatrix m = node.GetGlobalTransform();
    m.Inverse();
    m.ReverseMajority();
    Graphics::GetRenderer()->UpdateBuffer(modelView, m.GetData(), 0, 16);



    //node.UpdateModelTransforms(modelView);

    if (fb && type == Type::Orthographic2D) {
        if (lastW != Graphics::GetRenderer()->GetTarget()->Width() ||
            lastH != Graphics::GetRenderer()->GetTarget()->Height()) {

            lastW = Graphics::GetRenderer()->GetTarget()->Width();
            lastH = Graphics::GetRenderer()->GetTarget()->Height();

            projectionMatrix = Matrix_ProjectionOrthogonal(0, lastW, lastH, 0, -1024.f, 1024.f);

            TransformMatrix m = projectionMatrix;
            m.ReverseMajority();
            Graphics::GetRenderer()->UpdateBuffer(projection, m.GetData(), 0, 16);



        }
    }
}



TransformMatrix Camera::GetProjectionTransform() {
    return projectionMatrix;
}


void Camera::SetRenderResolution(int w, int h) {
    GetFramebuffer()->Resize(w, h);

}

int Camera::Width() {
    return GetFramebuffer()->Width();
}

int Camera::Height() {
    return GetFramebuffer()->Height();
}


TransformMatrix Matrix_ProjectionOrthogonal(
        float left,    float right,
        float bottom,  float top,
        float zNear,   float zFar) {
    TransformMatrix out;
    float * projection = out.GetData();
    projection[0] = (2)  / (right - left);
    projection[5] = (2)  / (top - bottom);
    projection[10] = (-2) / (zFar - zNear);

    projection[3]  = -(right + left) / (right - left);
    projection[7]  = -(top + bottom) / (top - bottom);
    projection[11] = -(zFar + zNear) / (zFar - zNear);
    return out;
}

TransformMatrix Matrix_ProjectionPerspective(
        float fovy, float ratio,
        float zNear, float zFar) {

    TransformMatrix out;
    float * projection = out.GetData();
    float perspective[4];
    perspective[0] = fovy * (Math::Pi() / 180.f);
    perspective[1] = ratio;
    perspective[2] = zNear;
    perspective[3] = zFar;

    float f = 1.f / (tan(perspective[0] / 2.f));
    projection[0] = f / perspective[1];
    projection[5] = f;
    projection[10] = (perspective[2] + perspective[3]) /
                       (perspective[2] - perspective[3]);

    projection[11] = (perspective[2] * perspective[3] * 2) /
                       (perspective[2] - perspective[3]);

    projection[14] = -1.f;

    return out;

}


TransformMatrix Matrix_ViewLookAt(const Dynacoe::Vector & camPos,
            const Dynacoe::Vector & target,
            const Dynacoe::Vector & upVec) {
    TransformMatrix out;

    float * laScratch = out.GetData();
    Vector F = target - camPos;
    F = F.Normalize();


    Vector s = F.Cross(upVec);
    s = s.Normalize();

    Vector u = s.Cross(F);

    laScratch[0] = s.x;
    laScratch[1] = s.y;
    laScratch[2] = s.z;
    laScratch[3] = 0;
    laScratch[4] = u.x;
    laScratch[5] = u.y;
    laScratch[6] = u.z;
    laScratch[7] = 0;
    laScratch[8] = -F.x;
    laScratch[9] = -F.y;
    laScratch[10] = -F.z;
    laScratch[11] = 0;
    laScratch[12] = 0;
    laScratch[13] = 0;
    laScratch[14] = 0;
    laScratch[15] = 1;


    out.Translate(-camPos.x, -camPos.y, -camPos.z);
    return out;
}



AssetID Camera::CopyDisplay() {
    std::vector<uint8_t> data;
    data.resize(fb->Width() * fb->Height() * 4);
    fb->GetRawData(&data[0]);

    /*
    for(uint32_t i = 0; i < fb->Width()*fb->Height(); ++i) {
        data[i*4+3] = 255;
    }
    */
    AssetID out = Assets::New(Assets::Type::Image);
    Image & img = Assets::Get<Image>(out);
    img.frames.push_back(Image::Frame(
        fb->Width(), fb->Height(), data
    ));

    return out;
}
