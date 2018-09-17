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
#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Image.h>

#include <ctime>

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


/// command interpretation

class Command_SGR_help : public Interpreter::Command {
  public:
    ShaderGLRenderer * ref;
    Command_SGR_help(ShaderGLRenderer * input) {
        ref = input;
    }
    std::string operator()(const std::vector<std::string> & args) {
        return Chain()
            << "Renderer info:\n\n" 
            << "Name: " << ref->Name() << "\n"
            << "Version: " << ref->Version() << "\n"   
            << "Shader Language: " << ref->ProgramGetLanguage() << "\n\n"
            << "Light count: " << ref->NumLights() << "\n\n"

            << "Dynamic Renderer:\n"
            << "    avg :" << ref->diagnostic_dynamic_vtex_per_render_avg << " vtex per render\n"
            << "    low : " << ref->diagnostic_dynamic_vtex_per_render_min << " vtex per render\n"
            << "    high: " << ref->diagnostic_dynamic_vtex_per_render_max << " vtex per render\n" 
            << "    " << ref->diagnostic_dynamic_vtex_render_last << " draws/s (ideal: ~framerate)\n\n"
            
            << "Static Renderer:\n"
            << "    avg :" << ref->diagnostic_static_object_avg_indices << " vtex per render\n" 
            << "    " << ref->diagnostic_static_object_count_per_second_last << " objects/s (ideal: ~framerate)\n\n"
        ;
    }
    
    std::string Help() const {return "";}
        
};

// retrives available texture information
class Command_SGR_texture : public Interpreter::Command {
  public:
    ShaderGLRenderer * ref;
    Command_SGR_texture(ShaderGLRenderer * input) {
        ref = input;
    }
    
    typedef std::string (*TextureCommand)(const std::vector<std::string> & args);

    

    std::string TextureCommand_GetInfo(const std::vector<std::string> & args) const {
        TextureManager * texture = ref->GetTextureManager();
        return Chain() 
            << "TextureManager info:\n" 
            << "Dimensions: " << texture->GetTextureW() << "x" << texture->GetTextureH() << " (~" << (texture->GetTextureW()*texture->GetTextureH()*8) / (1024) << "KB of VRAM)\n"
            << "Filter Mode (at the time of command interpretation): " << (int)texture->GetFilter() << "\n"
            << "OpenGL atlas texture handle: " << Chain(texture->GetTexture()) << "\n"
            << "\n"
            << "Commands:\n"
            << " last-id" 
            << " extract [id] [path to image]\n"
            << " delete [id] \n";
    }
    
    std::string TextureCommand_GetLastID(const std::vector<std::string> & args) const {
        return Chain() << "Newest texture id:\n" << ref->GetTextureManager()->GetLastNewID() << "\n";
        
    }
    
    std::string TextureCommand_ExtractTexture(const std::vector<std::string> & args) const {
        if (args.size() != 4) {
            return "Usage: texture extract [id] [path to image]\n";
        }
        int handle = Chain(args[2]).AsUInt32();
        std::string path = args[3];
        
        

        uint32_t width = ref->GetTextureWidth(handle);
        uint32_t height = ref->GetTextureHeight(handle);
        uint32_t numBytes = width*height*4;
        uint8_t * data = new uint8_t[numBytes];
        ref->GetTexture(handle, data);

        // dump it to an image
        AssetID id = Assets::New(Assets::Type::Image, "dump-texture");
        if (id == AssetID()) {
            return "Temporary image creation failed";
        }


        {
            Image & img = Assets::Get<Image>(id);
            img.frames.push_back(Image::Frame(width, height, std::vector<uint8_t>(data, data+numBytes)));
            if (!Assets::Write(id, "png", path)) {
                return "Failed to write texture.";
            }
        }
        Assets::Remove(id);

        delete[] data;
        return "Done.";
        
        
    }


    
    std::string operator()(const std::vector<std::string> & args) {
        std::string cmd = args.size() < 2 ? "" : args[1];
        if (cmd == "extract") {
            return TextureCommand_ExtractTexture(args);
        } else if (cmd == "last-id") {
            return TextureCommand_GetLastID(args);
        
        } else {
            return TextureCommand_GetInfo(args);            
        }
    }
    
    std::string Help() const {return  TextureCommand_GetInfo({});}
};


ShaderGLRenderer::ShaderGLRenderer() {
    GetInterpreter()->AddCommand("info",    new Command_SGR_help(this));
    GetInterpreter()->AddCommand("texture", new Command_SGR_texture(this));
    

    attachedDisplay = nullptr;

    createContext();

    initGL();
    if (!valid) {
        return;
    }

    framebufferHandle = 0;
    framebuffer = nullptr;
    texture =      new TextureManager();
    renderer2D =   new Renderer2D(texture);
    glClearColor(.1, 0, .07, 1.f);
    
    
    diagnostic_dynamic_vtex_per_render_avg = 0;
    diagnostic_dynamic_vtex_per_render_min = 10000000000;
    diagnostic_dynamic_vtex_per_render_max = 0;
    diagnostic_dynamic_vtex_per_render_accumulated_avg = 0;
    diagnostic_dynamic_vtex_per_render_accumulated_avg_ct = 0;
    diagnostic_dynamic_vtex_render_last = 0;
    diagnostic_dynamic_vtex_render_frame_time = time(NULL);
    diagnostic_dynamic_vtex_render_per_frame = 0;
    
    diagnostic_static_object_avg_indices = 0;
    diagnostic_static_object_avg_indices_ct = 0;
    diagnostic_static_object_count_per_second = 0;
    diagnostic_static_object_avg_indices_acc = 0;
    diagnostic_static_object_count_per_second_last = 0;
    diagnostic_static_object_count_time = time(NULL);
    
}





