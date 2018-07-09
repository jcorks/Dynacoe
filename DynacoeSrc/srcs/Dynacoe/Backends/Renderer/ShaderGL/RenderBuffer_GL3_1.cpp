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

#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer_GL3_1.h>
#include <cstring>
#include <iostream>
#include <Dynacoe/Util/Chain.h>
#include <cassert>

// So. Synchronization is the newest relevant issue.
// Work arounds so far:
// 1) Avoid glSubBufferData: reform the store every time (keep a cpu-copy of data around to form from it when updating)
//      Well funnily enough, this does nothing different from glSubBufferData.
//      it looks as if it reuses the old store
// 2) New buffer every time 
//      Each Buffer is actually a collection of buffer objects 
//          each new one has 4 or so new buffers. This is its "buffer pool"
//          A buffer from the pool is in use when its object ID is retrieved.
//          until the frame is drawn (so we need a frame refresh callback) 
//      when a buffer is requested to be changed:
//          Take an unused buffer from the the pool
//          copy buffer data from old current gl buffer and make mods
//          that new buffer is now the current buffer 
//
//      querying the id returns the curent buffer, whcih again, may change at any time.
//      Other notes on this implementation:
//           probably best to keep around a cpu-copy of the data. buffer copying is 3.1+ only.
//
// 3) New buffer every tiem 2.0
//      CPU copies only.
//      when request an ID:
//          Check to see if we have a free buffer around.
//          if so, 
using namespace Dynacoe;
using namespace std;

static int ctr = 0;
static const int RENDER_BUFFER_GL3_CLAIMED_BUFFER_COUNT = 10;

RenderBuffer_GL3_1::RenderBuffer_GL3_1() {
    if (RENDER_BUFFER_GL3_CLAIMED_BUFFER_COUNT) {
        GLuint bufs[RENDER_BUFFER_GL3_CLAIMED_BUFFER_COUNT];
        glGenBuffers(RENDER_BUFFER_GL3_CLAIMED_BUFFER_COUNT, bufs); 

        for(uint32_t i = 0; i < RENDER_BUFFER_GL3_CLAIMED_BUFFER_COUNT; ++i) {
            deadIDs.push(bufs[i]);
        }
    }
    size = 0;
    stateChanged = true;
    type = GL_ARRAY_BUFFER;
    data = nullptr;
    //cout << "[RenderBuffer_GL3_1]: Instantiating buffer" << endl;
}

RenderBuffer_GL3_1::~RenderBuffer_GL3_1() {
    ReclaimIDs();
    GLuint id;
    while(!usedIDs.empty()) {
        id = usedIDs.top();
        glDeleteBuffers(1, &id);
    }
    //cout << "[RenderBuffer_GL3_1]: Deleting buffer" << endl;
    if (data) delete[] data;
}



int RenderBuffer_GL3_1::Size() {
    return size;
}

void RenderBuffer_GL3_1::Define(const float * dataSrc, int numElts) {
    size = numElts*sizeof(float);
    if (data) delete[] data;
    data = new float[numElts];
    if (dataSrc)
        memcpy(data, dataSrc, sizeof(float)*numElts);
    stateChanged = true;
}

void RenderBuffer_GL3_1::UpdateData(const float * dataSrc, int offset, int numElts) {
    if ((offset+numElts) > size/sizeof(float)) {
        cout << "Updated past buffer store..." << endl;
        return;
    }
    memcpy(data+(offset), dataSrc, numElts*sizeof(float));
    stateChanged = true;
}

void RenderBuffer_GL3_1::GetData(float * outputData, int offset, int numElts) {
    if (data)
        memcpy(outputData, data+offset, numElts*sizeof(float));
}

float * RenderBuffer_GL3_1::GetData() {
    return data;
}

void RenderBuffer_GL3_1::SetType(GLenum e) {
    type = e;
}

GLuint RenderBuffer_GL3_1::GenerateBufferID() {
    if (!stateChanged) return lastID;
    
    // recycling may have issues?
    
    GLuint glID;
    if (!deadIDs.empty()) {
        glID = deadIDs.top();
        deadIDs.pop();
    } else {
        glGenBuffers(1, &glID);
    }
    usedIDs.push(glID);
    glBindBuffer(type, glID);
    glBufferData(type, size, data, GL_STATIC_DRAW);
    glBindBuffer(type, 0);

    assert(glGetError() == GL_NO_ERROR);
    lastID = glID;
    
    
    /*
    GLuint glID;
    glGenBuffers(1, &glID);
    glBindBuffer(type, glID);
    glBufferData(type, size, data, GL_STATIC_DRAW);
    glBindBuffer(type, 0);
    glDeleteBuffers(1, &lastID);
    lastID = glID;
    */
    stateChanged = false;
    return glID;
}

void RenderBuffer_GL3_1::ReclaimIDs() {
    
    while(deadIDs.size() < RENDER_BUFFER_GL3_CLAIMED_BUFFER_COUNT) {
        deadIDs.push(usedIDs.top());
        usedIDs.pop();
    }
    GLuint glID;
    bool inUse = false;
    while(usedIDs.size()) {
        glID = usedIDs.top();
        usedIDs.pop();
        
        // Do not toss out overflow ID if it is in use as the lastID.
        if (glID != lastID) {
            glDeleteBuffers(1, &glID);
        } else {
            inUse = true;
        }
    }
    if (inUse) 
        usedIDs.push(lastID);
}


#endif

