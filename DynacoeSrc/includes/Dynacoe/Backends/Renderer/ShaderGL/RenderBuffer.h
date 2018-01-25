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
#ifndef H_DC_BACKENDS_GL_RENDERBUFFER_INCLUDED
#define H_DC_BACKENDS_GL_RENDERBUFFER_INCLUDED

#include <Dynacoe/Backends/Renderer/ShaderGL_Multi.h>

namespace Dynacoe {
class RenderBuffer {
  public:

    virtual ~RenderBuffer(){}

    // returns the size in bytes of the data store
    virtual int Size() = 0;

    // Redefines the data store with the specified buffer
    virtual void Define(const float * dataSrc, int numElts) = 0;

    // Updates the data of the data store
    virtual void UpdateData(const float * dataSrc, int offset, int numElts) = 0;
    
    // Retrieves the data of the data store.
    virtual void GetData(float * outputData, int offset, int dataSize) = 0;

    // Retrieves a pointer to the data store. This pointer is owned by 
    // the render buffer and should not be freed. The pointer is valid until 
    // the next function call for this RenderBuffer.
    virtual float * GetData() = 0;

    // Sets the type of the render buffer. Can either be GL_ARRAY_BUFFER or GL_UNIFORM_BUFFER
    virtual void SetType(GLenum e) = 0;

    // Generates an openGL buffer object name 
    // that reflects the state of the RenderBuffer at the time that 
    // this function is called. The buffer is valid until 
    // ReclaimIDs() is valled
    virtual GLuint GenerateBufferID() = 0;

    // Invalidates all IDs given from GenerateBufferID() calls.
    // this is not guaranteed to free any client/server memory
    // but most of the time will.
    virtual void ReclaimIDs() = 0;
    
};

// Returns a new render buffer
RenderBuffer * CreateRenderBuffer(bool forceTextureVariant = false);

}

#endif
#endif