uint32_t ShaderGLRenderer::Add2DObject() {
    return renderer2D->Add2DObject();
}

void ShaderGLRenderer::Remove2DObject(uint32_t a) {
    renderer2D->Remove2DObject(a);
}

void ShaderGLRenderer::Set2DObjectParameters(uint32_t object, Renderer::Render2DObjectParameters p) {
    renderer2D->Set2DObjectParameters(object, p);
}




uint32_t ShaderGLRenderer::Add2DVertex() {
    return renderer2D->Add2DVertex();
}

void ShaderGLRenderer::Remove2DVertex(uint32_t object) {
    renderer2D->Remove2DVertex(object);
}

void ShaderGLRenderer::Set2DVertex(uint32_t vertex, Renderer::Vertex2D src) {
    renderer2D->Set2DVertex(vertex, src);
}

Renderer::Vertex2D ShaderGLRenderer::Get2DVertex(uint32_t vertex) {
    return renderer2D->Get2DVertex(vertex);
}




void ShaderGLRenderer::Queue2DVertices(
    const uint32_t * indices,
    uint32_t count
) {
    renderer2D->Queue2DVertices(indices, count);
}



// Clears all requests queued before the last RenderDynamicQueue
void ShaderGLRenderer::Clear2DQueue() {
    renderer2D->Clear2DQueue();
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




int ShaderGLRenderer::AddTexture(int w, int h, const uint8_t * data) {
    return texture->NewTexture(w, h, (uint8_t*)data);
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


void ShaderGLRenderer::Render2DVertices(const Render2DStaticParameters & params) {
    framebufferCheck();
    uint32_t count = renderer2D->Render2DVertices(drawMode, params);
    diagnostic_dynamic_vtex_per_render_accumulated_avg += count;
    diagnostic_dynamic_vtex_per_render_accumulated_avg_ct++;
    if (diagnostic_dynamic_vtex_per_render_accumulated_avg_ct > 20) {
        diagnostic_dynamic_vtex_per_render_avg = diagnostic_dynamic_vtex_per_render_accumulated_avg / diagnostic_dynamic_vtex_per_render_accumulated_avg_ct;
        diagnostic_dynamic_vtex_per_render_accumulated_avg = diagnostic_dynamic_vtex_per_render_avg;
        diagnostic_dynamic_vtex_per_render_accumulated_avg_ct = 1;
    }
    if (diagnostic_dynamic_vtex_per_render_min > count && count) diagnostic_dynamic_vtex_per_render_min = count;
    if (diagnostic_dynamic_vtex_per_render_max < count) diagnostic_dynamic_vtex_per_render_max = count;
    diagnostic_dynamic_vtex_render_per_frame++;
    if (diagnostic_dynamic_vtex_render_frame_time != time(NULL)) {
        diagnostic_dynamic_vtex_render_last = diagnostic_dynamic_vtex_render_per_frame;
        diagnostic_dynamic_vtex_render_per_frame = 0;
        diagnostic_dynamic_vtex_render_frame_time = time(NULL);
    }
    if (count) {
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

    diagnostic_static_object_avg_indices_acc += obj->indices->size();
    diagnostic_static_object_avg_indices_ct++;
    if (diagnostic_static_object_avg_indices_ct > 100) {
        diagnostic_static_object_avg_indices = diagnostic_static_object_avg_indices_acc / diagnostic_static_object_avg_indices_ct;
        diagnostic_static_object_avg_indices_acc = diagnostic_static_object_avg_indices;
        diagnostic_static_object_avg_indices_ct = 1;
    }


    diagnostic_static_object_count_per_second++;
    if (diagnostic_static_object_count_time != time(NULL)) {
        diagnostic_static_object_count_per_second_last = diagnostic_static_object_count_per_second;
        diagnostic_static_object_count_per_second = 0;
        diagnostic_static_object_count_time = time(NULL);
    }


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
    return "";
}


ProgramID ShaderGLRenderer::ProgramGetBuiltIn(BuiltInShaderMode b) {
    switch(b) {
        case BuiltInShaderMode::BasicShader:   return basicProgramID;
        case BuiltInShaderMode::LightMaterial: return lightProgramID;
    }
    return ProgramID();
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
    bool operator()(LightData * const & a, LightData * const & b) const {
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




bool ShaderGLRenderer::IsSupported(Capability) {
    return true;
}











#endif
