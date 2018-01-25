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

#ifndef H_DC_GL_RENDER_TARGET_FBO_INCLUDED
#define H_DC_GL_RENDER_TARGET_FBO_INCLUDED

#include "GLRenderTarget.h"

namespace Dynacoe {
class GLRenderTarget_FBO : public GLRenderTarget {
  public:
    GLRenderTarget_FBO();
    ~GLRenderTarget_FBO();
  
    // Resizes the render target
    void Resize(int, int);
  
    // Returns the texture handle representing the contents of the drawn resutls
    GLuint GetTexture();
  
    // Assures that rendering results are drawn to this framebuffer
    // and its contents are written to the texture object 
    void DrawTo();
  
    // After this call, all pending results are guaranteed to be 
    // written to the textures
    void Sync();
  
    // Marks the GLRenderTarget as dirty so that Sync() calls will 
    // force an update.
    void Invalidate();

    // equivalent to Framebuffer.h's equivalent function
    void GetRawData(uint8_t *);
    
     // enable/disable bilinear filtering on the texture
    void SetFiltering(bool);
  
  private:
    GLuint texture;
    GLuint framebuffer;
    GLuint renderbuffer;
    int w; int h;
};  
}



#endif
