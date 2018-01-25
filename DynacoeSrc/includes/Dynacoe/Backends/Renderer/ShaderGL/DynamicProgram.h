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
#ifndef H_DC_BACKENDS_DYNAMIC_PROGRAM
#define H_DC_BACKENDS_DYNAMIC_PROGRAM

#include <Dynacoe/Backends/Renderer/ShaderGL_Multi.h>

namespace Dynacoe {

class TransformManager;
class TextureManager;

// Dynamic rendering manager
class DynamicProgram {
  public:
    virtual ~DynamicProgram(){}
    struct DynamicVertexEx {
        Renderer::DynamicVertex vertex;
        float transform[16];
    };


    // Adds polygon data to be drawn upon next render with the given 
    // transformation
    virtual void Queue(const Renderer::DynamicVertex * v, uint32_t num, DynamicTransformID) = 0;

    // Adds a new transform to use. Expected to be an array of 16 floats
    virtual DynamicTransformID NewTransform(float * matr) = 0;

    // Renders all queued data. This fizzles all transforms as well.
    // returns the number of vertex entries rendered.
    virtual uint32_t Render(GLenum polygon) = 0;

    // Clears the queued data and transforms
    virtual void Clear() = 0;

    // Associates a texture manager with this instance of DynamicProgram
    virtual void AttachTextureManager(TextureManager * t) = 0;


    
};

DynamicProgram * CreateDynamicProgram();

}

#endif
#endif
