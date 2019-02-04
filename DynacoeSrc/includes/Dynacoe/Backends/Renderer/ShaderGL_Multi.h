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


#ifndef GLINTERFACE_H_INCLUDED
#define GLINTERFACE_H_INCLUDED



#include <Dynacoe/Backends/Renderer/Renderer.h>
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#ifdef DC_BACKENDS_SHADERGL_X11
    #include <GL/glew.h>
#else
    #ifdef _MBCS 
        #include <GL/glew.h>    
    #else
        #include <glew.h>
    #endif
#endif
#include <Dynacoe/Backends/Renderer/ShaderGL/TextureManager.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/Renderer2D.h>
#include <vector>
#include <stack>



namespace Dynacoe {

class RenderBuffer;
class StaticProgram;
class StaticState;
class DynamicProgram;
struct ShaderGLRenderer : public Dynacoe::Renderer {
  public:
    ShaderGLRenderer();

    std::string Name();
    std::string Version();
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
    
    void RenderStatic(StaticState *);
    void ClearRenderedData();
    RenderBufferID GetStaticViewingMatrixID();
    RenderBufferID GetStaticProjectionMatrixID();

    int AddTexture(int, int, const uint8_t *);
    void UpdateTexture(int, const uint8_t *);
    void RemoveTexture(int tex);
    void GetTexture(int, uint8_t*);
    void SetTextureFilter(TexFilter);
    TexFilter GetTextureFilter();
    int GetTextureWidth(int);
    int GetTextureHeight(int);
    int MaxSimultaneousTextures();
    
    RenderBufferID AddBuffer(float *, int);
    void UpdateBuffer(RenderBufferID, float *, int, int);
    void ReadBuffer(RenderBufferID, float *, int, int);
    int BufferSize(RenderBufferID);
    void RemoveBuffer(RenderBufferID);
    

    LightID AddLight(LightType);
    void UpdateLightAttributes(LightID, float *);
    void EnableLight(LightID, bool doIt);
    void RemoveLight(LightID);
    int MaxEnabledLights();
    int NumLights();

    std::string ProgramGetLanguage();
    ProgramID ProgramGetBuiltIn(BuiltInShaderMode);
    ProgramID ProgramAdd(const std::string&, const std::string &, std::string &);


    bool IsSupported(Capability);
    void SetDrawingMode(Polygon, DepthTest, AlphaRule);
    void GetDrawingMode(Polygon *, DepthTest *, AlphaRule *);
    void AttachTarget(Dynacoe::Framebuffer *);
    Dynacoe::Framebuffer * GetTarget();
    std::vector<Dynacoe::Framebuffer::Type> SupportedFramebuffers();
    TextureManager * GetTextureManager() { return texture; }


    double   diagnostic_dynamic_vtex_per_render_accumulated_avg;
    uint32_t diagnostic_dynamic_vtex_per_render_accumulated_avg_ct;
    uint32_t diagnostic_dynamic_vtex_per_render_avg;
    uint32_t diagnostic_dynamic_vtex_per_render_min;
    uint32_t diagnostic_dynamic_vtex_per_render_max;
    uint32_t diagnostic_dynamic_vtex_render_per_frame;
    uint32_t diagnostic_dynamic_vtex_render_last;
    time_t   diagnostic_dynamic_vtex_render_frame_time;
    uint32_t diagnostic_static_object_count_per_second;
    time_t   diagnostic_static_object_count_time;
    uint32_t diagnostic_static_object_count_per_second_last;
    double   diagnostic_static_object_avg_indices_acc;
    uint32_t diagnostic_static_object_avg_indices_ct;
    uint32_t diagnostic_static_object_avg_indices;
    
  private:
    void gl3warning(const char *);
    void gl3fatal(const char *);


    // list of reserved active textures
    void resolveDisplayMode(Renderer::Polygon, Renderer::DepthTest, Renderer::AlphaRule);
    void resolveDisplayMode();
    void onFirstAttach();
    void initGL();
    void initGLBuffers();

    Display * attachedDisplay;
    GLuint * texImageBounds;

    TextureManager * texture;
    Renderer2D * renderer2D;




    GLenum drawMode;

    Framebuffer * framebuffer;

    void prepareTextures(StaticState*);
    void framebufferCheck();

    RenderBufferID  mainProjectionUniform;
    RenderBufferID  mainViewUniform;
    RenderBufferID  mainTextureUniform;
    RenderBufferID  mainTextureUniform2;
    RenderBufferID  mainLightUniform;
    RenderBufferID  mainLightUniform2;

    Dynacoe::Table<RenderBuffer*> buffers;
    Dynacoe::Table<StaticProgram*> shaderPrograms;

    Dynacoe::LookupID basicProgramID;
    Dynacoe::LookupID lightProgramID;
    
    
    

    bool checkSupported();
    bool createContext();

    bool samplebufferSet;
    GLuint lastSamplebuffer;
    
    GLuint framebufferHandle;
    GLuint framebufferW;
    GLuint framebufferH;
    GLuint framebufferTex;
    
    bool valid;

    Polygon curPolygon;
    AlphaRule curAlphaRule;
    DepthTest curDepthTest;


    

    bool lightsDirty;
    float * lightDataSrc;
    float * lightDataSrc2;
    void SyncLightBuffer();
    
    

};
}

#endif
#endif

