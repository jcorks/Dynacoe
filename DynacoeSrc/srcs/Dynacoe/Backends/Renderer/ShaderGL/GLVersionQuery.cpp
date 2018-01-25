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

#if ( defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32 )

#include <Dynacoe/Backends/Renderer/ShaderGL/GLVersionQuery.h>
#include <cassert>

static bool gl_version3_0 = false;
static bool gl_version3_1 = false;
static bool gl_version2_1 = false;
static bool gl_uniform_buffer_object = false;
static bool gl_framebuffer_object = false;

static bool isInited = false;

using namespace Dynacoe;
bool Dynacoe::GLVersionInit() {
    if (glewInit() != GLEW_OK) return false;
    gl_version3_0 = glewIsSupported("GL_VERSION_3_0");
    gl_version3_1 = glewIsSupported("GL_VERSION_3_1");
    gl_version2_1 = glewIsSupported("GL_VERSION_2_1");
    gl_uniform_buffer_object = gl_version3_1 ? true : glewIsSupported("GL_ARB_uniform_buffer_object");
    gl_framebuffer_object = gl_version3_0 || gl_version3_1 ? true : glewIsSupported("GL_EXT_framebuffer_object");
    isInited = true;

    return true;
}

bool Dynacoe::GLVersionQuery(int mask) {
    bool out = true;
    assert(isInited);
    if (mask & GL_Version3_0)          out &= gl_version3_0;
    if (mask & GL_Version3_1)          out &= gl_version3_1;
    if (mask & GL_Version2_1)          out &= gl_version2_1;
    if (mask & GL_UniformBufferObject) out &= gl_uniform_buffer_object;
    if (mask & GL_FramebufferObject)   out &= gl_framebuffer_object;
    return out;
}
    
    
#endif