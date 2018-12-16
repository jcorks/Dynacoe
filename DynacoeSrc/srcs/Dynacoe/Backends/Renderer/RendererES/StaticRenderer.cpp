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


#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Backends/Renderer/RendererES/StaticRenderer.h>
#include <Dynacoe/Backends/Renderer/StaticState.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget.h>
#include <GLES2/gl2.h>
#include <cassert>
#include "static_es.glsl"

using namespace Dynacoe;

struct MAT4_DEBUG {
    float f[16];
};

struct VEC4_DEBUG {
    float v[4];
};

struct VTEX_DEBUG {
    float pos[3];
    float normal[3];
    float vecs[2];
    float userData[4];
};

static MAT4_DEBUG mat4_reg;
static VEC4_DEBUG vec4_reg;
static VTEX_DEBUG vtex_reg;


// A discrete program object to interface directly with OpenGLES
class ProgramES {
  public:

    // Initializes the program. If invalid, IsValid() returns false.
    //
    ProgramES(
        const std::string & vert,  
        const std::string & frag
    ) {
        valid = false;
        handle = glCreateProgram();

        GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);
        GLuint vertID = glCreateShader(GL_VERTEX_SHADER);

        std::string vertPreamble =             
        "attribute highp   vec3 Dynacoe_Position;\n"
        "attribute mediump vec3 Dynacoe_Normal;\n"
        "attribute highp   vec2 Dynacoe_UV;\n"
        "attribute highp   vec4 Dynacoe_UserData;\n#line 1 1\n";

        std::string vSrc;
        std::string fSrc;
        std::string header;

        // TODO: fill this out, dork
        header = static_es_glsl;
        vSrc = header + vertPreamble + vert;
        fSrc = header + frag;


        const char * vPtr = vSrc.c_str();
        const char * fPtr = fSrc.c_str();

        glShaderSource(fragID, 1, &fPtr, NULL);
        glShaderSource(vertID, 1, &vPtr, NULL);



        glAttachShader(handle, fragID);
        glAttachShader(handle, vertID);

        glCompileShader(fragID);
        glCompileShader(vertID);

        assert(glGetError() == GL_NO_ERROR);

        int success =0;
        glGetShaderiv(fragID, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            log << "[Dynacoe::Program]: Fragment shader failed to compile:" << "\n";
            int logLength;
            glGetShaderiv(fragID, GL_INFO_LOG_LENGTH, &logLength);
            char * logData = (char*)malloc(logLength+1);
            glGetShaderInfoLog(fragID, logLength, &logLength, logData);
            log << logData << "\n";
            free(logData);
            return;
        }


        glGetShaderiv(vertID, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            log << "[Dynacoe::Program]: Vertex shader failed to compile:" << "\n";
            int logLength;
            glGetShaderiv(vertID, GL_INFO_LOG_LENGTH, &logLength);
            char * logData = (char*)malloc(logLength+1);
            glGetShaderInfoLog(vertID, logLength, &logLength, logData);
            log << logData << "\n";
            free(logData);
            return;
        }

        // Binding is always applied fo rth next link
        glLinkProgram(handle);


        glGetProgramiv(handle, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            log << "[Dynaoce::Program]: Linking for program failed" << "\n";

            int logLength;
            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
            char * logData = (char*)malloc(logLength+1);
            glGetProgramInfoLog(handle, logLength, &logLength, logData);
            log << logData << "\n";
            free(logData);
            return;
        }

        glUseProgram(handle);

        attribLocation_pos      = glGetAttribLocation(handle, "Dynacoe_Position");
        attribLocation_normal   = glGetAttribLocation(handle, "Dynacoe_Normal");
        attribLocation_uvs      = glGetAttribLocation(handle, "Dynacoe_UV");
        attribLocation_userData = glGetAttribLocation(handle, "Dynacoe_UserData");

        uniformLocation_ViewTransform = glGetUniformLocation(handle, "Dynacoe_ViewTransform");
        uniformLocation_ViewNormalTransform = glGetUniformLocation(handle, "Dynacoe_ViewNormalTransform");
        uniformLocation_ProjectionTransform = glGetUniformLocation(handle, "Dynacoe_ProjectionTransform"); 
        
