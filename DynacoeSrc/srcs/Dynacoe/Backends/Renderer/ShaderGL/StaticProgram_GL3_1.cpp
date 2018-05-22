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

#include <Dynacoe/Backends/Renderer/ShaderGL/StaticProgram_GL3_1.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer.h>
#include <iostream>
#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Backends/Renderer/StaticState.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/GLVersionQuery.h>
#include <cassert>


using namespace Dynacoe;
using std::cout;
using std::endl;
using Dynacoe::Chain;

static const char * vertex_name_c = "_impl_Dynacoe_Input";
static const char * vertex_aux_name_c = "Dynacoe_Input";
//const char * position_name_c = "Dynacoe_Position";
//const char * normal_name_c   = "Dynacoe_Normal";
//const char * uv_name_c       = "Dynacoe_UVs;";



enum StaticProgramLocations {
    
    PositionLocation = 0,
    NormalLocation = 1,
    UVLocation = 2
};




// Every incoming shader has this inserted at the beginning of the shader
static const char * DynacoeProgramHeader =
"#define Dynacoe_MaterialShininess Dynacoe_MaterialShininess_SRC.x\n"
"#define Dynacoe_Position (vec3(_impl_Dynacoe_Input[0].x, _impl_Dynacoe_Input[0].y, _impl_Dynacoe_Input[0].z))\n"
"#define Dynacoe_Normal   (vec3(_impl_Dynacoe_Input[0].w, _impl_Dynacoe_Input[1].x, _impl_Dynacoe_Input[1].y))\n"
"#define Dynacoe_UV       (vec2(_impl_Dynacoe_Input[1].z, _impl_Dynacoe_Input[1].w))\n"


"uniform _binding_DynacoeView {\n"
"   mat4 Dynacoe_ViewTransform;\n"
"   mat4 Dynacoe_ViewNormalTransform;\n"
"};\n"

"uniform _binding_DynacoeProjection{\n"
"   mat4 Dynacoe_ProjectionTransform ;\n"
"};\n"

"uniform _binding_DynacoeMaterial {\n"
"   vec4    Dynacoe_MaterialAmbient;\n"
"   vec4    _impl_Dynacoe_MaterialDiffuse;\n" // xyz - pos, w - amount
"   vec4    _impl_Dynacoe_MaterialSpecular;\n"
"   vec4    Dynacoe_MaterialShininess_SRC;\n" // needs to be a vec4 for alignment purposes.  
"   vec4[8] Dynacoe_MaterialData;\n"
"};\n"

"#define Dynacoe_MaterialDiffuse        (_impl_Dynacoe_MaterialDiffuse.xyz)\n"
"#define Dynacoe_MaterialDiffuseAmount  (_impl_Dynacoe_MaterialDiffuse.w)\n"
"#define Dynacoe_MaterialSpecular       (_impl_Dynacoe_MaterialSpecular.xyz)\n"
"#define Dynacoe_MaterialSpecularAmount (_impl_Dynacoe_MaterialSpecular.w)\n"


"uniform _binding_DynacoeModel {\n"
"   mat4 Dynacoe_ModelTransform;\n"
"   mat4 Dynacoe_ModelNormalTransform;\n"
"};\n"



/* TO avoid needing to query offsets, the data will be implicitly laid
   out in memory as so:
   - The first  element will refer to the GUTw followed by the GUTh, x and y accordingly

   Each each other vec4 will belong to a Texture Info packet

   - Texture info is packed as follows in 2 elements:
     . the first element holds whether or not this Texture info refers to a texture that should be used
       If the x value is above .5, this texture info block is valid and should be processed. If
       the x value is below -.5, the TexInfo is invalid or marks the end of textures to be read and texture reading should cease

     . the second element holds the texture base coordinates, x, y, z, w   

   - This continues until the implementation limit is reached, which is hardcoded into the 
     shader.

   - This implementation will hold up to 511 simultaneous texture bindings.

*/
"uniform _binding_DynacoeTexture {\n"

"   vec4[1 + 511*2] _impl_Dynacoe_TexInfo;\n"
"};\n"

"uniform _binding_DynacoeTexture2 {\n"
"   vec4[1 + 511*2] _impl_Dynacoe_TexInfo2;\n"
"};\n"


"uniform _binding_DynacoeLighting {\n"

