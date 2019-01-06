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

#ifndef H_DC_GL_RENDER_TARGET_INCLUDED
#define H_DC_GL_RENDER_TARGET_INCLUDED

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#ifdef DC_SUBSYSTEM_X11
    #if defined DC_BACKENDS_GLESFRAMEBUFFER_X11
        #include <GLES2/gl2.h>
    #else
        #include <GL/glew.h>
    #endif
#else
    #include <glew.h>    
#endif
#include <stdint.h>

namespace Dynacoe {
class GLRenderTarget {
  public:
    virtual ~GLRenderTarget(){}
    
    // Resizes the render target
    virtual void Resize(int, int) = 0;
  
    // Returns the texture handle representing the contents of the drawn resutls
    virtual GLuint GetTexture() = 0;
  
    // Assures that rendering results are drawn to this framebuffer
    // and its contents are written to the texture object 
    virtual void DrawTo() = 0;
  
    // After this call, all pending results are guaranteed to be 
    // written to the textures if the GLRenderTarget was invalidated
    virtual void Sync() = 0;

    // Marks the GLRenderTarget as dirty so that Sync() calls will 
    // force an update.
    virtual void Invalidate() = 0;
  
    // equivalent to Framebuffer.h's equivalent function
    virtual void GetRawData(uint8_t *) = 0;
  
    // enable/disable bilinear filtering on the texture
    virtual void SetFiltering(bool) = 0;

};  

GLRenderTarget * CreateGLRenderTarget();
}



#endif
