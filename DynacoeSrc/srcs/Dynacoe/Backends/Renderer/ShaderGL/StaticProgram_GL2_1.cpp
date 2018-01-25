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

#include <Dynacoe/Backends/Renderer/ShaderGL/StaticProgram_GL2_1.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer.h>
#include <iostream>
#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Backends/Renderer/StaticState.h>
#include <cfloat>


using namespace Dynacoe;
using std::cout;
using std::endl;
using Dynacoe::Chain;

static const char * vertex_name_c = "_impl_Dynacoe_Input";
static const char * vertex_aux_name_c = "Dynacoe_Input";



enum StaticProgramLocations {
    
    PositionLocation = 0,
    NormalLocation = 1,
    UVLocation = 2
};




// Every incoming shader has this inserted at the beginning of the shader
static const char * DynacoeProgramHeader =

"#define Dynacoe_Position (vec3(_impl_Dynacoe_Input[0].x, _impl_Dynacoe_Input[0].y, _impl_Dynacoe_Input[0].z))\n"
"#define Dynacoe_Normal   (vec3(_impl_Dynacoe_Input[0].w, _impl_Dynacoe_Input[1].x, _impl_Dynacoe_Input[1].y))\n"
"#define Dynacoe_UV       (vec2(_impl_Dynacoe_Input[1].z, _impl_Dynacoe_Input[1].w))\n"

"uniform mat4[2] _b_DV;\n"
"   #define Dynacoe_ViewTransform    (_b_DV[0])\n"
"   #define Dynacoe_NormalTransform  (_b_DV[1]\n"


"uniform mat4 _b_DP;\n"
"   #define Dynacoe_ProjectionTransform (_b_DP)\n"

"uniform vec4[4] _b_DM;\n"
"   #define Dynacoe_MaterialAmbient   (_b_DM[0])\n"
"   #define Dynacoe_MaterialDiffuse   (_b_DM[1].xyz)\n"
"   #define Dynacoe_MaterialSpecular  (_b_DM[2].xyz)\n"
"   #define Dynacoe_MaterialDiffuseAmount (_b_DM[1].w)\n"
"   #define Dynacoe_MaterialSpecularAmount (_b_DM[2].w)\n"
"   #define Dynacoe_MaterialShininess (_b_DM[3].x)\n" // needs to be a vec4 for alignment purposes.  
"   uniform vec4[8] _b_DM2;\n"
"   #define Dynacoe_MaterialData _b_DM2\n"

"uniform mat4[2] _b_DMo;\n"
"   #define Dynacoe_ModelTransform       (_b_DMo[0])\n"
"   #define Dynacoe_ModelNormalTransform (_b_DMo[1])\n"





// up to 64 simultaneous texture bindings
"uniform vec4[65] _b_DT;\n" // DynacoeTexture
"uniform sampler2D _impl_TextureTex;\n"

// up to 128 lights
"uniform vec4[128]  _b_LA;\n" // light active
"uniform sampler2D _impl_LightTex;\n"


// textures
"uniform sampler2D _BSI_Dynacoe_BaseTexture;\n"
"uniform sampler2D _BSI_Dynacoe_FBtexture;\n"
"uniform int       _BSI_Dynacoe_hasFBtexture;\n"
"uniform float      _impl_lightTextureH;\n"
"uniform float      _impl_texTextureH;\n"

"vec4 _impl_sampleDataT(in int i) {\n"
"   return texture2D(_impl_TextureTex, vec2(mod(i, 64)/64.f, (i/64)/_impl_texTextureH));\n"
"}\n"

"vec4 _impl_sampleDataL(in int i) {\n"  
"   return texture2D(_impl_LightTex, vec2(mod(i, 64)/64.f, (i/64)/_impl_lightTextureH));\n"
"}\n"

// Internal interface for standard functions.
"#define _BSI_Dynacoe_Texture_GUT_x         (_b_DT[0].x)\n"
"#define _BSI_Dynacoe_Texture_GUT_y         (_b_DT[0].y)\n"
"vec2  _BSI_Dynacoe_Texture_pos (in int i) { return _b_DT[i+1].xy;}\n"
"vec2  _BSI_Dynacoe_Texture_dims(in int i) { return _b_DT[i+1].zw;}\n"
"float _BSI_Dynacoe_Texture_exists(in int i) { return _impl_sampleDataT(i).r;}\n"



"vec3  _BSI_Dynacoe_Light_pos(in int i)       {return _b_LA[i].xyz;}\n"
"float _BSI_Dynacoe_Light_intensity(in int i) {return _b_LA[i].w;}\n"
"float _BSI_Dynacoe_Light_type(in int i)      {return _impl_sampleDataL(i).w;}\n"
"vec3  _BSI_Dynacoe_Light_color(in int i)     {return _impl_sampleDataL(i).xyz;}\n"



#include "base.glsl.h"







StaticProgram_GL2_1::StaticProgram_GL2_1(){
    incomplete = true;
}


