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
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer_Tex1D.h>
#include <cstring>
#include <cfloat>
#include <algorithm>
using namespace Dynacoe;


/* Same as RenderBuffer_Tex except is in an easier-accessible 1D texture.

*/

RenderBuffer_Tex1D::RenderBuffer_Tex1D() {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_1D, tex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 64, 0, GL_RGBA, GL_FLOAT, NULL);
    data = new float[64];
    stateChanged = false;
    size = 0;
    length = 64;
    
}


RenderBuffer_Tex1D::~RenderBuffer_Tex1D() {
    if (data)
        delete[] data;
    glDeleteTextures(1, &tex);
}



int RenderBuffer_Tex1D::Size() {
    return size*sizeof(float);
}


void RenderBuffer_Tex1D::Define(const float * dataSrc, int numElts) {

    if (data) delete[] data;
    data = new float[numElts];
    if (dataSrc)
        memcpy(data, dataSrc, numElts*sizeof(float));
    
    stateChanged = true;
    size = numElts;
}

void RenderBuffer_Tex1D::UpdateData(const float * dataSrc, int offset, int numElts) {
    if (!dataSrc || !data) return;
    memcpy(data+offset, dataSrc, sizeof(float)*std::min(size, numElts));
    stateChanged = true;
}

void RenderBuffer_Tex1D::GetData(float * outputData, int offset, int numElts) {
    if (!outputData || !data) return;
    memcpy(outputData, data+offset, sizeof(float)*std::min(size, numElts));
}

float * RenderBuffer_Tex1D::GetData() {
    // reverification
    /*
    static float * dataReal = nullptr;
    if (dataReal) delete[] dataReal;
    dataReal = new float[size];

    GLuint oldTex;
    glGetIntegerv(GL_TEXTURE_BINDING_1D, (GLint*) &oldTex);
    glBindTexture(GL_TEXTURE_1D, tex);
    glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA, GL_FLOAT, dataReal);    
    glBindTexture(GL_TEXTURE_1D, oldTex);

    return dataReal;
    */
    return data;
}


GLuint RenderBuffer_Tex1D::GenerateBufferID() {
    if (stateChanged) {
        bool texResizeNeeded = false;
        GLuint oldTex;
        glGetIntegerv(GL_TEXTURE_BINDING_1D, (GLint*) &oldTex);
        glBindTexture(GL_TEXTURE_1D, tex);

        while (length <= size) { length += 128; texResizeNeeded = true; }

        // normalize input data
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, size/4, 0, GL_RGBA, GL_FLOAT, data);    
        glBindTexture(GL_TEXTURE_1D, oldTex);
        stateChanged = false;
    }
    return tex;
}







#endif