/*


    // light struct (8 floats)
    
    vec3  lightPos   -   if point light or spot light, its the position.
                            if a directional light, refers to the directional vector


    float lightIntensity-   light strength multiplier.

*/
"   vec4 _impl_Dynacoe_LightData[1024];\n" //512
"};\n"


"uniform _binding_DynacoeLighting2 {\n"

/*


    // light struct (4 floats)
    vec3  lightColor    -   rgb of light

    float lightType    -   
                            [1, 10] - point light
                            [10, 20] - directional light
                            [20, 30] - spotlight
    

*/
"   vec4 _impl_Dynacoe_LightData2[1024];\n" //512
"};\n"


// Internal interface for standard functions.
"#define _BSI_Dynacoe_Texture_GUT_x         (_impl_Dynacoe_TexInfo[0].x)\n"
"#define _BSI_Dynacoe_Texture_GUT_y         (_impl_Dynacoe_TexInfo[0].y)\n"
"vec2  _BSI_Dynacoe_Texture_pos (in int i) { return _impl_Dynacoe_TexInfo[i+1].xy;}\n"
"vec2  _BSI_Dynacoe_Texture_dims(in int i) { return _impl_Dynacoe_TexInfo[i+1].zw;}\n"
"float _BSI_Dynacoe_Texture_exists(in int i) { return _impl_Dynacoe_TexInfo2[i].x;}\n"

// textures
"uniform sampler2D _BSI_Dynacoe_BaseTexture;\n"
"uniform sampler2D _BSI_Dynacoe_FBtexture;\n"
"uniform int       _BSI_Dynacoe_hasFBtexture;\n"

// lights
"vec3  _BSI_Dynacoe_Light_pos(in int i)       {return _impl_Dynacoe_LightData[i].xyz;}\n"
"float _BSI_Dynacoe_Light_intensity(in int i) {return _impl_Dynacoe_LightData[i].w;}\n"

"vec3  _BSI_Dynacoe_Light_color(in int i)      {return _impl_Dynacoe_LightData2[i].xyz;}\n"
"float _BSI_Dynacoe_Light_type(in int i)      {return _impl_Dynacoe_LightData2[i].w;}\n"


#include "base.glsl.h"










static const GLuint STATIC_PROGRAM_GL3__UNIFORM_INDEX__VIEW       = 0;
static const GLuint STATIC_PROGRAM_GL3__UNIFORM_INDEX__PROJECTION = 1;
static const GLuint STATIC_PROGRAM_GL3__UNIFORM_INDEX__MATERIAL   = 2;
static const GLuint STATIC_PROGRAM_GL3__UNIFORM_INDEX__MODEL      = 3;
static const GLuint STATIC_PROGRAM_GL3__UNIFORM_INDEX__TEXTURE    = 4;
static const GLuint STATIC_PROGRAM_GL3__UNIFORM_INDEX__LIGHTING   = 5;
static const GLuint STATIC_PROGRAM_GL3__UNIFORM_INDEX__TEXTURE2   = 6;
static const GLuint STATIC_PROGRAM_GL3__UNIFORM_INDEX__LIGHTING2  = 7;


StaticProgram_GL3_1::StaticProgram_GL3_1(){}


