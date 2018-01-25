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

#ifndef H_DC_MESH_INLCUDED
#define H_DC_MESH_INLCUDED

#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Material.h>
#include <Dynacoe/Util/RefBank.h>

namespace Dynacoe {

/// \brief 3D object defined by triangle primitives.
///
/// A Mesh contains 2 components: the Mesh itself and a series of MeshObject s.
/// The Mesh itself provides the base data: vertex positions, normals,
/// UVs, you name it. A MeshObject contains the actual application of that source data, expressing what
/// vertices to use, the material to render the vertices with, etc.
/// Using the Mesh - MeshObject reloationship aptly can allow for flexible
/// control over performance vs. memory cost.
class Mesh {
  public:
    Mesh();
    Mesh(const Mesh &);
    Mesh & operator=(const Mesh &);
    ~Mesh();

    Mesh MakeShallowCopy() const;

    /// \brief Object representing the geometry of a rendered object.
    ///
    /// Each integer element of the face signifies which vertex
    /// is being referred to. Thus, Vertices must be defined before defining
    /// faces and each vertex index must be less than the number of vertices,
    /// or undefined behavior will occur.
    class MeshObject {
      public:

        /// \brief Geometry of the mesh object. Contains indices to the
        /// vertices defined in the Mesh.
        ///
        std::vector<uint32_t> faceList;


    };

    /// \brief The type of vertex attribute.
    ///
    enum class VertexAttribute {
        Position, ///< Position of the vertex. 3 components: xyz.
        Normal,   ///< The normal vector of the vertex. 3 components: xyz.
        UV,       ///< The texture cooridates of the vertex. 2 components: xy.
        UserData  ///< User-defined data. 3 components: xyz.
    };



    /// \brief sets the number of vertices this mesh will hold.
    /// This will pre-emptively allocated space for the mesh
    /// vertices as well.
    ///  @param i Number of vertices this Mesh should hold.
    ///
    void SetVertexCount(uint32_t t);



    /// \brief Explicitly defines the vertices of the mesh.
    ///
    /// This overwrites any previous geometry data. Normal data
    /// is also overwritten to be zero. The vectors contain per-vertex data:
    ///  @param vertexPositions x, y, z of the local vertex
    ///  @param vertexTextureCoordinates x, y of the texture, 0 to 1
    void DefineVertices(VertexAttribute, const std::vector<Dynacoe::Vector> & vertexData);


    void DefineVerticesState(const std::vector<Renderer::StaticVertex> &);

    /// \brief Gets the vertex at the given index.
    ///
    Vector GetVertex(uint32_t index, VertexAttribute) const;


    /// \brief Sets the vertex at the given.
    ///
    void SetVertex(uint32_t index, VertexAttribute, const Vector & p);

    /// \brief Returns the number of vertices.
    ///
    uint32_t NumVertices() const;






    /// \brief  Adds a MeshObject to the Mesh. When drawn, the MeshObject's
    /// faceList will refer to vertices of this mesh.
    ///
    int AddObject(const MeshObject &);
    int AddObject();
    /// \brief Retrieves the i'th mesh object of the mesh.
    ///
    /// If none exists at the specified position
    /// nullptr is returned.
    MeshObject * Get(int i);

    /// \brief Removes the obejct at the speicifed position.
    ///
    void RemoveObject(int);


    /// \brief Returns the number of added objects.
    ///
    int NumObjects();

    /// \brief Returns whether the mesh is a shallow mesh,
    /// mean it does not own its vertices.
    bool IsShallow();

    /// \brief If shallow, the mesh creates is own copy of
    /// vertices, making it no longer shallow. If the Mesh is
    /// not shallow, no action is taken.
    ///
    void MakeUnique();


    void PopulateState(StaticState *, int objectIndex);

    /// \brief Returns a Mesh representing a boring cube.
    ///
    static const Mesh & Basic_Cube();
    static const Mesh & Basic_Sphere();
    static const Mesh & Basic_Pyramid();
    static const Mesh & Basic_Triangle();

    /// \brief Returns a Mesh representing a boring square.
    ///
    static const Mesh & Basic_Square();

  private:

    RenderBufferID vertices;
    std::vector<MeshObject> objs;
    int numElts;
    bool isShallow;


};
}



#endif