bool StaticProgram_GL2_1::Set(const char * vertSrc_raw,
                     const char * fragSrc_raw,
                     const std::string & name,
                     RenderBuffer * viewUniform_in,
                     RenderBuffer * projectionUniform_in,
                     RenderBuffer * DynacoeTextureData_uniformID, // 4 components: xy pos, zw dims 0 -> GUT, 
                     RenderBuffer * DynacoeTextureInfo_uniformID, // 4 component: x- enabled
                     RenderBuffer * DynacoeLightingData_uniformID, // 4 components: xyz position, w intensity
                     RenderBuffer * DynacoeLightingInfo_uniformID // 4 components: xyz color, w type


                     ) 
                     
                     {
    
    log = "";
    passedTexture = false;
    progName = name;
    
    /* // dont think its worth linking it
    if (!dynamic_cast<RenderBuffer_Tex*>(lightUniform_in)) {
        std::cout << "Internal Error! (GL2.1 needs a texture-variant renderbuffer for its lighting storage, dummy!);" << std::endl;
        return false;
    }*/
    
    Chain fragSrc;
    Chain vertSrc;
    
    viewUniform       = viewUniform_in;
    projectionUniform = projectionUniform_in;
    textureUniform    = DynacoeTextureData_uniformID;
    lightingUniform   = DynacoeLightingData_uniformID;
    textureUniformInfo = DynacoeTextureInfo_uniformID;
    lightingUniformInfo = DynacoeLightingInfo_uniformID;
    


    std::string header = "#version 120\n";


    fragSrc << header.c_str() << DynacoeProgramHeader << fragSrc_raw;
    vertSrc << header.c_str() 
            /*<< "in vec3 Dynacoe_Position;\n"
               "in vec3 Dynacoe_Normal;\n"
               "in vec2 Dynacoe_UV;\n"
            */
            << "attribute mat2x4 _impl_Dynacoe_Input;\n"
            << "attribute vec4 Dynacoe_Input;\n"
            << DynacoeProgramHeader << vertSrc_raw;


    log += "[Dynacoe::OpenGL]: Building program " + name + "\n";


    progID = glCreateProgram();



    fragID = glCreateShader(GL_FRAGMENT_SHADER);
    vertID = glCreateShader(GL_VERTEX_SHADER);

    const char * fragSrcPtr = fragSrc.ToString().c_str();
    const char * vertSrcPtr = vertSrc.ToString().c_str();

    glShaderSource(fragID, 1, &fragSrcPtr, NULL);
    glShaderSource(vertID, 1, &vertSrcPtr, NULL);

    glCompileShader(fragID);
    glCompileShader(vertID);

    int success;
    glGetShaderiv(fragID, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        log += "[Dynacoe::OpenGL]: Fragment shader failed to compile:\n";
        printShaderLog(fragID);
        return FailSet();
    }


    glGetShaderiv(vertID, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        log += "[Dynacoe::OpenGL]: Vertex shader failed to compile:\n";
        printShaderLog(vertID);
        return FailSet();
    }


    glAttachShader(progID, fragID);
    glAttachShader(progID, vertID);

    

    // bind locations
    glBindAttribLocation(progID, 0,  vertex_name_c);
    glBindAttribLocation(progID, 13, vertex_aux_name_c);






    glLinkProgram(progID);

    glGetProgramiv(progID, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        log += "[Dynaoce::OpenGL]: Linking for program failed (But compilation of both stages was successful!)\n";
        printProgramLog(progID);
        return false;
    }    

     
    
    
    // get locations for all used uniforms upfront
    if ((viewUniform_location             = glGetUniformLocation(progID, "_b_DV"))  == -1) log += "Warning: view matrices appear to be unused.\n";
    if ((projectionUniform_location       = glGetUniformLocation(progID, "_b_DP"))  == -1) log += "Warning: projection matrix appears to be unused.\n";
    if ((textureDataUniform_location      = glGetUniformLocation(progID, "_b_DT"))  == -1) log += "Warning: texturing data appears to be unused.\n";
    if ((lightingDataUniform_location     = glGetUniformLocation(progID, "_b_LA"))  == -1) log += "Warning: light data appears to be unused.\n";
    if ((textureInfoUniform_location      = glGetUniformLocation(progID, "_impl_TextureTex"))  == -1) log += "Warning: texturing data appears to be unused.\n";
    if ((lightingInfoUniform_location     = glGetUniformLocation(progID, "_impl_LightTex"))    == -1) log += "Warning: light data appears to be unused.\n";    
    if ((materialUniform_location         = glGetUniformLocation(progID, "_b_DM"))  == -1) log += "Warning: material data appears to be unused.\n";
    if ((materialDataUniform_location     = glGetUniformLocation(progID, "_b_DM2")) == -1) log += "Warning: material aux data appears to be unused.\n";
    if ((modelUniform_location            = glGetUniformLocation(progID, "_b_DMo")) == -1) log += "Warning: model transform data pears to be unused.\n";
    if ((lightingHeightUniform_location   = glGetUniformLocation(progID, "_impl_lightTextureH")) == -1);
    if ((textureHeightUniform_location   = glGetUniformLocation(progID, "_impl_texTextureH")) == -1);

    ((fbTexture_location               = glGetUniformLocation(progID, "_BSI_Dynacoe_FBtexture"))    == -1);
    ((baseTexture_location             = glGetUniformLocation(progID, "_BSI_Dynacoe_BaseTexture"))  == -1);
    ((hasFbTexture_location            = glGetUniformLocation(progID, "_BSI_Dynacoe_hasFBtexture")) == -1);

    int status = glGetError();
    if (status != GL_NO_ERROR) {
        return FailSet();
    } else {
        log+= "[Dynacoe::OpenGL]: " + name + " successfully built";
    }
    incomplete = false;
    return !incomplete;
}

