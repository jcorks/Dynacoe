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

#if ( defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32 || defined DC_BACKENDS_LEGACYGL_WIN32 || defined DC_BACKENDS_LEGACYGL_X11)

#include <Dynacoe/Backends/Framebuffer/OpenGLFB_Multi.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget.h>

#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Image.h>


using namespace Dynacoe;




OpenGLFB::OpenGLFB() : Framebuffer(
        Framebuffer::Type::GLFBPacket,
        640,
        480,
        &rt
    ){
    rt = CreateGLRenderTarget();
    OnResize(&rt, 640, 480);
}

OpenGLFB::~OpenGLFB() {
    delete rt;
}


bool OpenGLFB::OnResize(void * source, int newW, int newH) {
    rt->Resize(newW, newH);
    return true;
}


bool OpenGLFB::GetRawData(uint8_t * data) {
    rt->Sync();
    rt->GetRawData(data);
    return true;
}

std::string OpenGLFB::Name() {return "OpenGL Framebuffer";}
std::string OpenGLFB::Version() {return "v1.0 (OpenGL 3.0 or framebuffer_obj_EXT)";}
bool        OpenGLFB::Valid() {return true;}





void OpenGLFB::OnFilterChange(bool b) {
    rt->SetFiltering(b);
}

#endif
