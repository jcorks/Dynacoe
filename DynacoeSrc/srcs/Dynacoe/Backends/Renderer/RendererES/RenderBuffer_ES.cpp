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
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer_ES.h>
#include <cstring>
#include <iostream>
#include <Dynacoe/Util/Chain.h>
#include <cassert>


using namespace Dynacoe;
using namespace std;

static int ctr = 0;

RenderBuffer::RenderBuffer() {
    glGenBuffers(1, &glID); 
    size = 0;
    
    data = nullptr;
    type = GL_ARRAY_BUFFER;
    //cout << "[RenderBuffer]: Instantiating buffer" << endl;
}

RenderBuffer::~RenderBuffer() {
    if (type == GL_ARRAY_BUFFER)
        glDeleteBuffers(1, &glID);
    //cout << "[RenderBuffer]: Deleting buffer" << endl;
    if (data) delete[] data;
}



int RenderBuffer::Size() {
    return size;
}

void RenderBuffer::Define(const float * dataSrc, int numElts) {
    assert(glGetError() == GL_NO_ERROR);
    size = numElts * sizeof(float);
    if (type == GL_ARRAY_BUFFER) {
        glBindBuffer(GL_ARRAY_BUFFER, glID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*numElts, dataSrc, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }    

    assert(glGetError() == GL_NO_ERROR);
    //if (data == dataSrc && data != nullptr) return;
    if (data) delete[] data;
    data = new float[numElts];
    if (data && dataSrc)
        memcpy(data, dataSrc, sizeof(float)*numElts);
}

void RenderBuffer::UpdateData(const float * dataSrc, int offset, int numElts) {

    assert(glGetError() == GL_NO_ERROR);

    if ((offset+numElts) > size/sizeof(float)) {
        cout << "Updated past buffer store..." << endl;
        return;
    }
    memcpy(data+(offset), dataSrc, numElts*sizeof(float));
  

    if (type == GL_ARRAY_BUFFER) {
        // Better, but still slow.
        glBindBuffer(GL_ARRAY_BUFFER, glID); 
        //glBufferData(type, size, data, GL_DYNAMIC_DRAW);

        
        glBufferSubData(
            GL_ARRAY_BUFFER, 
            offset*sizeof(float), 
            numElts*sizeof(float),
            dataSrc
        );
        


        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }    

    assert(glGetError() == GL_NO_ERROR);

}

void RenderBuffer::GetData(float * outputData, int offset, int numElts) {
    if (data)
        memcpy(outputData, data+(offset), numElts*sizeof(float));
}

float * RenderBuffer::GetData() { 
    return data;
}

void RenderBuffer::SetType(GLenum e) {

}

GLuint RenderBuffer::GenerateBufferID() {
    return glID;
}


void RenderBuffer::SetOffline() {
    if (type == GL_ARRAY_BUFFER) {
        glDeleteBuffers(1, &glID);
        type = 0;
    }
}
#endif