bool StaticProgram_GL3_1::Set(const char * vertSrc_raw,
                     const char * fragSrc_raw,
                     const std::string & name,
                     RenderBuffer * v,
                     RenderBuffer * p,
                     RenderBuffer * DynacoeTexture_uniformID, // 4 components: xy pos, zw dims 0 -> GUT, 
                     RenderBuffer * DynacoeTextureInfo_uniformID, // 4 component: x- enabled
                     RenderBuffer * DynacoeLighting_uniformID, // 4 components: xyz position, w intensity
                     RenderBuffer * DynacoeLightingInfo_uniformID // 4 components: xyz color, w type


                     ) 
                     
                     {
                         
    log = "";
    passedTexture = false;
    progName = name;
    
    Chain fragSrc;
    Chain vertSrc;

    viewUniform         = v;
    projectionUniform   = p;
    textureUniformData  = DynacoeTexture_uniformID;
    lightUniformData    = DynacoeLighting_uniformID;
    textureUniformInfo  = DynacoeTextureInfo_uniformID;
    lightUniformInfo    = DynacoeLightingInfo_uniformID;
    
    
    std::string header;
    if (GLVersionQuery(GL_Version3_1)) {
        header = "#version 140\n";
    } else if (GLVersionQuery(GL_Version3_0 | GL_UniformBufferObject)) {
        header = "#version 130\n"
                 "#extension GL_ARB_uniform_buffer_object: require\n";
    }


    fragSrc << header.c_str() << DynacoeProgramHeader << fragSrc_raw;
    vertSrc << header.c_str() 
            << "in mat2x4 _impl_Dynacoe_Input;\n"
            << "in vec4   Dynacoe_Input;\n"
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
    glBindAttribLocation(progID, 0, vertex_name_c);
    glBindAttribLocation(progID, 13, vertex_aux_name_c);


    glLinkProgram(progID);

    glGetProgramiv(progID, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        log += "[Dynaoce::OpenGL]: Linking for program failed (But compilation of both stages was successful!)\n";
        printProgramLog(progID);
        return false;
    }    

     
    
    

    GLuint uniformIndex = glGetUniformBlockIndex(progID, "_binding_DynacoeView");
    if(uniformIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(progID, uniformIndex, STATIC_PROGRAM_GL3__UNIFORM_INDEX__VIEW);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 0, viewUniform->GetID());
    } else {
        log += "Reqested ignoring of viewing matrix (optimized out?)\n";
    }
    uniformIndex = glGetUniformBlockIndex(progID, "_binding_DynacoeProjection");
    if (uniformIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(progID, uniformIndex, STATIC_PROGRAM_GL3__UNIFORM_INDEX__PROJECTION);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 1, projectionUniform->GetID());
    } else {
        log += "Reqested ignoring of projection matrix (optimized out?)\n";
    }


    uniformIndex = glGetUniformBlockIndex(progID, "_binding_DynacoeMaterial");
    if(uniformIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(progID, uniformIndex, STATIC_PROGRAM_GL3__UNIFORM_INDEX__MATERIAL);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 0, viewUniform->GetID());
    } else {
        log += "Reqested ignoring of material data (optimized out?)\n";
    }
    uniformIndex = glGetUniformBlockIndex(progID, "_binding_DynacoeModel");
    if (uniformIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(progID, uniformIndex, STATIC_PROGRAM_GL3__UNIFORM_INDEX__MODEL);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 1, projectionUniform->GetID());
    } else {
        log += "Reqested ignoring of model transform (optimized out?)\n";
    }
    
    


    
    uniformIndex = glGetUniformBlockIndex(progID, "_binding_DynacoeTexture");
    if (uniformIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(progID, uniformIndex, STATIC_PROGRAM_GL3__UNIFORM_INDEX__TEXTURE);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 4, textureUniform->GetID());
    } else {
        log+= "Reqested ignoring of texture data (optimized out?)\n" ;
    }
    uniformIndex = glGetUniformBlockIndex(progID, "_binding_DynacoeTexture2");
    if (uniformIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(progID, uniformIndex, STATIC_PROGRAM_GL3__UNIFORM_INDEX__TEXTURE2);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 4, textureUniform->GetID());
    } 




    uniformIndex = glGetUniformBlockIndex(progID, "_binding_DynacoeLighting");
    if (uniformIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(progID, uniformIndex, STATIC_PROGRAM_GL3__UNIFORM_INDEX__LIGHTING);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 5, lightUniform->GetID());
    } else {
        log+= "Reqested ignoring of lighting data (optimized out?)\n";
    }
    uniformIndex = glGetUniformBlockIndex(progID, "_binding_DynacoeLighting2");
    if (uniformIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(progID, uniformIndex, STATIC_PROGRAM_GL3__UNIFORM_INDEX__LIGHTING2);
        //glBindBufferBase(GL_UNIFORM_BUFFER, 5, lightUniform->GetID());
    } 



    int status = glGetError();
    if (status != GL_NO_ERROR) {
        log+= "[Dynacoe::OpenGL]: FATAL WARNING: An error occured while linking buffers.\n";
        log+= "                   Perhaps a bad buffer id was given, or the GUT was invalid?\n";

        char * bf = new char[2048];
        int numActive, dummy;
        glGetProgramiv(progID, GL_ACTIVE_UNIFORM_BLOCKS, &numActive);
        log += "Currently active uniform buffers:\n";
        for(int i = 0; i < numActive; ++i) {
            glGetActiveUniformBlockName(progID, i, 2048, &dummy, bf);
            log += (std::string(bf) + "\n");
        }
        return FailSet();
    } else {
        log+= "[Dynacoe::OpenGL]: " + name + " successfully built";
    }

    return true;
}

