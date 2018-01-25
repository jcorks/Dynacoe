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

#if (defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32)


#include <Dynacoe/Backends/Renderer/ShaderGL_Multi.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/StaticProgram.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget.h>
#include <Dynacoe/Backends/Renderer/StaticState.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/GLVersionQuery.h>


#include <cstring>
#include <cassert>

#include <Dynacoe/Color.h>
#include <Dynacoe/Backends/Display/Display.h>
#include <Dynacoe/Util/Chain.h>

#include <iostream>





const int DBUFFER_LIGHT_INDEX_LIGHT_DATA      = 256*4;



#include <cstdlib>
#include <iostream>

using namespace std;
using namespace Dynacoe;

ShaderGLRenderer::ShaderGLRenderer() {
    attachedDisplay = nullptr;

    createContext();

    initGL();
    if (!valid) {
        return;
    }

    framebufferHandle = 0;
    framebuffer = nullptr;
    texture =      new TextureManager();
    dynamic =      CreateDynamicProgram();
    dynamic->AttachTextureManager(texture);
    glClearColor(.1, 0, .07, 1.f);
}



void ShaderGLRenderer::SetTextureFilter(TexFilter f) {
    texture->SetFilter(f);
}

Dynacoe::Renderer::TexFilter ShaderGLRenderer::GetTextureFilter(void) {
    return texture->GetFilter();
}

int ShaderGLRenderer::GetTextureWidth(int id) {
    return texture->GetSubTextureBounds(id)[2];
}

int ShaderGLRenderer::GetTextureHeight(int id) {
    return texture->GetSubTextureBounds(id)[3];
}




void ShaderGLRenderer::QueueDynamicVertices(const DynamicVertex * v, uint32_t numVertices, DynamicTransformID id) {
    dynamic->Queue(v, numVertices, id);
    //assert(id);
}

DynamicTransformID ShaderGLRenderer::CacheDynamicTransform(float * f) {
    return dynamic->NewTransform(f);
}

int ShaderGLRenderer::AddTexture(int w, int h, const uint8_t * data) {
    return texture->NewTexture(w, h, (uint8_t*)data);
}



void ShaderGLRenderer::ClearDynamicQueue() {
    dynamic->Clear();
}



void ShaderGLRenderer::SetDrawingMode(Polygon p, Dimension d, AlphaRule a) {
    resolveDisplayMode(p, d, a);
}


void ShaderGLRenderer::GetDrawingMode(Polygon * p, Dimension * d, AlphaRule * a) {
    *p = curPolygon;
    *d = curDimension;
    *a = curAlphaRule;
}

void ShaderGLRenderer::AttachTarget(Framebuffer * f) {
    if (!f) {
        framebuffer = nullptr;
        return;
    }

    if (f == framebuffer) return;

    if (f->GetHandleType() != Dynacoe::Framebuffer::Type::GLFBPacket) return;

    framebuffer = f;



}

Dynacoe::Framebuffer * ShaderGLRenderer::GetTarget() {
    return framebuffer;
}


void ShaderGLRenderer::RenderDynamicQueue() {
    framebufferCheck();
    if (dynamic->Render(drawMode)) {
        (*(GLRenderTarget**)framebuffer->GetHandle())->Invalidate();
    }
}






void ShaderGLRenderer::RemoveTexture(int tex) {
    texture->DeleteTexture(tex);
}


void ShaderGLRenderer::GetTexture(int tex, uint8_t * data) {
    texture->GetTextureData(tex, data);
}