std::string StaticProgram_GL2_1::GetLog() {
    return log;
}





void StaticProgram_GL2_1::UpdateUniforms(RenderBuffer * model, RenderBuffer *material) {
    float texData[2];
    int h, oldTex;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);
    glBindTexture(GL_TEXTURE_2D, lightingUniformInfo->GenerateBufferID());
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    texData[0] = h;

    glBindTexture(GL_TEXTURE_2D, textureUniformInfo->GenerateBufferID());
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    texData[1] = h;
    glBindTexture(GL_TEXTURE_2D, oldTex);

    if (viewUniform_location           != -1) glUniformMatrix4fv(viewUniform_location,           2,   GL_FALSE, viewUniform->GetData());
    if (projectionUniform_location     != -1) glUniformMatrix4fv(projectionUniform_location,     1,   GL_FALSE, projectionUniform->GetData());
    if (textureDataUniform_location    != -1) glUniform4fv      (textureDataUniform_location,    64,            textureUniform->GetData());
    if (lightingDataUniform_location   != -1) glUniform4fv      (lightingDataUniform_location,   128,           lightingUniform->GetData());    
    if (lightingHeightUniform_location != -1) glUniform1f       (lightingHeightUniform_location,                texData[0]); // 
    if (textureHeightUniform_location  != -1) glUniform1f       (textureHeightUniform_location,                 texData[1]); // 
    if (materialUniform_location       != -1) glUniform4fv      (materialUniform_location,       4,             material->GetData());
    if (materialDataUniform_location   != -1) glUniform4fv      (materialDataUniform_location,   8,             material->GetData()+16); //check
    if (modelUniform_location          != -1) glUniformMatrix4fv(modelUniform_location,          2,   GL_FALSE, model->GetData());
}


void StaticProgram_GL2_1::printShaderLog(int shaderID) {
    int logLength;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
    char l[logLength];

    glGetShaderInfoLog(shaderID, logLength, &logLength, l);
    
    log += std::string(l);
}

void StaticProgram_GL2_1::printProgramLog(int shaderID) {
    int logLength;
    glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
    char l[logLength];

    glGetProgramInfoLog(shaderID, logLength, &logLength, l);
    
    log += std::string(l);
}





void StaticProgram_GL2_1::Run(uint32_t * indexList, uint32_t numIndices, RenderBuffer * vertexBufferID, RenderBuffer * matBufferID, RenderBuffer * modelBufferID, GLuint baseTex, GLuint fbTex) {
    if (incomplete) return;

    glActiveTexture(GetBaseTextureActiveIndex());
    glBindTexture(GL_TEXTURE_2D,  baseTex);
    
    glActiveTexture(GetSourceFBTextureActiveIndex());
    glBindTexture(GL_TEXTURE_2D, fbTex);
    
    glActiveTexture(GL_TEXTURE0 + 12);
    glBindTexture(GL_TEXTURE_2D, lightingUniformInfo->GenerateBufferID());

    glActiveTexture(GL_TEXTURE0 + 13);
    glBindTexture(GL_TEXTURE_2D, textureUniformInfo->GenerateBufferID());
    
    
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID->GenerateBufferID());
    glVertexAttribPointer(0,  4, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void *) 0);
    glVertexAttribPointer(1,  4, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void *) (sizeof(GLfloat)*4));
    glVertexAttribPointer(13, 4, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void *) (sizeof(GLfloat)*8));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glUseProgram(progID);
    UpdateUniforms(modelBufferID, matBufferID);    
    
    
    

    
    glUniform1i(hasFbTexture_location, fbTex);
    if (!passedTexture) {
        glUniform1i(baseTexture_location, GetBaseTextureActiveIndex() - GL_TEXTURE0);
        glUniform1i(fbTexture_location,   GetSourceFBTextureActiveIndex() - GL_TEXTURE0);
        glUniform1i(lightingInfoUniform_location, 12);
        glUniform1i(textureInfoUniform_location, 13);

        
        passedTexture = true;
    } 


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(13);
    
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexList);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(13);
}

bool StaticProgram_GL2_1::FailSet() {
    glDeleteShader(vertID);
    glDeleteShader(fragID);
    glDeleteProgram(progID);
    return false;
}





#endif
