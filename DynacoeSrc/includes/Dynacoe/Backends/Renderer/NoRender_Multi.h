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

#ifndef H_DC_BACKENDS_NORENDER_INCLUDED
#define H_DC_BACKENDS_NORENDER_INCLUDED

#include <Dynacoe/Backends/Renderer/Renderer.h>

namespace Dynacoe {
class NoRenderer : public Renderer {
  public:
    std::string Name(){return "NoRenderer";}
    std::string Version(){return "v1.0";}
    bool Valid();


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
    
    void RenderStatic(StaticState *){}
    void ClearRenderedData(){}
    RenderBufferID GetStaticViewingMatrixID(){return RenderBufferID();}
    RenderBufferID GetStaticProjectionMatrixID(){return RenderBufferID();}

    int AddTexture(int, int, const uint8_t*){return 0;}
    void UpdateTexture(int, const uint8_t *){}
    void RemoveTexture(int tex){}
    void SetTextureFilter(TexFilter){}
    void GetTexture(int, uint8_t*){}
    TexFilter GetTextureFilter(){return TexFilter::NoFilter;}
    int GetTextureWidth(int){return 0;}
    int GetTextureHeight(int) {return 0;}
    int MaxSimultaneousTextures(){return 0;}

    RenderBufferID AddBuffer(float *, int)  {return RenderBufferID();}
    void UpdateBuffer(RenderBufferID, float *, int, int){}
    void ReadBuffer(RenderBufferID, float *, int, int){}
    int BufferSize(RenderBufferID){return 0;}
    void RemoveBuffer(RenderBufferID){}
    

    LightID AddLight(LightType){return LightID();}
    void UpdateLightAttributes(LightID, float *){}
    void EnableLight(LightID, bool doIt){}
    void RemoveLight(LightID){}
    int MaxEnabledLights(){return 0;}
    int NumLights(){return 0;}

    std::string ProgramGetLanguage(){return "A hypothetical shader language";}
    ProgramID ProgramGetBuiltIn(BuiltInShaderMode){return ProgramID();}
    ProgramID ProgramAdd(const std::string&, const std::string &, std::string &){return ProgramID();}


    bool IsSupported(Capability){return false;}
    void SetDrawingMode(Polygon, Dimension, AlphaRule){}
    void GetDrawingMode(Polygon *, Dimension *, AlphaRule *){}
    void AttachTarget(Dynacoe::Framebuffer *){}
    Dynacoe::Framebuffer * GetTarget(){return nullptr;}
    std::vector<Dynacoe::Framebuffer::Type> SupportedFramebuffers(){
        return std::vector<Dynacoe::Framebuffer::Type>({
            Dynacoe::Framebuffer::Type::RGBA_PixelArray,
            Dynacoe::Framebuffer::Type::GLFBPacket
        });
    }

};
}

#endif
