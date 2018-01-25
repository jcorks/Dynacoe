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

#ifndef H_DC_RENDER_MESH
#define H_DC_RENDER_MESH

#include <Dynacoe/Backends/Renderer/StaticState.h>
#include <Dynacoe/Material.h>
#include <Dynacoe/AssetID.h>
#include <Dynacoe/Mesh.h>
#include <Dynacoe/Components/Node.h>

namespace Dynacoe {
using CameraID = Dynacoe::LookupID;
/**
 * \brief Aspect representing a 3D object that reduces to a Mesh.
 *
 * Mesh s are typically loaded from files or data buffers via Assets::Load.
 * See Mesh documentation for more information.
 */
class RenderMesh : public Component {
  public:



    RenderMesh();
    ~RenderMesh();
    RenderMesh & operator=(const RenderMesh &);
    RenderMesh(const RenderMesh &);

    /// \brief Node of the RenderMesh.
    ///
    Node node;

    /// \brief  Forms the aspect into a 3D mesh based on the stored Mesh asset.
    ///
    void AddMesh(const Mesh &);


    /// \brief Forms the aspect into a 3D mesh via vertcies you specify.
    ///
    /// This is a convenience function for generating your own Mesh instance manually 
    /// The produced mesh is freed when this RenderMesh is destroyed.
    void AddMesh(
        const std::vector<Vector> & vertices,
        const std::vector<uint32_t> & faces,
        const std::vector<Vector> & UVs = std::vector<Vector>(),
        const std::vector<Vector> & normals = std::vector<Vector>(),
        const std::vector<Vector> & userData = std::vector<Vector>()
    );



    /// \brief Retrieves the given mesh instance owned.
    ///
    Mesh & GetMesh(uint32_t);

    /// \brief Returns how many meshes are attached.
    ///
    uint32_t GetMeshCount();

    /// \brief Resets the Aspect.
    ///
    void Clear();


    /// \brief Sets the material
    ///
    Dynacoe::Material & Material();


    


    /// \brief Sets the rendering primitive. The default (and most useful) is Triangle
    ///
    void SetRenderPrimitive(Renderer::Polygon);
    
    /// \brief Returns the current rendering primitive.
    /// 
    Renderer::Polygon GetRenderPrimitive();

    
    
    void RenderSelf(Renderer *);
    void OnDraw();
    void OnStep();
    void OnAttach();
    std::string GetInfo();
  private:
      
    void Alloc();
    void initValues();
    void OnTransformUpdate();
    
    std::vector<Mesh*> meshes;
    Dynacoe::Material mat;
    
    RenderBufferID modelTransform; 
    
    Entity::ID framebufferID;
    Renderer::Polygon prim;
    

};


}

#endif
