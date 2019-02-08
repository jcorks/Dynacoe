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

#if (defined DC_BACKENDS_GLES_X11)

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <Dynacoe/Backends/Renderer/RendererES.h>
#include <Dynacoe/Backends/Renderer/RendererES/Texture_ES.h>
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer_ES.h>
#include <Dynacoe/Backends/Renderer/RendererES/Renderer2D_ES.h>
#include <Dynacoe/Backends/Renderer/RendererES/StaticRenderer.h>
#include <Dynacoe/Backends/Renderer/RendererES/Light_ES.h>

#include <Dynacoe/Util/Table.h>
#include <Dynacoe/Backends/Framebuffer/Framebuffer.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget.h>
#include <Dynacoe/Backends/Display/OpenGLFramebuffer_Multi.h>

#include <cassert>
using namespace Dynacoe;

struct Dynacoe::GLES2Implementation {
    Dynacoe::Table<RenderBuffer*> buffers;
    Dynacoe::Framebuffer * target;
    Dynacoe::Texture_ES * texture;   
    Dynacoe::Renderer2D * render2d;
    Dynacoe::StaticRenderer * renderStatic;
    Dynacoe::Light_ES * lighting;

    Renderer::Polygon curPolygon;
    Renderer::DepthTest curDepthTest;
    Renderer::AlphaRule curAlphaRule;
    GLenum drawMode;
    

    GLES2Implementation() {
        #ifdef DC_BACKENDS_GLESFRAMEBUFFER_X11
        #ifndef ANDROID
            (new Dynacoe::OpenGLFBDisplay)->Hide(true);
        #else 
            assert(eglGetCurrentContext());
        #endif
        
        #else 
            // assume the environment already initialized the
            // opengl es 
            assert(eglGetCurrentContext());
        #endif


        target = nullptr;
        texture = new Texture_ES();
        render2d = new Renderer2D(texture);
        lighting = new Light_ES();
        renderStatic = new StaticRenderer(texture, &buffers, lighting);

        drawMode = GL_TRIANGLES;
        assert(glGetError() == 0);

        curAlphaRule = Renderer::AlphaRule::Allow;
        curDepthTest = Renderer::DepthTest::None;
        curPolygon = Renderer::Polygon::Triangle;

    }

};




#include <cassert>
GLES2::GLES2() {
    ES = new GLES2Implementation();
    assert(glGetError() == 0);
    SetDrawingMode(ES->curPolygon, ES->curDepthTest, ES->curAlphaRule);


}

GLES2::~GLES2() {
    delete ES;
}


Dynacoe::Framebuffer * GLES2::GetTarget() {
    return ES->target;
}


void GLES2::AttachTarget(Framebuffer * f) {
    if (!f) {
        ES->target = nullptr;
        return;
    }

    if (f == ES->target) return;
    if (f->GetHandleType() != Dynacoe::Framebuffer::Type::GLFBPacket) return;
    ES->target = f;

}

void GLES2::ClearRenderedData() {
    (*(GLRenderTarget**)ES->target->GetHandle())->DrawTo();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    assert(glGetError() == 0);

}

void GLES2::Reset(Renderer::DataLayer layer) {
    (*(GLRenderTarget**)ES->target->GetHandle())->DrawTo();
    switch(layer) {
      case Renderer::DataLayer::Color: glClear(GL_COLOR_BUFFER_BIT); break;
      case Renderer::DataLayer::Depth: glClear(GL_DEPTH_BUFFER_BIT); break;
      case Renderer::DataLayer::Etch:  glClear(GL_STENCIL_BUFFER_BIT); break;
    }
}








///////////////////// static render
void GLES2::RenderStatic(StaticState * st) {
    if (!ES->target) return;
    (*(GLRenderTarget**)ES->target->GetHandle())->DrawTo();
    ES->renderStatic->Render(st);
    (*(GLRenderTarget**)ES->target->GetHandle())->Invalidate();
    assert(glGetError() == 0);

}
RenderBufferID GLES2::GetStaticViewingMatrixID() {
    return ES->renderStatic->GetViewingMatrixID();
}

RenderBufferID GLES2::GetStaticProjectionMatrixID() {
    return ES->renderStatic->GetProjectionMatrixID();
}


