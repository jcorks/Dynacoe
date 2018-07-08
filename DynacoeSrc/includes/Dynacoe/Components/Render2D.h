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

#ifndef H_DC_RENDER2D_INLCUDED
#define H_DC_RENDER2D_INLCUDED

#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <Dynacoe/Component.h>
#include <Dynacoe/Components/Node.h>
namespace Dynacoe {


/// \brief Base component to render a 2D object.
///
class Render2D : public Component {
  public:

    /// \brief A RenderMode specifies how the aspect should be expressed.
    ///
    enum class RenderMode {
        Normal, ///<This is the default. Colors are interpreted as you would normally expect
                /// with the alpha channle being expressed as a transparency value.
        Translucent, ///<Translucency in Dynacoe is a way of expressing transparency. When an Aspect is
                     /// translucent, the aspect's colors are added to whatever is drawn underneath it.
                     /// This is referred to in some places as additive blending.
        None, ///< If an aspect's render mode is None, it will not be drawn.
    };

    /// \brief  The visual mode for rendered vertices
    ///
    /// See Renderer.h
    RenderMode mode;

    /// \brief The Node for the visual. Z is ignored.
    ///
    Node node;

    /// \brief Sets whether drawing calculations from the node should be interpreted as absolute 
    /// pixel values on the display or should be within the hierarchy. The default is false.
    ///
    bool absolute;

    /// \brief Returns the raw vertices compiled for the renderable object 
    ///
    /// See Renderer.h
    std::vector<Renderer::Vertex2D> GetVertices() const;    
    
    uint32_t GetObjectID() const;
    
    const std::vector<uint32_t> & GetVertexIDs() const;
    
    /// \brief Returns the polygon to be used when drawing the vertices 
    ///
    /// See Renderer.h
    Renderer::Polygon GetPolygon() const;
    ~Render2D();

  protected:
    Render2D(const std::string &);

    void SetVertices(const std::vector<Renderer::Vertex2D> &);
    void SetPolygon(Renderer::Polygon);


  private:
     std::vector<uint32_t> vertexSrc;
     int objectID;
     
     Renderer::Polygon polygon;
     void OnAttach();
     void OnStep();
};

}

#endif
