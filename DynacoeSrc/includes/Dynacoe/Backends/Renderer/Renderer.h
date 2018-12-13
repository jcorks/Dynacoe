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

#ifndef DC_Renderer_INTERFACE_H_INCLUDED
#define DC_Renderer_INTERFACE_H_INCLUDED



/* Renderer,
   A light renderer interface for Dynacoe's rendering layer.
   In this renderer abstraction, lighting and shaders are extensions
   and, while typically implemented in GPU-oriented backends, is not
   core functionality and may not be provided. In such a case, the
   extension functions can be called, but should not affect the renderer state.


   Johnathan Corkery, 2015

*/
#include <Dynacoe/Util/Table.h>
#include <Dynacoe/Backends/Display/Display.h>
#include <string>
#include <unordered_map>
#include <cstdint>


namespace Dynacoe {
class Color;
class Display;
class StaticState;
struct RendererImpl;

using RenderBufferID     = Dynacoe::LookupID;
using ProgramID           = Dynacoe::LookupID;
using LightID            = Dynacoe::LookupID;
using DynamicTransformID = int;


class Renderer : public Backend {
  public:
    static const int MINIMUM_LIGHT_COUNT = 32;
    static const int MINIMUM_TEXTURE_BINDING_COUNT = 32;

    // Struct representing a dynamic vertex state.
    // Dynamic vertices do not support lighting.
    struct Vertex2D {
        Vertex2D(){}
        Vertex2D(float x_, float y_,
                      float r_, float g_, float b_, float a_,
                      float tex, float tx, float ty)
            : x(x_), y(y_),
              r(r_), g(g_), b(b_), a(a_), useTex(tex), texX(tx), texY(ty) {}
        Vertex2D(float x_, float y_,
                      float r_, float g_, float b_, float a_)
            : x(x_), y(y_), 
              r(r_), g(g_), b(b_), a(a_), useTex(-1) {}

        Vertex2D(float x_, float y_,
                      float tex, float tx, float ty)
            : x(x_), y(y_),
              r(1.f), g(1.f), b(1.f), a(1.f),
              useTex(tex), texX(tx), texY(ty) {}
        float x, y;                           // vertex position
        float r, g, b, a; // color, scale from 0.f to 1.f (red, green, blue, and alpha)
        float texX, texY;                     // texture coordinates (0, 0 is topleft)
        float useTex;                         // if not used, set to -1, else float form of texture id
        float object;                         // the transform reference object
    };

    // For use with StaticState. See StaticState.h
    struct StaticVertex {
        StaticVertex(){}
        StaticVertex(float x_, float y_, float z_) :
            x(x_), y(y_), z(z_),
            normalX(0), normalY(0), normalZ(0),
            texX(0), texY(0),
            userDefinedData{0, 0, 0, 0}
        {}

        StaticVertex(
            float x_, float y_, float z_,
            float normalX_, float normalY_, float normalZ_,
            float texX_, float texY_,
            float userdata0,
            float userdata1,
            float userdata2,
            float userdata3
        ) :
            x(x_), y(y_), z(z_),
            normalX(normalX_), normalY(normalY_), normalZ(normalZ_),
            texX(texX_), texY(texY_),
            userDefinedData{
                userdata0,
                userdata1,
                userdata2,
                userdata3
            }
        {}


        float x, y, z;
        float normalX, normalY, normalZ;
        float texX, texY;
        float userDefinedData[4];
    };

    enum class TexFilter {
        Linear,
        NoFilter,
    };

    enum class Polygon {
        Triangle,
        Line
    };


    enum class Dimension {
        D_2D,
        D_3D
    };


    enum class AlphaRule {
        Allow,
        PassThrough,
        Opaque,
        Translucent
    };

    enum class Capability {
        Lighting,
        UserShaders

    };

    enum class BuiltInShaderMode {
        // MaterialIDs for render objects should be only 4 floats
        // Each float represents a color, all 4 corresponding to RGBA.
        // No lighting is enabled
        BasicShader,

