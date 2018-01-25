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


#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget_Legacy.h>
#include <cassert>
using namespace Dynacoe;
static GLRenderTarget_Legacy * currentGLFB = nullptr;

GLRenderTarget_Legacy::GLRenderTarget_Legacy() {
    GLint curTex;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &curTex);

    assert(glGetError() == GL_NO_ERROR);

    dirty = false;
    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, curTex);

    w = 64;
    h = 64;
    assert(glGetError() == GL_NO_ERROR);
}







GLRenderTarget_Legacy::~GLRenderTarget_Legacy() {
    assert(glGetError() == GL_NO_ERROR);
    glDeleteTextures(1, &colorTex);
    glDeleteTextures(1, &depthTex);
    assert(glGetError() == GL_NO_ERROR);
}



void GLRenderTarget_Legacy::Resize(int _w, int _h) {
    assert(glGetError() == GL_NO_ERROR);
    w = _w;
    h = _h;

    GLint curTex;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &curTex);

    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, curTex);
    assert(glGetError() == GL_NO_ERROR);
}






GLuint GLRenderTarget_Legacy::GetTexture() {
    return colorTex;
}


void GLRenderTarget_Legacy::DrawTo() {
    assert(glGetError() == GL_NO_ERROR);
    GLint curTex;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &curTex);

    // if there is a current framebuffer, we need
    // to save its current contents if applicable
    if (currentGLFB) {
        currentGLFB->Sync();
    }



    // first load in previous content

    uint8_t * data = new uint8_t[w*h*4];

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, colorTex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, depthTex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);
    glDrawPixels(w, h, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);

    glViewport(0, 0, w, h);

    currentGLFB = this;

    glBindTexture(GL_TEXTURE_2D, curTex);
    assert(glGetError() == GL_NO_ERROR);
}





void GLRenderTarget_Legacy::Sync() {
    if (!dirty) return;

    assert(glGetError() == GL_NO_ERROR);
    GLint curTex;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &curTex);

    glBindTexture(GL_TEXTURE_2D, colorTex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, w, h, 0);

    glBindTexture(GL_TEXTURE_2D, depthTex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, w, h, 0);

    dirty = false;

    glBindTexture(GL_TEXTURE_2D, curTex);
    assert(glGetError() == GL_NO_ERROR);
}


void GLRenderTarget_Legacy::Invalidate() {
    dirty = true;
}


void GLRenderTarget_Legacy::GetRawData(uint8_t * data) {
    GLint old;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, old);

}


void GLRenderTarget_Legacy::SetFiltering(bool doIt) {
    GLint oldBinding;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldBinding);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, doIt ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, doIt ? GL_LINEAR : GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, oldBinding);
}

#endif
