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

#ifndef H_DC_BACKENDS_GLES_RENDERER
#define H_DC_BACKENDS_GLES_RENDERER


#include <Dynacoe/Backends/Renderer/Renderer.h>


namespace Dynacoe {
class GLES2Implementation;
struct GLES2 : public Dynacoe::Renderer {
  public:
    GLES2();
    ~GLES2();
    std::string Name(){return "OpenGL ES 2.0";}
    std::string Version(){return "v1.0";}
    bool Valid(){return ES!=nullptr;}



    // 2D
    void Queue2DVertices(
        const uint32_t * indices,
        uint32_t count
    );
    
    uint32_t Add2DObject();
    void Remove2DObject(uint32_t);
    uint32_t Add2DVertex();
    void Remove2DVertex(uint32_t object);
    void Set2DVertex(uint32_t vertex, Vertex2D);
    Vertex2D Get2DVertex(uint32_t vertex);
    void Set2DObjectParameters(uint32_t object, Render2DObjectParameters);
    void Render2DVertices(const Render2DStaticParameters &);
    void Clear2DQueue();
    // 2D

    

    // static render
    void RenderStatic(StaticState *);
    RenderBufferID GetStaticViewingMatrixID();
    RenderBufferID GetStaticProjectionMatrixID();
    std::string ProgramGetLanguage(){return "GLSL ES 1.0";}
    ProgramID ProgramGetBuiltIn(BuiltInShaderMode);
    ProgramID ProgramAdd(const std::string&, const std::string &, std::string &);
    // static render



    // texture
    int AddTexture(int, int, const uint8_t *);
    void UpdateTexture(int, const uint8_t *);
    void RemoveTexture(int tex);
    void GetTexture(int, uint8_t*);
    void SetTextureFilter(TexFilter);
    TexFilter GetTextureFilter();
    int GetTextureWidth(int);
    int GetTextureHeight(int);
    int MaxSimultaneousTextures();
    // texture
    

    // render buffer
    RenderBufferID AddBuffer(float *, int);
    void UpdateBuffer(RenderBufferID, float *, int, int);
    void ReadBuffer(RenderBufferID, float *, int, int);
    int BufferSize(RenderBufferID);
    void RemoveBuffer(RenderBufferID);
    // render buffer    


    // lighting - TODO
    LightID AddLight(LightType) {return LightID();}
    void UpdateLightAttributes(LightID, float *) {}
    void EnableLight(LightID, bool doIt){}
    void RemoveLight(LightID){}
    int MaxEnabledLights(){return 0;}
    int NumLights(){return 0;}
    // lighting


    // drawing engine options
    void SetDrawingMode(Polygon, Dimension, AlphaRule);
    void GetDrawingMode(Polygon *, Dimension *, AlphaRule *);
    // drawing engine options

    void ClearRenderedData();
    bool IsSupported(Capability){return true;}
    void AttachTarget(Dynacoe::Framebuffer *);
    Dynacoe::Framebuffer * GetTarget();
    std::vector<Dynacoe::Framebuffer::Type> SupportedFramebuffers(){
        return std::vector<Dynacoe::Framebuffer::Type>({
            Dynacoe::Framebuffer::Type::GLFBPacket
        });
    }

    
  private:
    GLES2Implementation * ES;
    
    

};
}

#endif
#endif
