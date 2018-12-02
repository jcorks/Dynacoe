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

#ifndef H_DC_BACKENDS_GLES_RENDERBUFFER_INCLUDED
#define H_DC_BACKENDS_GLES_RENDERBUFFER_INCLUDED


#if (defined DC_BACKENDS_GLES_X11)

#define GL_UNIFORM_BUFFER 1

namespace Dynacoe {
class RenderBuffer {
  public:

    RenderBuffer();
    ~RenderBuffer();

    // returns the size in bytes of the data store
    int Size() ;

    // Redefines the data store with the specified buffer
    void Define(const float * dataSrc, int numElts) ;

    // Updates the data of the data store
    void UpdateData(const float * dataSrc, int offset, int numElts) ;
    
    // Retrieves the data of the data store.
    void GetData(float * outputData, int offset, int numElts) ;

    // Retrieves a pointer to the data store. This pointer is owned by 
    // the render buffer and should not be freed. The pointer is valid until 
    // the next function call for this RenderBuffer.
    float * GetData() ;

    // Sets the type of the render buffer. Can either be GL_ARRAY_BUFFER or GL_UNIFORM_BUFFER
    void SetType(GLenum e) ;

    // Generates an openGL buffer object name 
    // that reflects the state of the RenderBuffer at the time that 
    // this function is called. The buffer is valid until 
    // ReclaimIDs() is valled
    GLuint GenerateBufferID() ;

    // Invalidates all IDs given from GenerateBufferID() calls.
    // this is not guaranteed to free any client/server memory
    // but most of the time will.
    virtual void ReclaimIDs() ;
  private:
    GLuint glID;
    int size;
    GLenum type;
    float * data;
    
};

}

#endif
#endif