ProgramID GLES2::ProgramGetBuiltIn(BuiltInShaderMode s) {
    return ES->renderStatic->ProgramGetBuiltIn(s);
}

ProgramID GLES2::ProgramAdd(const std::string & v, const std::string & f, std::string & log) {
    auto out = ES->renderStatic->ProgramAdd(v, f, log);
    assert(glGetError() == 0);
    return out;

}

///////////////////// static render












/////////////////////////////////// 2D

void GLES2::Queue2DVertices(
    const uint32_t * indices,
    uint32_t count
) {

    return ES->render2d->Queue2DVertices(
        indices,
        count
    );
}

uint32_t GLES2::Add2DObject() {
    return ES->render2d->Add2DObject();    
}

// Removes a 2D object of the given ID
void GLES2::Remove2DObject(uint32_t id) {
    ES->render2d->Remove2DObject(id);
}



// Adds a new vertex 
uint32_t GLES2::Add2DVertex() {
    return ES->render2d->Add2DVertex();
}

void GLES2::Remove2DVertex(uint32_t object) {
    ES->render2d->Remove2DVertex(object);
}

void GLES2::Set2DVertex(uint32_t vertex, Vertex2D data) {
    ES->render2d->Set2DVertex(vertex, data);
}

Renderer::Vertex2D GLES2::Get2DVertex(uint32_t vertex) {
    return ES->render2d->Get2DVertex(vertex);
}

void GLES2::Set2DObjectParameters(uint32_t object, Render2DObjectParameters data) {
    ES->render2d->Set2DObjectParameters(object, data);
}

void GLES2::Render2DVertices(const Render2DStaticParameters & data) {
    if (!ES->target) return;
    (*(GLRenderTarget**)ES->target->GetHandle())->DrawTo();
    ES->render2d->Render2DVertices(ES->drawMode, data);
    (*(GLRenderTarget**)ES->target->GetHandle())->Invalidate();

}

// Clears all requests queued before the last RenderDynamicQueue
void GLES2::Clear2DQueue() {
    ES->render2d->Clear2DQueue();
}
////////////////////////// 2D









/////////////////////////// TEXTURING


int GLES2::AddTexture(int w, int h, const uint8_t* rgba) {
    return ES->texture->NewTexture(w, h, rgba);
}


void GLES2::UpdateTexture(int tex, const uint8_t * newData) {
    ES->texture->UpdateTexture(tex, newData);
}

void GLES2::RemoveTexture(int tex) {
    ES->texture->DeleteTexture(tex);
}

void GLES2::GetTexture(int tex, uint8_t * data) {
    ES->texture->GetTextureData(tex, data);
}

void GLES2::SetTextureFilter(Renderer::TexFilter tex) {
    ES->texture->SetFilter(tex);
}

Renderer::TexFilter GLES2::GetTextureFilter() {
    return ES->texture->GetFilter();
}

int GLES2::GetTextureWidth(int tex) {
    return ES->texture->GetSubTextureBounds(tex)[2];
}

int GLES2::GetTextureHeight(int tex) {
    return ES->texture->GetSubTextureBounds(tex)[3];
}

int GLES2::MaxSimultaneousTextures() {
    return 64;
}


//////////////// TEXTURING






////////// RenderBuffer

RenderBufferID GLES2::AddBuffer(float * data, int numElements) {
    RenderBuffer * buffer = new RenderBuffer();
    buffer->Define(data, numElements);
    buffer->SetType(GL_ARRAY_BUFFER);
    assert(glGetError() == 0);

    return ES->buffers.Insert(buffer);
}

void GLES2::UpdateBuffer(RenderBufferID id, float * newData, int offset, int numElements) {
    if (!ES->buffers.Query(id)) return;
    auto * buffer = ES->buffers.Find(id);
    buffer->UpdateData(newData, offset, numElements);
    assert(glGetError() == 0);

}

void GLES2::ReadBuffer(RenderBufferID id, float * outputData, int offset, int numElements) {
    if (!ES->buffers.Query(id)) return;
    auto * buffer = ES->buffers.Find(id);
    buffer->GetData(outputData, offset, numElements);
    assert(glGetError() == 0);

}

