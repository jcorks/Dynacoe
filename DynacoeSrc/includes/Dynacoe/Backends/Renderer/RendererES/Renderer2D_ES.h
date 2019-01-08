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
#ifndef H_DC_BACKENDS_GLES_RENDERER2D_INCLUDED
#define H_DC_BACKENDS_GLES_RENDERER2D_INCLUDED

#include <Dynacoe/Backends/Renderer/Renderer.h>
class Renderer2DData;
namespace Dynacoe {
class Texture_ES;
class Renderer2D {
  public:
    Renderer2D(Texture_ES * textureSource);
    


    

    

    // Adds a new symbolic 2D object and returns its ID. the object may be referred to 
    // within vertices as the object parameter.
    uint32_t Add2DObject();

    // Forgets an object
    void Remove2DObject(uint32_t);

    // Sets the parameters to use for the object
    void Set2DObjectParameters(uint32_t object, Renderer::Render2DObjectParameters);




    // Adds a new vertex and returns its index
    uint32_t Add2DVertex();

    // Removes the object. Uzing its object ID in rendering is probably not a great idea
    void Remove2DVertex(uint32_t object);

    // Sets the data for the vertex
    void Set2DVertex(uint32_t vertex, Renderer::Vertex2D);

    // Gets the data stored for the particular vertex.
    Renderer::Vertex2D Get2DVertex(uint32_t vertex);




    // Queues additional vertices to be drawn sequentially
    void Queue2DVertices(
        const uint32_t * indices,
        uint32_t count
    );

    void Enable2DRenderMode();
    void Disable2DRenderMode();

    // renders all currently queued vertices. This clears the queue upon completion.
    uint32_t Render2DVertices(GLenum drawMode, const Renderer::Render2DStaticParameters &);

    // Clears all requests queued before the last RenderDynamicQueue
    void Clear2DQueue();
  private:
    Renderer2DData * data;
    
};
}


#endif
#endif
