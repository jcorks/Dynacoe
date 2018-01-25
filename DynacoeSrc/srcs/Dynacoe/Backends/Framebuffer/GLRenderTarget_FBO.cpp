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

#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget_FBO.h>


#ifdef DC_SUBSYSTEM_X11
    #include <X11/X.h>
    #include <X11/Xlib.h>
    #ifndef GLEW_STATIC
    #define GLEW_STATIC
    #endif
    #include <GL/glew.h>
    #include <GL/glx.h>
    typedef Display X11Display;
#endif

#ifdef DC_SUBSYSTEM_WIN32
    #ifndef GLEW_STATIC
    #define GLEW_STATIC
    #endif
    #include <glew.h>
    #include <GL/gl.h>
    #include <windows.h>
#endif
#include <algorithm>

////// GLRenderTarget meant for framebuffer implementations.
////// Performance wise, this implementation will ALWAYS be faster than
////// GLRenderTarget_Legacy, so it should be preferred always


using namespace Dynacoe;

GLRenderTarget_FBO::GLRenderTarget_FBO() {
    // generate the data structures
    glGenTextures(1, &texture);
    glGenFramebuffers (1, &framebuffer);
    glGenRenderbuffers(1, &renderbuffer);


    GLint oldT, oldFB, oldRB;
    glGetIntegerv(GL_TEXTURE_BINDING_2D,   &oldT);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING,  &oldFB);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRB);

    glBindTexture(GL_TEXTURE_2D, texture);



    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640,
                                            480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, texture, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 640, 480);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, oldT);
    glBindFramebuffer(GL_FRAMEBUFFER, oldFB);
    glBindRenderbuffer(GL_RENDERBUFFER, oldRB);

    w = 640;
    h = 480;

}

GLRenderTarget_FBO::~GLRenderTarget_FBO() {
    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &renderbuffer);
}


void GLRenderTarget_FBO::Resize(int newW, int newH) {
    GLint old, oldRB;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRB);

    // init tex
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newW,
                                           newH, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, newW, newH);

    glBindTexture(GL_TEXTURE_2D, old);
    glBindRenderbuffer(GL_RENDERBUFFER, oldRB);

    w = newW;
    h = newH;
}


void GLRenderTarget_FBO::GetRawData(uint8_t * data) {
    GLint old;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, old);


    uint32_t * data32 = (uint32_t*)data;
    for(uint32_t y = 0; y < h/2; ++y) {
        for(uint32_t x = 0; x < w; ++x) {
            std::swap(data32[x+y*w], data32[x+w*(h-y-1)]);
        }
    }
}


void GLRenderTarget_FBO::DrawTo() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, w, h);
}


void GLRenderTarget_FBO::Sync() {

    // implicit!! WOO!
}

void GLRenderTarget_FBO::Invalidate() {

}

GLuint GLRenderTarget_FBO::GetTexture() {
    return texture;
}


void GLRenderTarget_FBO::SetFiltering(bool doIt) {
    GLint oldBinding;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, doIt ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, doIt ? GL_LINEAR : GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, oldBinding);
}




#endif