std::string StaticProgram_GL3_1::GetLog() {
    return log;
}

void StaticProgram_GL3_1::SetUniformBindings(RenderBuffer * mat, RenderBuffer * model) {
    glBindBufferBase(GL_UNIFORM_BUFFER, STATIC_PROGRAM_GL3__UNIFORM_INDEX__VIEW,       viewUniform->GenerateBufferID());
    glBindBufferBase(GL_UNIFORM_BUFFER, STATIC_PROGRAM_GL3__UNIFORM_INDEX__PROJECTION, projectionUniform->GenerateBufferID());
    glBindBufferBase(GL_UNIFORM_BUFFER, STATIC_PROGRAM_GL3__UNIFORM_INDEX__TEXTURE,    textureUniformData->GenerateBufferID());
    glBindBufferBase(GL_UNIFORM_BUFFER, STATIC_PROGRAM_GL3__UNIFORM_INDEX__LIGHTING,   lightUniformData->GenerateBufferID());
    glBindBufferBase(GL_UNIFORM_BUFFER, STATIC_PROGRAM_GL3__UNIFORM_INDEX__TEXTURE2,   textureUniformInfo->GenerateBufferID());
    glBindBufferBase(GL_UNIFORM_BUFFER, STATIC_PROGRAM_GL3__UNIFORM_INDEX__LIGHTING2,  lightUniformInfo->GenerateBufferID());        
    glBindBufferBase(GL_UNIFORM_BUFFER, STATIC_PROGRAM_GL3__UNIFORM_INDEX__MODEL,      model->GenerateBufferID());
    glBindBufferBase(GL_UNIFORM_BUFFER, STATIC_PROGRAM_GL3__UNIFORM_INDEX__MATERIAL,   mat->GenerateBufferID());

    assert(glGetError() == GL_NO_ERROR);
}




void StaticProgram_GL3_1::SetVertexBinding(int vertexBufferID) {

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void *) 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void *) (sizeof(GLfloat)*4));
    glVertexAttribPointer(13, 4, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void *) (sizeof(GLfloat)*8));





    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void StaticProgram_GL3_1::printShaderLog(int shaderID) {
    int logLength;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
    std::string l;
    l.resize(logLength);

    glGetShaderInfoLog(shaderID, logLength, &logLength, &l[0]);
    
    log += std::string(l);
}

void StaticProgram_GL3_1::printProgramLog(int shaderID) {
    int logLength;
    glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
    std::string l;
    l.resize(logLength);

    glGetProgramInfoLog(shaderID, logLength, &logLength, &l[0]);
    
    log += std::string(l);
}





void StaticProgram_GL3_1::Run(uint32_t * indexList, uint32_t numIndices, RenderBuffer * vertexBufferID, RenderBuffer * matBufferID, RenderBuffer * modelBufferID, GLuint baseTex, GLuint fbTex) {
    glUseProgram(progID);

    glActiveTexture(GetBaseTextureActiveIndex());
    glBindTexture(GL_TEXTURE_2D,  baseTex);
    
    glActiveTexture(GetSourceFBTextureActiveIndex());
    glBindTexture(GL_TEXTURE_2D, fbTex);
    

    
    SetUniformBindings(matBufferID, modelBufferID);
    SetVertexBinding(vertexBufferID->GenerateBufferID());    
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID->GenerateBufferID());
    
    
    


    int location = glGetUniformLocation(progID, "_BSI_Dynacoe_hasFBtexture");
    glUniform1i(location, fbTex);

    if (!passedTexture) {
        int texLoc = glGetUniformLocation(progID, "_BSI_Dynacoe_BaseTexture");
        glUniform1i(texLoc, GetBaseTextureActiveIndex() - GL_TEXTURE0);

        texLoc = glGetUniformLocation(progID, "_BSI_Dynacoe_FBtexture");
        glUniform1i(texLoc, GetSourceFBTextureActiveIndex() - GL_TEXTURE0);
        passedTexture = true;
    } 


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(13);

    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexList);

    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glEnableVertexAttribArray(13);
}

bool StaticProgram_GL3_1::FailSet() {
    glDeleteShader(vertID);
    glDeleteShader(fragID);
    glDeleteProgram(progID);
    return false;
}





#endif
