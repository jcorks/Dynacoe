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

#if (DC_BACKENDS_GLES_X11)

#ifndef H_DC_BACKENDS_STATICRENDERER
#define H_DC_BACKENDS_STATICRENDERER

#include <GLES2/gl2.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer_ES.h>
#include <Dynacoe/Backends/Renderer/RendererES/Texture_ES.h>

namespace Dynacoe {
class StaticRenderer_Data;
class StaticRenderer {
  public:
    StaticRenderer(Texture_ES *);

    // adds a new usable graphics program.
    ProgramID ProgramAdd(
        const std::string & vert, 
        const std::string & frag, 
        std::string & log
    );


    // Renders the given static state.
    // The static state represents a dynamic set of attributes that should 
    // define the drawing action in and of itself.
    void Render(StaticState *);

    // Returns the view matrix used for static rendering operations
    RenderBufferID GetViewingMatrixID();

    // Returns the projection matrix used for static rendering.
    RenderBufferID GetProjectionMatrixID();


    // Gets the built-in program asked for. These are typically basic
    ProgramID ProgramGetBuiltIn(Renderer::BuiltInShaderMode);

    


  private:
    StaticRenderer_Data * ES;


};
}


#endif
#endif
