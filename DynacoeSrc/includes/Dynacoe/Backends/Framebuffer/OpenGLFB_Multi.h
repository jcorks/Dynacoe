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


#if ( defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32 || defined DC_BACKENDS_LEGACYGL_WIN32 || defined DC_BACKENDS_LEGACYGL_X11 || defined DC_BACKENDS_GLESFRAMEBUFFER_X11)
#ifndef H_DC_BACKENDS_OPENGLFB_INCLUDED
#define H_DC_BACKENDS_OPENGLFB_INCLUDED

#include "Framebuffer.h"


// Framebuffer in an openGL context
// (it's pretty much just a wrapper for an OpenGL texture)

namespace Dynacoe {
class GLRenderTarget;
class OpenGLFB : public Dynacoe::Framebuffer {
  public:
    OpenGLFB();
    ~OpenGLFB();

    

    bool GetRawData(uint8_t *);

    
    std::string Name();
    std::string Version();
    bool Valid();


  private:
    GLRenderTarget * rt;

  protected:
    bool OnResize(void *, int, int);
    void OnFilterChange(bool);
};

}

#endif
#endif