        uniformLocation_MaterialAmbient = glGetUniformLocation(handle, "_impl_Dynacoe_MaterialAmbient");
        uniformLocation_MaterialDiffuse = glGetUniformLocation(handle, "_impl_Dynacoe_MaterialDiffuse");
        uniformLocation_MaterialSpecular = glGetUniformLocation(handle, "_impl_Dynacoe_MaterialSpecular");
        
        uniformLocation_MaterialData = glGetUniformLocation(handle, "Dynacoe_MaterialData");
        uniformLocation_ModelTransform = glGetUniformLocation(handle, "Dynacoe_ModelTransform");
        uniformLocation_ModelNormalTransform = glGetUniformLocation(handle, "Dynacoe_ModelNormalTransform");

        uniformLocation_fragTex_slots = glGetUniformLocation(handle, "fragTex_slots");
        uniformLocation_TexInfo_coords = glGetUniformLocation(handle, "_impl_Dynacoe_TexInfo_coords");
        uniformLocation_TexInfo_handle = glGetUniformLocation(handle, "_impl_Dynacoe_TexInfo_handle");

        uniformLocation_LightData1 = glGetUniformLocation(handle, "_impl_Dynacoe_LightData");
        uniformLocation_LightData2 = glGetUniformLocation(handle, "_impl_Dynacoe_LightData2");

        uniformLocation_hasFBtexture = glGetUniformLocation(handle, "_BSI_Dynacoe_hasFBtexture");
        uniformLocation_FBtexture = glGetUniformLocation(handle, "_BSI_Dynacoe_FBtexture");



        if (attribLocation_pos < 0)      log << "Position attribute was opmtimized out.\n";
        if (attribLocation_normal < 0)   log << "Normal attribute was opmtimized out.\n";
        if (attribLocation_uvs < 0)      log << "UVs attribute was opmtimized out.\n";
        if (attribLocation_userData < 0) log << "User data attribute was opmtimized out.\n";


        if (uniformLocation_ViewTransform < 0) log << "ViewTransform optimized out.\n";
        if (uniformLocation_ViewNormalTransform < 0) log << "ViewNormalTransform optimized out.\n";
        if (uniformLocation_ProjectionTransform < 0) log << "ProjectionTransform optimized out.\n";
        if (uniformLocation_MaterialAmbient < 0) log << "MaterialAmbient optimized out.\n";
        if (uniformLocation_MaterialDiffuse < 0) log << "MaterialDiffuse optimized out.\n";
        if (uniformLocation_MaterialSpecular < 0) log << "MaterialSpecular optimized out.\n";

        if (uniformLocation_MaterialData < 0) log << "MaterialData optimized out.\n";
        if (uniformLocation_ModelTransform < 0) log << "ModelTransform optimized out.\n";
        if (uniformLocation_ModelNormalTransform < 0) log << "ModelNormalTransform optimized out.\n";

        if (uniformLocation_fragTex_slots < 0) log << "Texturing info (FTS) optimized out.\n";
        if (uniformLocation_TexInfo_coords < 0) log << "Texturing info (TIC) optimized out.\n";
        if (uniformLocation_TexInfo_handle < 0) log << "Texturing info (TIH) optimized out.\n";

        if (uniformLocation_LightData1 < 0) log << "Lighting info (LD1) optimized out.\n";
        if (uniformLocation_LightData2 < 0) log << "Lighting info (LD2) optimized out.\n";

        if (uniformLocation_FBtexture < 0) log << "Framebuffer texture slot optimized out.\n";
        if (uniformLocation_hasFBtexture < 0) log << "Framebuffer texture condition optimized out.\n";