        // The objects output color is determined by
        // a phong shading-like algorithm that utilizes any Light objects
        // and set material for the object
        LightMaterial,


    };


    enum class LightType {
        Point,
        Directional,
        Spot
    };
    
    
    struct Render2DStaticParameters {
        float contextWidth;
        float contextHeight;
        
        float * contextTransform;
    };

    struct Render2DObjectParameters {
        
        // transform
        float data[16];
        
    };











    /* Requesting Geometry */

    // The general use case for 2D geometry is we will draw many 
    // small objects with unique tranform properties. Thus, a stati crendering 
    // approach is less favorable as that would lead to more frequent draws.
    // Render2DVertices, using user-maintained global vertices, is a more 
    // performant option for drawing specifically 2D vertices.


    virtual void Queue2DVertices(
        const uint32_t * indices,
        uint32_t count
    ) = 0;
    
    // Adds a new 2D object and returns its ID.
    virtual uint32_t Add2DObject() = 0;
    
    // Removes a 2D object of the given ID
    virtual void Remove2DObject(uint32_t) = 0;
    
    // Adds a new vertex 
    virtual uint32_t Add2DVertex() = 0;
    
    virtual void Remove2DVertex(uint32_t object) = 0;
    
    virtual void Set2DVertex(uint32_t vertex, Vertex2D) = 0;
    
    virtual Vertex2D Get2DVertex(uint32_t vertex) = 0;
    
    virtual void Set2DObjectParameters(uint32_t object, Render2DObjectParameters) = 0;

    virtual void Render2DVertices(const Render2DStaticParameters &) = 0;

    // Clears all requests queued before the last RenderDynamicQueue
    virtual void Clear2DQueue() = 0;








    // Unlike Dynamic requests, RenderStatic render execution times are entirely up
    // to the renderer and backend. The only guarantee is that the object will render
    // on the committed display. The benefit of this mode is utilizing advanced shaders
    // and the ability to utilize specialized renderer storage, which will almost always
    // yield better performance for static rendering over dynamic rendering. Vertices
    // here are transformed by the renderer.
    virtual void RenderStatic(StaticState *) = 0;

    // Resets the renderer's render space to an initial state.
    virtual void ClearRenderedData() = 0;

    // Returns the transformation matrix buffer IDs for static rendering.
    // All RenderStatic positional vertices are multiplied by the Viewing and
    // projection matrices respectively. Once determined by the renderer,
    // these ID's values will not change per instance.
    virtual RenderBufferID GetStaticViewingMatrixID() = 0;
    virtual RenderBufferID GetStaticProjectionMatrixID() = 0;







    /* Texture Management */

    // Stores texture data so that it may be accessed in the drawing structure.
    // Returns the textureIndex. Passing a nullptr as the rgbaTextureData
    // will allocate space for the texture, keep its contents as undefined
    virtual int AddTexture(int w, int h, const uint8_t * rgbaTextureData) = 0;

    // redefines the contents of an existing image without
    // deleting it and re-adding it. The image is expected to be the
    // same dimensions as the original.
    virtual void UpdateTexture(int tex, const uint8_t * newData) = 0;

    // Flags a texture for deletion. Deletion is not guaranteed to be immediate, nor does it guarantee
    // freeing of GPU memory; however, it does guarantee the possibility of adding one or more
    // textures of its size or less.
    virtual void RemoveTexture(int tex) = 0;

    // Populates the given array with the texture data.
    // THe input buffer should be allocated to GetTextureWidth()*GetTextureHeight()*4
    // As with input buffered data, the format is RGBA with no padding.
    virtual void GetTexture(int tex, uint8_t *) = 0;


    // sets/ gets the stretching filter to be used during the next request resolution
    virtual void SetTextureFilter(TexFilter) = 0;
    virtual TexFilter GetTextureFilter() = 0;


    virtual int GetTextureWidth(int tex) = 0;
    virtual int GetTextureHeight(int tex) = 0;

    // Returns the maximum number of textures that can be referred to
    // by a StaticObject when rendering.
    virtual int MaxSimultaneousTextures() = 0;





