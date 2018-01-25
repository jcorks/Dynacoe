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
#ifndef H_DC_MODEL_INCLUDED
#define H_DC_MODEL_INCLUDED

#include <Dynacoe/Entity.h>
#include <Dynacoe/Modules/Assets.h>

namespace Dynacoe {
class Mesh;
class Material;
class Animation;
/// \brief A 3D object as a collection of meshes and materials.
///
class Model : public Asset {
  public:
    Model(const std::string & str) : Asset(str){}
    ~Model();


    /// \brief creates an Entity with many children that, if Draw()n, 
    /// will express the Model in its entirety
    Entity::ID Create();

    /// \brief Adds a new section of the Model. A Section includes 
    /// a Mesh and a Material. 
    ///
    void AddSection();
                
    /// \brief Returns the number of sections that belong to the Model.
    ///
    uint32_t GetSectionCount() const;

    /// \brief Returns the Mesh of the given section.
    ///
    Mesh & SectionMesh(uint32_t);

    /// \brief Returns the Material of the given section.
    ///
    Material & SectionMaterial(uint32_t);
    
    /// \brief Removes all Sections.
    void Clear();

    // Returns the Animation objects that belong to this actor
    // All are of type Animation
    //std::vector<Animation> animations;



  private:
    
    std::vector<Mesh *> meshes;
    std::vector<Material *> materials;
    //std::vector<Animation> animations;

};

    
    
}


#endif