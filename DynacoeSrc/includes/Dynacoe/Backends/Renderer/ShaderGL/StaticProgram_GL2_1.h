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

#if ( defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32 )

#ifndef H_DC_STATICPROGRAM_GL2_1_INCLUDED
#define H_DC_STATICPROGRAM_GL2_1_INCLUDED

#include <Dynacoe/Backends/Renderer/ShaderGL_Multi.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/StaticProgram.h>
#include <string>

namespace Dynacoe {



class StaticProgram_GL2_1 : public StaticProgram {
  public:
    StaticProgram_GL2_1();
    
    
    
    bool Set(const char * vertexShader,
              const char * fragShader,
              
              const std::string & name, 
              RenderBuffer * DynacoeView_uniformID, 
              RenderBuffer * Projection_uniformID,
              RenderBuffer * DynacoeTexturePos_uniformID, // 4 components: xy pos, zw dims 0 -> GUT, 
              RenderBuffer * DynacoeTextureInfo_uniformID, // 4 component: x- enabled
              RenderBuffer * DynacoeLighting_uniformID, // 4 components: xyz position, w intensity
              RenderBuffer * DynacoeLightingInfo_uniformID // 4 components: xyz color, w type
    );
    
    std::string GetLog();

    void Run(uint32_t *, uint32_t, RenderBuffer *, RenderBuffer *, RenderBuffer *, GLuint, GLuint);

    
    int MaxLights() { return 128; }
    int MaxTextures() { return 64; }
    
  private:
    bool FailSet();
  

    void UpdateUniforms(RenderBuffer * model, RenderBuffer * material);

    void printShaderLog(int shaderID);
    void printProgramLog(int program);
    
    int progID;
    int vertID;
    int fragID;
    int uniformIndex;

    bool passedTexture;
    int  baseTextureActiveIndex;


    std::string progName;
    std::string log;
    bool incomplete;

    RenderBuffer * viewUniform;
    RenderBuffer * projectionUniform;
    RenderBuffer * textureUniform;
    RenderBuffer * lightingUniform;
    RenderBuffer * textureUniformInfo;
    RenderBuffer * lightingUniformInfo;

    int viewUniform_location;
    int projectionUniform_location;
    int textureInfoUniform_location;
    int lightingDataUniform_location;
    int lightingInfoUniform_location;
    int textureDataUniform_location;

    int lightingHeightUniform_location;
    int textureHeightUniform_location;
    int materialUniform_location;
    int materialDataUniform_location;
    int modelUniform_location;

    int fbTexture_location;
    int baseTexture_location;
    int hasFbTexture_location;

};

StaticProgram * CreateStaticProgram();
}


#endif
#endif