    /* RenderBuffer storage */

    // Allocates a new data store. It is not guaranteed where this data store is kept,
    // but if you are using a backend with hardware support, it is likely in VRAM.
    // If the allocation fails in some way, the id returned will be invalid
    virtual RenderBufferID AddBuffer(float * data, int numElements) = 0;

    // Updates numElements elements of the buffer starting at the offset'th element
    // No check is made to see if this oversteps the original buffer size. If
    // overstepping occurs, the result is undefined.
    virtual void UpdateBuffer(RenderBufferID bufferID, float * newData, int offset, int numElements) = 0;


    // Populates outputData with the relevant data contained in the buffer. On most implementations
    // this will require expensive communication, so use with caution.
    virtual void ReadBuffer(RenderBufferID bufferID, float * ouputData, int offset, int numELements) = 0;

    // Returns the number of elements of the buffer.
    virtual int BufferSize(RenderBufferID bufferID) = 0;

    // Frees the buffer store.
    virtual void RemoveBuffer(RenderBufferID bufferID) = 0;






    /* Shader Programs */


    // Returns a string denoting the name of the shading language supported.
    // If shaders are not supported, an empty string is returned.
    virtual std::string ProgramGetLanguage() = 0;

    // Compiles and enacts a shader for use with Static Rendering. If the shader fails
    // to compile or is invalid in some way, the id returned is invalid. See StaticState.
    // In any case, log will be populated with information on the building process status.
    virtual ProgramID ProgramAdd(const std::string & vertexSrc, const std::string & fragSrc, std::string & log) = 0;

    // Returns the id referring to a builtin shader.
    virtual ProgramID ProgramGetBuiltIn(BuiltInShaderMode) = 0;






    /* Lighting */


    // Creates a new light. When the light is first created, the state of the light
    // is undefined. (It is expected that you will update the lights attributes
    // before drawing);
    virtual LightID AddLight(LightType) = 0;

    // As the default, the attributes requred are
    //  Position (3-components)
    //  Color (3-components)
    //  Intensity (1 component)
    // Thus, the renderer expects the array passed to have at least 7 components.

    virtual void UpdateLightAttributes(LightID, float *) = 0;


    // Enables or diables the light by default, once added, the light is enabled by default
    virtual void EnableLight(LightID, bool doIt) = 0;

    // Removes all resources associated with the light ID.
    virtual void RemoveLight(LightID) = 0;

    // returns the maximum number of lights supported by the renderer.
    virtual int MaxEnabledLights() = 0;

    // returns the number of lightx currently in use.
    virtual int NumLights() = 0;











    /* Extension management */

    // Returns whether or not the capability is supported on this backend
    // For the Renderer, capabilities are close to "standardized extensions":
    // functionality that isnt necessarily supported by a renderer's backend to
    // function properly, but is usually supported by most modern, complete backends.
    virtual bool IsSupported(Capability) = 0;


    /* Display management */

    // Modifes how all following drawn geometry is handled.
    virtual void SetDrawingMode (
        Polygon,
        Dimension,
        AlphaRule
    ) = 0;

    // Retrieves how all the geometry will be drawn
    virtual void GetDrawingMode (
        Polygon * polygon,
        Dimension * dimension,
        AlphaRule * alhaRule
    ) = 0;


    // The passed framebuffer becomes the destination for all future renderings
    // until a different valid framebuffer is given. If the Type of the given
    // Framebuffer is not one of the types from SupportedFramebuffers, no
    // action is taken. If nullptr is passed, rendering will have no effect.
    virtual void AttachTarget(Framebuffer *) = 0;


    // Returns the current target for renderings. The default is
    // nullptr.
    virtual Dynacoe::Framebuffer * GetTarget() = 0;


    // Returns the framebuffer types that this renderer supports.
    // Should the framebuffer not match one of the given types, the framebuffer
    // attachment will fail
    virtual std::vector<Dynacoe::Framebuffer::Type> SupportedFramebuffers() = 0;
};



}


#endif
