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

#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer_GL2_1.h>
#include <cstring>
#include <iostream>
#include <Dynacoe/Util/Chain.h>
#include <cassert>


using namespace Dynacoe;
using namespace std;

static int ctr = 0;

RenderBuffer_GL2_1::RenderBuffer_GL2_1() {
    glGenBuffers(1, &glID); 
    size = 0;
    
    type = GL_ARRAY_BUFFER;
    data = nullptr;
    //cout << "[RenderBuffer_GL2_1]: Instantiating buffer" << endl;
}

RenderBuffer_GL2_1::~RenderBuffer_GL2_1() {
    if (type == GL_ARRAY_BUFFER) 
        glDeleteBuffers(1, &glID);
    //cout << "[RenderBuffer_GL2_1]: Deleting buffer" << endl;
    if (data) delete[] data;
}



int RenderBuffer_GL2_1::Size() {
    return size;
}

void RenderBuffer_GL2_1::Define(const float * dataSrc, int numElts) {
    assert(glGetError() == GL_NO_ERROR);
    if (type == GL_ARRAY_BUFFER) {
        glBindBuffer(type, glID);
        glBufferData(type, sizeof(float)*numElts, dataSrc, GL_STATIC_DRAW);
        glGetBufferParameteriv(type, GL_BUFFER_SIZE, &size);
        glBindBuffer(type, 0);
        size /= sizeof(float);
    } else {
        size = numElts * sizeof(float);
    }

    assert(glGetError() == GL_NO_ERROR);
    if (data == dataSrc && data != nullptr) return;
    if (data) delete[] data;
    data = new float[numElts];
    if (dataSrc)
        memcpy(data, dataSrc, sizeof(float)*numElts);
}

void RenderBuffer_GL2_1::UpdateData(const float * dataSrc, int offset, int numElts) {

    assert(glGetError() == GL_NO_ERROR);

    if ((offset+numElts) > size) {
        cout << "Updated past buffer store..." << endl;
        return;
    }
    memcpy(data+(offset), dataSrc, numElts*sizeof(float));
  

    // Better, but still slow.
    if (type == GL_ARRAY_BUFFER) {
        glBindBuffer(type, glID); 

        float * stored = (float*)glMapBuffer(type, GL_WRITE_ONLY);
        memcpy(stored+(offset), dataSrc, numElts*sizeof(float));
        glUnmapBuffer(type);

        glBindBuffer(type, 0);
    }
    

    assert(glGetError() == GL_NO_ERROR);

}

void RenderBuffer_GL2_1::GetData(float * outputData, int offset, int dataSize) {
    if (data)
        memcpy(outputData, data+(sizeof(float)*offset), dataSize*sizeof(float));
}

float * RenderBuffer_GL2_1::GetData() { 
    return data;
}

void RenderBuffer_GL2_1::SetType(GLenum e) {
    if (e != type) {
        type = e;
        if (e == GL_ARRAY_BUFFER) {
            glGenBuffers(1, &glID);
            Define(data, size); 
        } else if (e == GL_UNIFORM_BUFFER) {
            glDeleteBuffers(1, &glID);
            glID = 0;
        }
    }
}

GLuint RenderBuffer_GL2_1::GenerateBufferID() {
    return glID;
}

void RenderBuffer_GL2_1::ReclaimIDs() {
    
}

#endif

