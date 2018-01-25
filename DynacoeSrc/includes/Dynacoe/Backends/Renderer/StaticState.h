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

#ifndef H_DC_BACKENDS_RENDERER_STATIC_STATE
#define H_DC_BACKENDS_RENDERER_STATIC_STATE

#include <Dynacoe/Backends/Renderer/Renderer.h>

namespace Dynacoe {



// conventional rendering object for working with statically
// drawn objects and information.

struct StaticState {
    
    // Vertices points to a renderbuffer containing all the vertex dat  a pertinent to the RenderObject.
    // Each vertex consists of:
    //      3-components for position,
    //      3-components for the normal.
    //      2-copmonents for UVs (texture coordinates)
    //      4-components for user-defined data. Meant to be unitlized with custom programs.
    RenderBufferID vertices;
    
    // Specifies how to render the vertices
    ProgramID program;

    // material matrix , 48-components containing
    // lighting material and arbitrary data setup.
    RenderBufferID materialData;  

    // local transform matrix (scale + rotation), 32-component
    // Normal matrix (inverse transpose of modelData)
    RenderBufferID modelData;  

    // the source framebuffer optionally accessible 
    // during rendering. If the samplebuffer is null,
    // the no source framebuffer will be made available
    Framebuffer * samplebuffer;


    // specifies the textures to be used. Each pair is a slot referred to by 
    // each program and the ID of the texture.
    std::vector<std::pair<int, int>> * textures;


    /* Static index list */
    // Index lists are used to build the geometry of the StaticState.
    // Each index refer to the index of the vertex to be used to build the triangle.
    // every 3 vertices, a triangle is formed. If a multiple of three is not used,
    // the behavior is implementation defined.
    std::vector<uint32_t> * indices;
    


};






}



#endif