        assert(glGetError() == GL_NO_ERROR);
        valid = true;
        glUseProgram(0);
    }




    // returns the last recorded log for the program. Usually means bad things
    std::string GetLog() const {
        return log;
    }

    // returns whether the program is valid
    bool IsValid() const {
        return valid;
    }

    
    // prepares the program for use.
    void Run(const float * viewingMatrix, // 2 matrices: view and viewNormal
             const float * projectionMatrix,
             const float * formattedLightData,
             StaticState * state,
             Dynacoe::Table<RenderBuffer*> * buffers,
             Texture_ES * textureState) {

        assert(glGetError() == 0);

        RenderBuffer * vertices     = buffers->Find(state->vertices);
        RenderBuffer * materialData = buffers->Find(state->materialData);
        RenderBuffer * modelData    = buffers->Find(state->modelData);
        float * textures = GenerateTextureData(state, textureState);


        if (!vertices || !modelData || !materialData) {
            return;
        }

        glUseProgram(handle);

        // force these buffers to only have conventional memory references
        materialData->SetOffline();
        modelData   ->SetOffline();
        

        // vertex state
        GLuint vbo = vertices->GenerateBufferID();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        if (attribLocation_pos      >= 0) glVertexAttribPointer(attribLocation_pos,      3, GL_FLOAT, GL_FALSE, sizeof(float)*12, (void*)(0));
        if (attribLocation_normal   >= 0) glVertexAttribPointer(attribLocation_normal,   3, GL_FLOAT, GL_FALSE, sizeof(float)*12, (void*)(sizeof(float)*3));
        if (attribLocation_uvs      >= 0) glVertexAttribPointer(attribLocation_uvs,      2, GL_FLOAT, GL_FALSE, sizeof(float)*12, (void*)(sizeof(float)*6));
        if (attribLocation_userData >= 0) glVertexAttribPointer(attribLocation_userData, 4, GL_FLOAT, GL_FALSE, sizeof(float)*12, (void*)(sizeof(float)*8));

        
        // bind uniforms

        // global view matrices
        if (uniformLocation_ViewTransform >= 0)       glUniformMatrix4fv(uniformLocation_ViewTransform,       1, false, viewingMatrix);
        if (uniformLocation_ViewNormalTransform >= 0) glUniformMatrix4fv(uniformLocation_ViewNormalTransform, 1, false, viewingMatrix+16);
        if (uniformLocation_ProjectionTransform >= 0) glUniformMatrix4fv(uniformLocation_ProjectionTransform, 1, false, projectionMatrix);



        // material       
        if (uniformLocation_MaterialAmbient >= 0)  glUniform4fv(uniformLocation_MaterialAmbient,  1, materialData->GetData());
        if (uniformLocation_MaterialDiffuse >= 0)  glUniform4fv(uniformLocation_MaterialDiffuse,  1, materialData->GetData()+4);
        if (uniformLocation_MaterialSpecular >= 0) glUniform4fv(uniformLocation_MaterialSpecular, 1, materialData->GetData()+8);
        if (uniformLocation_MaterialData >= 0)     glUniform4fv(uniformLocation_MaterialData,     8, materialData->GetData()+12);

        
        // model transform
        if (uniformLocation_ModelTransform >= 0)       glUniformMatrix4fv(uniformLocation_ModelTransform,       1, false, modelData->GetData()+0);
        if (uniformLocation_ModelNormalTransform >= 0) glUniformMatrix4fv(uniformLocation_ModelNormalTransform, 1, false, modelData->GetData()+16);


        // texture 
        static int ACTIVE_SLOTS[128];
        static int ACTIVE_IDS  [128];
        int slots = textureState->GetActiveTextureSlots(ACTIVE_SLOTS, ACTIVE_IDS);
        for(uint32_t i = 0; i < slots; ++i) {
            glActiveTexture(ACTIVE_SLOTS[i]);
            glBindTexture(GL_TEXTURE_2D, ACTIVE_IDS[i]);
            ACTIVE_SLOTS[i] -= GL_TEXTURE0;
        }
        glUniform1iv(uniformLocation_fragTex_slots, slots, ACTIVE_SLOTS);
        

        // active texture slot lookup data
        if (uniformLocation_TexInfo_coords >= 0) glUniform4fv(uniformLocation_TexInfo_coords, 32, textures);
        if (uniformLocation_TexInfo_handle >= 0) glUniform1fv(uniformLocation_TexInfo_handle, 32, textures+32*4);
                        

        // active light data
        if (uniformLocation_LightData1 >= 0) glUniform4fv(uniformLocation_LightData1, 32, formattedLightData);
        if (uniformLocation_LightData2 >= 0) glUniform4fv(uniformLocation_LightData2, 32, formattedLightData+32*4);
    

        int hasFB = state->samplebuffer!= nullptr;
        float hasFBflt = hasFB;
        if (uniformLocation_hasFBtexture >= 0) glUniform1f(uniformLocation_hasFBtexture, hasFBflt);

        if (hasFB) {
            int slot = ACTIVE_SLOTS[slots-1]+1+GL_TEXTURE0;
            glActiveTexture(slot);

            GLRenderTarget * t = *(GLRenderTarget **)state->samplebuffer->GetHandle();
            t->Sync();
            GLint texture = t->GetTexture();


            glBindTexture(GL_TEXTURE_2D, texture);
            slot -= GL_TEXTURE0;
            if (uniformLocation_FBtexture >= 0) glUniform1i(uniformLocation_FBtexture, slot);   
        }



        if (attribLocation_pos      >= 0) glEnableVertexAttribArray(attribLocation_pos);
        if (attribLocation_uvs      >= 0) glEnableVertexAttribArray(attribLocation_uvs);      
        if (attribLocation_normal   >= 0) glEnableVertexAttribArray(attribLocation_normal);
        if (attribLocation_userData >= 0) glEnableVertexAttribArray(attribLocation_userData);


        glDrawElements(
            GL_TRIANGLES, 
            state->indices->size(), 
            GL_UNSIGNED_INT, 
            &((*state->indices)[0])
        );

        if (attribLocation_pos      >= 0) glDisableVertexAttribArray(attribLocation_pos);
        if (attribLocation_uvs      >= 0) glDisableVertexAttribArray(attribLocation_uvs);      
        if (attribLocation_normal   >= 0) glDisableVertexAttribArray(attribLocation_normal);
        if (attribLocation_userData >= 0) glDisableVertexAttribArray(attribLocation_userData);
        assert(glGetError() == 0);

            
    }
  private:
    GLint handle;
    bool valid;
    Dynacoe::Chain log;

    // standard entry points
    GLint attribLocation_pos; //3
    GLint attribLocation_uvs; //2
    GLint attribLocation_normal; //3
    GLint attribLocation_userData; //4


    //uniforms!
    GLint uniformLocation_ViewTransform; // mat4
    GLint uniformLocation_ViewNormalTransform; // mat4
    GLint uniformLocation_ProjectionTransform; //mat4;


    GLint uniformLocation_MaterialAmbient; // vec4
    GLint uniformLocation_MaterialDiffuse; // vec4
    GLint uniformLocation_MaterialSpecular; // vec4
    

    GLint uniformLocation_MaterialData;// vec4[8];
    GLint uniformLocation_ModelTransform;// mat4
    GLint uniformLocation_ModelNormalTransform;// mat4

    GLint uniformLocation_fragTex_slots; // sampler2D[32]


    GLint uniformLocation_TexInfo_coords; //vec4[32];
    GLint uniformLocation_TexInfo_handle; //vec4[32];

    GLint uniformLocation_LightData1; //vec4[32];
    GLint uniformLocation_LightData2; //vec4[32];

    GLint uniformLocation_FBtexture; // sampler2D
    GLint uniformLocation_hasFBtexture; // float


    float * GenerateTextureData(StaticState * state, Texture_ES * textureState) {
        static float rawData[32*4 + 32];
        
        // clear out old data.
        float * iterClear = &rawData[32*4];
        for(int i = 0; i < 32; ++i) {
            iterClear[i] = -1.f;
        }


        // for each texture, replace the data with that slot. 
        // the texture coordinates follow the layout specified in the static_es.glsl shader base.
        uint32_t num = state->textures->size();
        float * iterCoords;
        float * iterHandles;        
        for(uint32_t i = 0; i < num; ++i) {
            auto slot = (*state->textures)[0];
            iterCoords = &rawData[slot.first*4];
            iterHandles = &rawData[32*4 + slot.first];

            iterCoords[0] = 0.f; 
            iterCoords[1] = 0.f;
            iterCoords[2] = 1.f;
            iterCoords[3] = 1.f;

            textureState->TranslateCoords(iterCoords+0, iterCoords+1, slot.second);
            textureState->TranslateCoords(iterCoords+2, iterCoords+3, slot.second);
           
            // itercoords 2 and 3 need to be the "width" of the texture in atlas UVs
            iterCoords[2] -= iterCoords[0];
            iterCoords[3] -= iterCoords[1];


            *iterHandles = (float) textureState->GetSlotForTexture(slot.second);
    

        }

        return &rawData[0];
    }

    
};