void ShaderGLRenderer::ClearRenderedData() {
    framebufferCheck();
    std::vector<RenderBuffer*> bufs = buffers.List();
    for(RenderBuffer * b : bufs) {
        b->ReclaimIDs();
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}



void ShaderGLRenderer::UpdateTexture(int tex, const GLubyte * newData) {
    texture->UpdateTexture(tex, (uint8_t *)newData);
}

int ShaderGLRenderer::MaxSimultaneousTextures() {
    return shaderPrograms.Find(basicProgramID)->MaxLights();
}


RenderBufferID ShaderGLRenderer::AddBuffer(float * data, int numElements) {
    RenderBuffer * r = CreateRenderBuffer();
    r->Define(data, numElements);
    return buffers.Insert(r);
}

void ShaderGLRenderer::UpdateBuffer(RenderBufferID id, float * newData, int offset, int numElements) {
    buffers.Find(id)->UpdateData(newData, offset, numElements);
}

void ShaderGLRenderer::ReadBuffer(RenderBufferID id, float * newData, int offset, int numElements) {
    buffers.Find(id)->GetData(newData, offset, numElements);
}


int ShaderGLRenderer::BufferSize(RenderBufferID id) {
    return buffers.Find(id)->Size();
}

void ShaderGLRenderer::RemoveBuffer(RenderBufferID id) {
    if (!id.Valid()) return;
    delete buffers.Find(id);
    buffers.Remove(id);
    return;
}


void ShaderGLRenderer::RenderStatic(StaticState * obj) {
    framebufferCheck();
    if (!(obj->indices && obj->indices->size())) return;
    ProgramID program = obj->program;
    RenderBufferID vertexID = obj->vertices;
    static StaticProgram * progRef            = nullptr;
    static RenderBuffer * uniformData     = nullptr;
    static RenderBuffer * materialData    = nullptr;
    static RenderBuffer * modelData       = nullptr;
    static RenderBuffer * vertexData      = nullptr;


    materialData = buffers.Find(obj->materialData);
    modelData    = buffers.Find(obj->modelData);
    vertexData   = buffers.Find(vertexID);
    progRef      = shaderPrograms.Find(program);

    texture->ComputeTextureBindingData(
        *obj->textures,
        buffers.Find(mainTextureUniform),
        buffers.Find(mainTextureUniform2)
    );
    SyncLightBuffer();




    materialData->SetType(GL_UNIFORM_BUFFER);
    modelData   ->SetType(GL_UNIFORM_BUFFER);
    vertexData  ->SetType(GL_ARRAY_BUFFER);


    GLRenderTarget * t = nullptr;
    if (obj->samplebuffer) {
        t = *(GLRenderTarget **)obj->samplebuffer->GetHandle();
        t->Sync();
    }


    progRef->Run(&(*obj->indices)[0],
                    obj->indices->size(),
                 vertexData  ,
                 materialData,
                 modelData   ,
                 texture->GetTexture(),
                 t ? t->GetTexture()
                  : 0);
    if (framebuffer)
        (*(GLRenderTarget**)framebuffer->GetHandle())->Invalidate();


}


std::string ShaderGLRenderer::ProgramGetLanguage() {
    if (GLVersionQuery(GL_Version3_1))
        return "GLSL 1.4";
    if (GLVersionQuery(GL_Version3_0 | GL_UniformBufferObject))
        return "GLSL 1.3";
    if (GLVersionQuery(GL_Version2_1))
        return "GLSL 1.2";
}


ProgramID ShaderGLRenderer::ProgramGetBuiltIn(BuiltInShaderMode b) {
    switch(b) {
        case BuiltInShaderMode::BasicShader:   return basicProgramID;
        case BuiltInShaderMode::LightMaterial: return lightProgramID;
    }
}






RenderBufferID ShaderGLRenderer::GetStaticViewingMatrixID() {
    return mainViewUniform;
}

RenderBufferID ShaderGLRenderer::GetStaticProjectionMatrixID() {
    return mainProjectionUniform;
}


class LightData {
  public:
    LightData(ShaderGLRenderer::LightType t) {
        srcType = t;

        memset(data, 0, sizeof(float)*7);
        SetType();
        data[7] = 1.f;
    }



    float * GetData() {
        return data;
    }

    bool operator<(const LightData & other) const {
        if (data[0] < other.data[0]) return true;
        return false;
    }



    void Enable(bool src) {
        if (src) {
            SetType();
        } else {
            data[0] = 0;
        }
    }

  private:
    void SetType() {
        switch(srcType) {
          case ShaderGLRenderer::LightType::Point:
            data[0] = .05;
            break;
          case ShaderGLRenderer::LightType::Directional:
            data[0] = .15;
            break;
          case ShaderGLRenderer::LightType::Spot:
            data[0] = .25;
            break;
          default:
            data[0] = 0;
            break;
        }
    }

    ShaderGLRenderer::LightType srcType;


    float data[8];


};

class CompareLightData {
  public:
    bool operator()(LightData * const & a, LightData * const & b) {
        return *a < *b;
    }
};

static std::set<LightData*, CompareLightData> lightSet;
static Table<LightData*> lightTable;




LightID ShaderGLRenderer::AddLight(LightType type) {
    LightData * newLight = new LightData(type);
    lightSet.insert(newLight);
    lightsDirty = true;
    return lightTable.Insert(newLight);
}



void ShaderGLRenderer::UpdateLightAttributes(LightID id, float * data) {
    if (!lightTable.Query(id)) return;
    LightData * lightData = lightTable.Find(id);

    memcpy(lightData->GetData()+1, data, sizeof(float)*7);
    RenderBuffer * buf = buffers.Find(mainLightUniform);
    lightsDirty = true;
}

void ShaderGLRenderer::SyncLightBuffer() {
    if (!lightsDirty) return;
    if (!lightDataSrc) {
        lightDataSrc  = new float[MaxEnabledLights()*4];
        lightDataSrc2 = new float[MaxEnabledLights()*4];
    }

    uint32_t i = 0;
    auto iter = lightSet.begin();
    LightData * light;
    while(i < MaxEnabledLights() && iter != lightSet.end()) {
        light = *iter;
        // pos
        lightDataSrc[i*4+0] = light->GetData()[1];
        lightDataSrc[i*4+1] = light->GetData()[2];
        lightDataSrc[i*4+2] = light->GetData()[3];

        // intensity
        lightDataSrc[i*4+3] = light->GetData()[7];



        // color
        lightDataSrc2[i*4+0] = light->GetData()[4];
        lightDataSrc2[i*4+1] = light->GetData()[5];
        lightDataSrc2[i*4+2] = light->GetData()[6];

        // type
        lightDataSrc2[i*4+3] = light->GetData()[0];


        i++; iter++;
    }
    lightDataSrc2[i*4+3] = 0.f;


    RenderBuffer * buf = buffers.Find(mainLightUniform);
    buf->UpdateData(lightDataSrc, 0, i*4*sizeof(float));

                   buf = buffers.Find(mainLightUniform2);
    buf->UpdateData(lightDataSrc2, 0, i*4*sizeof(float));
    lightsDirty = false;
}

void ShaderGLRenderer::EnableLight(LightID id, bool doIt) {
    if (!lightTable.Query(id)) return;
    LightData * lightData = lightTable.Find(id);

    lightData->Enable(doIt);
}

void ShaderGLRenderer::RemoveLight(LightID id) {
    if (!lightTable.Query(id)) return;
    LightData * lightData = lightTable.Find(id);
    lightTable.Remove(id);
    lightSet.erase(lightData);
    delete lightData;
}


int ShaderGLRenderer::MaxEnabledLights() {
    return shaderPrograms.Find(basicProgramID)->MaxLights();
}

int ShaderGLRenderer::NumLights() {
    return lightSet.size();
}








bool ShaderGLRenderer::Valid() {
    return valid;
}

vector<Dynacoe::Framebuffer::Type> ShaderGLRenderer::SupportedFramebuffers() {
    return std::vector<Dynacoe::Framebuffer::Type>({
        Dynacoe::Framebuffer::Type::GLFBPacket
    });
}



std::string ShaderGLRenderer::Name() {return "Shader OpenGL";}
std::string ShaderGLRenderer::Version() {return "v1.0 (OpenGL 2.0/3.1)";}


std::string ShaderGLRenderer::RunCommand(const std::string & command, uint8_t * data = nullptr) {
    if (command == "update") {
        glFinish();
    }
    return "";
}


bool ShaderGLRenderer::IsSupported(Capability) {
    return true;
}



#endif
