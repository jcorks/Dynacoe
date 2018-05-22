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
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer_Tex.h>
#include <cstring>
#include <cfloat>
#include <algorithm>
using namespace Dynacoe;


/* Hacky alternative to uniform buffer storage thats backwards compatible with OpenGL 2.1 
   and allows for (basically/nearly) arbitrary amounts of static storage.

   Additional work to try to to achieve nearly lossless float transfer has been abandoned for now 
   Now, the values are clamped on [0, 1].

*/

RenderBuffer_Tex::RenderBuffer_Tex() {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 32, 0, GL_RGBA, GL_FLOAT, NULL);
    texH = 32;
    data = new float[64*32*4];
    stateChanged = false;
    size = 0;
    sizeAlloc = 64*32*4;
    
}


RenderBuffer_Tex::~RenderBuffer_Tex() {
    if (data)
        delete[] data;
    glDeleteTextures(1, &tex);
}



int RenderBuffer_Tex::Size() {
    return size;
}


void RenderBuffer_Tex::Define(const float * dataSrc, int numElts) {

    if (sizeAlloc < numElts) {
        delete[] data;
        data = new float[numElts];
        sizeAlloc = numElts;
    }

    if (dataSrc)
        memcpy(data, dataSrc, numElts*sizeof(float));
    
    stateChanged = true;
    size = numElts;
}

void RenderBuffer_Tex::UpdateData(const float * dataSrc, int offset, int numElts) {
    if (!dataSrc || !data) return;
    memcpy(data+offset, dataSrc, sizeof(float)*std::min(size, numElts));
    stateChanged = true;
}

void RenderBuffer_Tex::GetData(float * outputData, int offset, int dataSize) {
    if (!outputData || !data) return;
    memcpy(outputData, data+offset, sizeof(float)*std::min(size ,dataSize));
}

float * RenderBuffer_Tex::GetData() {
    return data;
}


GLuint RenderBuffer_Tex::GenerateBufferID() {
    if (stateChanged) {
        bool texResizeNeeded = false;
        GLuint oldTex;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*) &oldTex);
        glBindTexture(GL_TEXTURE_2D, tex);

        while (texH*64 < size) { texH*=2; texResizeNeeded = true; }
        if (texH*64 > sizeAlloc) {
            float * newData = new float[texH*64];
            memcpy(newData, data, sizeof(float)*size);
            delete[] data;
            data = newData;
            sizeAlloc = texH*64;
        }        


        // normalize input data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, texH, 0, GL_RGBA, GL_FLOAT, data);
    
        glBindTexture(GL_TEXTURE_2D, oldTex);
        stateChanged = false;
    }
    return tex;
}







#endif