class Dynacoe::StaticRenderer_Data {
  public:
    StaticRenderer_Data(Texture_ES * es, Dynacoe::Table<RenderBuffer*> * buffers_, Light_ES * lighting_) {
        texture = es;
        buffers = buffers_;
        lighting = lighting_;
        lightingBuffer = new float[32*4*2];


        // flat shading!
        ProgramES * builtIn_flat_ref = new ProgramES(

            ////// VERTEX SHADER //////
            "varying highp   vec2 UV;\n"



            "void main(void) {\n"
            "   gl_Position = Dynacoe_ProjectionTransform * (Dynacoe_ViewTransform  * (Dynacoe_ModelTransform * vec4(Dynacoe_Position, 1.0)));\n"
            "   UV = Dynacoe_UV;\n"
            "}\n",


    
            ////// FRAGMENT SHADER //////

            "varying highp   vec2 UV;\n"


            "void main(void) {\n"
            "   lowp vec4 color= vec4(1, 1, 1, 1);\n"
            "   if (Dynacoe_SlotHasTexture(0))\n"
            "       color = Dynacoe_SampleColor(0, UV);\n"
            "   else\n"
            "       color = vec4(Dynacoe_MaterialDiffuse, 1.0);\n"
            "   if (Dynacoe_CanSampleFramebuffer())\n"
            "       color = color*Dynacoe_SampleFramebuffer(UV);\n"
            "   gl_FragColor = color; \n"
            "}\n"

        ); 

        if (!builtIn_flat_ref->IsValid()) {
            printf("CRITICAL ERROR:\n\n%s\n", builtIn_flat_ref->GetLog().c_str());
            exit(1);
        }

        builtIn_flat = programs.Insert(builtIn_flat_ref);
        printf("%s\n", builtIn_flat_ref->GetLog().c_str());






        // light shading!
        ProgramES * builtIn_light_ref = new ProgramES(

            ////// VERTEX SHADER //////
            "varying highp vec2 UV;\n"
            "varying highp vec3 normalInterp;\n"
            "varying highp vec3 pos;\n"


            "void main(void) {\n"


            "   highp mat4 mv = (Dynacoe_ViewTransform  * Dynacoe_ModelTransform);"
            "   highp vec4 unprojPos = (mv * (vec4(Dynacoe_Position, 1.0)));\n"
            "   gl_Position = Dynacoe_ProjectionTransform * unprojPos;\n"
            "   UV = Dynacoe_UV;\n"
            "   normalInterp = (Dynacoe_ModelNormalTransform * vec4(Dynacoe_Normal, 0.0)).xyz;\n"
            "   pos = unprojPos.xyz / unprojPos.w;\n"

            "}\n",


    
            ////// FRAGMENT SHADER //////
            "varying highp vec2 UV;\n"
            "varying highp vec3 normalInterp;\n"
            "varying highp vec3 pos;\n"


            "void main(void) {\n"
            "   highp float reflectivity;\n"
            "   if (Dynacoe_SlotHasTexture(1)) {\n"
            "       reflectivity = Dynacoe_SampleShininess(1, UV);\n"
            "   } else { \n"
            "       reflectivity = 1.0;\n"
            "   }\n"
            "   gl_FragColor.xyz = Dynacoe_MaterialAmbient.xyz + reflectivity *  Dynacoe_CalculateLightFragment(\n"
            "         pos, \n"
            "         normalInterp,\n"
            "         Dynacoe_MaterialDiffuseAmount, Dynacoe_MaterialDiffuse, Dynacoe_MaterialSpecularAmount, Dynacoe_MaterialSpecular, Dynacoe_MaterialShininess);\n"
            "   if (Dynacoe_SlotHasTexture(0)) {\n"
            "       gl_FragColor = mix(gl_FragColor, Dynacoe_SampleColor(0, UV), .5);\n"
            "   }\n"
            "   if (Dynacoe_CanSampleFramebuffer()) {\n"
            "       gl_FragColor = gl_FragColor * Dynacoe_SampleFramebuffer(UV);\n"
            "   }\n"
            "   gl_FragColor.a = 1.0;\n"
            "}\n"

        ); 

        if (!builtIn_light_ref->IsValid()) {
            printf("CRITICAL ERROR:\n\n%s\n", builtIn_light_ref->GetLog().c_str());
            exit(1);
        }

        builtIn_lighting = programs.Insert(builtIn_light_ref);
        printf("%s\n", builtIn_light_ref->GetLog().c_str());

        viewingMatrix = new RenderBuffer();
        viewingMatrix->SetOffline();
        viewingMatrix->Define(nullptr, 32);
        viewingMatrixID = buffers->Insert(viewingMatrix);


        projectionMatrix = new RenderBuffer();
        projectionMatrix->SetOffline();
        projectionMatrix->Define(nullptr, 16);
        projectionMatrixID = buffers->Insert(projectionMatrix);
        
    }

