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

#ifndef H_DC_MATERIAL_INCLUDED
#define H_DC_MATERIAL_INCLUDED


#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Util/RefBank.h>
namespace Dynacoe {
class Camera;
/// \brief Defines how an AspectMesh is visualized.
///
class Material {
  public:
    Material(const Material &);
    Material();
    ~Material();

    
    Material & operator=(const Material &);

    /// \brief THe editable state of the Material
    ///
    struct State {
        State();
        
        /// \brief The ambient color of the material.
        ///
        Color ambient; 
        
        /// \brief The diffuse color of the material.
        ///
        Color diffuse;

        /// \brief The specular color of the material
        ///
        Color specular;

        /// \brief The amount of specular light that should be allowed.
        ///
        float specularAmount;

        /// \brief The amount of diffuse light that should be allowed.
        ///
        float diffuseAmount;

        /// \brief The amount of reflected specular light.
        ///
        float shininess;

        /// \brief Data that depends on the drawing mode. In implementations
        /// that support shaders, this can be used as you see fit.
        ///        
        float userData[32];
        
        bool operator==(const State & other);
    };
    
    /// \brief Public state of the Material.
    ///
    State state;


    /// \brief The shader type alters how the attributes of the Material
    /// are interpreted.
    ///
    enum class CoreProgram {
        Basic,   ///< Default material. No lighting. The color is determined by the Ambient color.
        Lighting ///< Lighting material. The lighting shader is guaranteed to follow and Phong-like shading model.
    };

    enum class TextureSlot {
        Color, ///< Interprets the texture as an RGBA texture.
        Normal,///< Interprets the texture as a normal map. Each 4-pixel component is interpreted as a normal vector, using rgb saturation values as xyz directions respectively. The alpha component is ignored.
        Shiny, ///< Interprets the texture as a shiny map. Each red component of each 4-pixel is interpreted as a shiny value. The bga values are ignored.
    };

    /// \brief Alters the rendering method to a built-in rendering program.
    ///
    void SetProgram(CoreProgram);
    
    /// \brief Alters the rendering method to a shader program.
    ///
    void SetProgram(ProgramID);
    
    /// \brief Adds a texture to be drawn.
    ///
    /// How textures interact with the mesh is dependent on the Material attached with 
    /// SetMaterial(). If the Material is using a Material::CoreProgram, TextureSlot::Color 
    /// will be used as an RGBA texture, TextureSlot::Normal will be used as a lighting normal map,
    /// and TextureSlot::Shiny will be used as a light reflectivity map. For user-made programs using
    /// Shader, the first argument refers to what texture slot you can access the texture from in 
    /// the shader programming language. Refer to Shader for more info. Note that using TextureSlot::Color,
    /// TextureSlot::Normal, and TextureSlot::Shiny is equivalent to calling AddTexture() with 0, 1, and 2 respectively as 
    /// the texture slot argument.
    /// 
    void AddTexture(TextureSlot, AssetID image);
    void AddTexture(uint32_t, AssetID image);
    
    
    /// \brief Sets the given camera as the source framebuffer.
    ///
    /// The source framebuffer is an optional input you can specify 
    /// to utilize the last known visual of the camera as a texture 
    /// source. This can be used in a few different ways. Once its set, 
    /// Material::CoreProgram s will map it as another texture binding.
    /// To disable this, simple SetFramebufferSource again with a 
    /// default CameraID(). If your material uses a custom shader, the shader can access 
    /// if through the Dynacoe_SampleFramebuffer function.
    ///
    void SetFramebufferSource(Camera &);
    
    /// \brief Iterates all textures to the next texture frame
    ///
    void NextTextureFrame();

    // Prepares a static state with attributes of the material
    void PopulateState(StaticState * target);

    std::string Info();
  private:

    ProgramID GetProgramID(CoreProgram);
    Entity::ID framebufferID;
    RenderBufferID buffer;
    ProgramID       type;     
    
    
    std::vector<std::pair<int, int>> texturesRaw;
    std::vector<std::pair<int, AssetID>> texturesSrc;
    int textureFrameCount;
    State previousState;
};
}


#endif
