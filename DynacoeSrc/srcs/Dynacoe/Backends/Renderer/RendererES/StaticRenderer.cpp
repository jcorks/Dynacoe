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
#include <GLES2/gl2.h>
#include <cassert>
#include "static_es.glsl"

using namespace Dynacoe;






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

        uniformLocation_fragTex_slots = glGetUniformLocation(handle, "fragTex_slots");
        uniformLocation_TexInfo_coords = glGetUniformLocation(handle, "_impl_Dynacoe_TexInfo_coords");
        uniformLocation_TexInfo_handle = glGetUniformLocation(handle, "_impl_Dynacoe_TexInfo_handle");

        uniformLocation_LightData1 = glGetUniformLocation(handle, "_impl_Dynacoe_LightData1");
        uniformLocation_LightData2 = glGetUniformLocation(handle, "_impl_Dynacoe_LightData2");




        if (attribLocation_pos < 0)      log << "Position attribute was opmtimized out.\n";
        if (attribLocation_normal < 0)   log << "Normal attribute was opmtimized out.\n";
        if (attribLocation_uvs < 0)      log << "UVs attribute was opmtimized out.\n";
        if (attribLocation_userData < 0) log << "User data attribute was opmtimized out.\n";


        if (uniformLocation_ViewTransform) log << "ViewTransform optimized out.\n";
        if (uniformLocation_ViewNormalTransform) log << "ViewNormalTransform optimized out.\n";
        if (uniformLocation_ProjectionTransform) log << "ProjectionTransform optimized out.\n";
        if (uniformLocation_MaterialAmbient) log << "MaterialAmbient optimized out.\n";
        if (uniformLocation_MaterialDiffuse) log << "MaterialDiffuse optimized out.\n";
        if (uniformLocation_MaterialSpecular) log << "MaterialSpecular optimized out.\n";

        if (uniformLocation_MaterialData) log << "MaterialData optimized out.\n";
        if (uniformLocation_ModelTransform) log << "ModelTransform optimized out.\n";

        if (uniformLocation_fragTex_slots) log << "Texturing info (FTS) optimized out.\n";
        if (uniformLocation_TexInfo_coords) log << "Texturing info (TIC) optimized out.\n";
        if (uniformLocation_TexInfo_handle) log << "Texturing info (TIH) optimized out.\n";

        if (uniformLocation_LightData1) log << "Lighting info (LD1) optimized out.\n";
        if (uniformLocation_LightData2) log << "Lighting info (LD2) optimized out.\n";

        if (uniformLocation_FBtexture) log << "Framebuffer texture slot optimized out.\n";
        if (uniformLocation_hasFBtexture) log << "Framebuffer texture condition optimized out.\n";



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
    void UseProgram(StaticState * state) {
            
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

    
};















class Dynacoe::StaticRenderer_Data {
  public:
    StaticRenderer_Data(Texture_ES * es) {
        texture = es;


        // flat shading!
        ProgramES * builtIn_flat_ref = new ProgramES(

            ////// VERTEX SHADER //////
            "varying highp   vec2 UV;\n"
            "varying mediump vec3 normals;\n"



            "void main(void) {\n"
            "   gl_Position = Dynacoe_ProjectionTransform * (Dynacoe_ViewTransform  * (Dynacoe_ModelTransform * vec4(Dynacoe_Position, 1.0)));\n"
            "   UV = Dynacoe_UV;\n"
            "   normals = Dynacoe_Normal;\n"
            "}\n",


    
            ////// FRAGMENT SHADER //////

            "varying highp   vec2 UV;\n"
            "varying mediump vec3 normals;\n"


            "void main(void) {\n"
            "   lowp vec4 color;\n"
            "   if (Dynacoe_SlotHasTexture(0))\n"
            "       color = Dynacoe_SampleColor(0, UV);\n"
            "   else\n"
            "       color = vec4(Dynacoe_MaterialDiffuse, 1.0);\n"
            "   if (Dynacoe_CanSampleFramebuffer())\n"
            "       color = .5*color+.5*Dynacoe_SampleFramebuffer(UV);\n"
            "   gl_FragColor = color;\n"
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



    }

    Dynacoe::LookupID builtIn_flat;
    Dynacoe::LookupID builtIn_lighting;
    Dynacoe::Table<ProgramES*> programs;

  private:  
    Texture_ES * texture;   


};








StaticRenderer::StaticRenderer(Texture_ES * t) {
    ES = new StaticRenderer_Data(t);
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

}


RenderBufferID StaticRenderer::GetViewingMatrixID() {
    return RenderBufferID();
}


RenderBufferID StaticRenderer::GetProjectionMatrixID() {
    return RenderBufferID();
}

ProgramID StaticRenderer::ProgramGetBuiltIn(Renderer::BuiltInShaderMode mode) {
    switch(mode) {
      case Renderer::BuiltInShaderMode::BasicShader:   return ES->builtIn_flat;
      case Renderer::BuiltInShaderMode::LightMaterial: return ES->builtIn_lighting;

    }
    return ProgramID();
}












#endif