    Dynacoe::LookupID builtIn_flat;
    Dynacoe::LookupID builtIn_lighting;
    Dynacoe::Table<ProgramES*> programs;
    Dynacoe::Table<RenderBuffer*> * buffers;
    Texture_ES * texture;   
    Light_ES * lighting;


    RenderBuffer * viewingMatrix;
    RenderBuffer * projectionMatrix;
    RenderBufferID viewingMatrixID;
    RenderBufferID projectionMatrixID;
    float * lightingBuffer;



};








StaticRenderer::StaticRenderer(Texture_ES * t, Dynacoe::Table<RenderBuffer*> * buffers, Light_ES * light) {
    ES = new StaticRenderer_Data(t, buffers, light);
}



ProgramID StaticRenderer::ProgramAdd(
        const std::string & vert,
        const std::string & frag,
        std::string & log ) {
    ProgramES * newProgram = new ProgramES(vert, frag);
    log = newProgram->GetLog();

    if (!newProgram->IsValid()) {
        delete newProgram;
        return ProgramID();
    }    
    
    return ES->programs.Insert(newProgram);
}


void StaticRenderer::Render(StaticState * st) {
    if (!ES->programs.Query(st->program)) return;
    ProgramES * program = ES->programs.Find(st->program);
    ES->lighting->SyncLightBuffer(ES->lightingBuffer);
    program->Run(
        ES->viewingMatrix->GetData(),
        ES->projectionMatrix->GetData(),
        ES->lightingBuffer, // TODO: lighting,
        st,
        ES->buffers,
        ES->texture
    );
}


RenderBufferID StaticRenderer::GetViewingMatrixID() {
    return ES->viewingMatrixID;
}


RenderBufferID StaticRenderer::GetProjectionMatrixID() {
    return ES->projectionMatrixID;
}

ProgramID StaticRenderer::ProgramGetBuiltIn(Renderer::BuiltInShaderMode mode) {
    switch(mode) {
      case Renderer::BuiltInShaderMode::BasicShader:   return ES->builtIn_flat;
      case Renderer::BuiltInShaderMode::LightMaterial: return ES->builtIn_lighting;

    }
    return ProgramID();
}












#endif