int GLES2::BufferSize(RenderBufferID id) {
    if (!ES->buffers.Query(id)) return 0;
    auto * buffer = ES->buffers.Find(id);
    assert(glGetError() == 0);

    return buffer->Size();
}

void GLES2::RemoveBuffer(RenderBufferID id) {
    if (!ES->buffers.Query(id)) return;
    ES->buffers.Remove(id);
    assert(glGetError() == 0);

}

////////////RenderBuffer















////////////Lighting 




LightID GLES2::AddLight(LightType type) {
    return ES->lighting->AddLight(type);
}


void GLES2::UpdateLightAttributes(LightID id, float * data) {
    ES->lighting->UpdateLightAttributes(id, data);
}

void GLES2::EnableLight(LightID id, bool doIt){
    ES->lighting->EnableLight(id, doIt);
}
void GLES2::RemoveLight(LightID id){
    ES->lighting->RemoveLight(id);
}

int GLES2::MaxEnabledLights(){
    return ES->lighting->MaxEnabledLights();
}

int GLES2::NumLights(){
    return ES->lighting->NumLights();
}







////////////Lighting













/////////// drawing engine options
void GLES2::SetDrawingMode(Renderer::Polygon p, Renderer::DepthTest d, Renderer::AlphaRule a) {

    
    switch(p) {
        case Renderer::Polygon::Triangle: 
            ES->drawMode = GL_TRIANGLES; ES->curPolygon = p; break;
        case Renderer::Polygon::Line: 
            ES->drawMode = GL_LINES;  ES->curPolygon = p; break;
        default:;


    }



    switch(d) {
        case Renderer::DepthTest::NoTest:  glDisable(GL_DEPTH_TEST); curDepthTest = d; break;
        case Renderer::DepthTest::Greater: glEnable(GL_DEPTH_TEST); glDepthFunc(GL_GREATER); ES->curDepthTest = d; break;
        case Renderer::DepthTest::Less:    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); ES->curDepthTest = d; break;
        case Renderer::DepthTest::GEQ:     glEnable(GL_DEPTH_TEST); glDepthFunc(GL_GEQUAL); ES->curDepthTest = d; break;
        case Renderer::DepthTest::LEQ:     glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); ES->curDepthTest = d; break;


        default:;
    }

    switch(a) {
        case Renderer::AlphaRule::Allow:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
            ES->curAlphaRule = a;
            break;


        case Renderer::AlphaRule::Opaque:
            glDisable(GL_BLEND);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
            ES->curAlphaRule = a;
            break;


        case Renderer::AlphaRule::Translucent:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);  
            ES->curAlphaRule = a;
            break;

        case Renderer::AlphaRule::Invisible:
            glDisable(GL_ALPHA_TEST);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
            ES->curAlphaRule = a;


        default:;

    }



    switch(e) {
        case Renderer::EtchRule::NoEtching: curEtchRule = e; glDisable(GL_STENCIL_TEST); break;
        case Renderer::EtchRule::EtchDefine: 
            ES->curEtchRule = e; 
            glEnable(GL_STENCIL_TEST); 
            glStencilFunc(GL_ALWAYS, 1, 0xff);
            glStencilMask(0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

        case Renderer::EtchRule::EtchUndefine: 
            ES->curEtchRule = e; 
            glEnable(GL_STENCIL_TEST); 
            glStencilFunc(GL_ALWAYS, 0, 0xff);
            glStencilMask(0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

        case Renderer::EtchRule::EtchIn: 
            ES->curEtchRule = e; 
            glEnable(GL_STENCIL_TEST); 
            glStencilFunc(GL_EQUAL, 1, 0xff);
            glStencilMask(0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_Replace);
            break;


    }
    assert(glGetError() == 0);

}


void GLES2::GetDrawingMode(Polygon * p, DepthTest * d, AlphaRule * a) {
    *p = ES->curPolygon;
    *d = ES->curDepthTest;
    *a = ES->curAlphaRule;
}

////////////// drawing engine options


void GLES2::Sync() {
    ES->render2d->Disable2DRenderMode();
}

#endif
