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
#include <Dynacoe/Backends/Renderer/RendererES.h>
#include <Dynacoe/Backends/Renderer/RendererES/Texture_ES.h>
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer.h>
#include <Dynacoe/Util/Table.h>
#include <Dynacoe/Backends/Framebuffer/Framebuffer.h>

#include <Dynacoe/Backends/Display/OpenGLFramebuffer_Multi.h>

using namespace Dynacoe;

struct Dynacoe::GLES2Implementation {
    Dynacoe::Table<RenderBuffer*> buffers;
    Dynacoe::Framebuffer * target;
    Dynacoe::Texture_ES * texture;    

    GLES2Implementation() {
        #ifdef DC_BACKENDS_GLESFRAMEBUFFER_X11
            new Dynacoe::OpenGLFBDisplay;
        #else 
            assert(!"Can't have OpenGLES renderer without GLESFramebuffer or some other means to start the context!");
        #endif


        target = nullptr;
        texture = new Texture_ES();
    }

};




#include <cassert>
GLES2::GLES2() {
    ES = new GLES2Implementation();

    // renderbuffer test
    /*
    float src[] = {
        1.f, 2.f, 3.f, 4.f,
        1.f, 2.f, 3.f, 4.f,
        1.f, 2.f, 3.f, 4.f,
    };

    auto b1 = AddBuffer(src, 12);
    auto b2 = AddBuffer(nullptr, 12);

    float src2[12];

    ReadBuffer(b1, src2, 0, 12);
    for(int i = 0; i < 12; ++i) {
        printf("%f %f\n", src[i], src2[i]);
    }

    UpdateBuffer(b2, src2, 0, 12);
    assert(BufferSize(b1) == BufferSize(b2));

    RemoveBuffer(b1);
    RemoveBuffer(b2);
    */


}

GLES2::~GLES2() {
    delete ES;
}


Dynacoe::Framebuffer * GLES2::GetTarget() {
    return ES->target;
}

void GLES2::AttachTarget(Dynacoe::Framebuffer * f) {
    ES->target = f;
}




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







////////// RenderBuffer

RenderBufferID GLES2::AddBuffer(float * data, int numElements) {
    RenderBuffer * buffer = new RenderBuffer();
    buffer->Define(data, numElements);
    buffer->SetType(GL_ARRAY_BUFFER);
    return ES->buffers.Insert(buffer);
}

void GLES2::UpdateBuffer(RenderBufferID id, float * newData, int offset, int numElements) {
    if (!ES->buffers.Query(id)) return;
    auto * buffer = ES->buffers.Find(id);
    buffer->UpdateData(newData, offset, numElements);
}

void GLES2::ReadBuffer(RenderBufferID id, float * outputData, int offset, int numElements) {
    if (!ES->buffers.Query(id)) return;
    auto * buffer = ES->buffers.Find(id);
    buffer->GetData(outputData, offset, numElements);
}

int GLES2::BufferSize(RenderBufferID id) {
    if (!ES->buffers.Query(id)) return 0;
    auto * buffer = ES->buffers.Find(id);
    return buffer->Size();
}

void GLES2::RemoveBuffer(RenderBufferID id) {
    if (!ES->buffers.Query(id)) return;
    ES->buffers.Remove(id);

}

////////////RenderBuffer






#endif
