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


#include <cstring>
#include <iostream>
#include <cassert>
#include <stack>
#include <Dynacoe/Util/TransformMatrix.h>
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer_ES.h>
#include <Dynacoe/Backends/Renderer/RendererES/Texture_ES.h>
#include <Dynacoe/Backends/Renderer/RendererES/Renderer2D_ES.h>
#include <stdint.h>
#include <stdio.h>

using namespace Dynacoe;



static int VBO_SLOT__POS;
static int VBO_SLOT__COLOR;
static int VBO_SLOT__UVS; // x, y, "z" (-1 if unused)

static int VBO_SLOT__OBJECT_A;
static int VBO_SLOT__OBJECT_B;
static int VBO_SLOT__OBJECT_C;
static int VBO_SLOT__OBJECT_D;

static int UNIFORM_SLOT__CONTEXT_WIDTH_F;
static int UNIFORM_SLOT__CONTEXT_HEIGHT_F;
static int UNIFORM_SLOT__CONTEXT_TRANSFORM_MAT4;
static int UNIFORM_SLOT__FRAG_TEX_I;


struct InterleavedRenderer2DVertex {
    float pos[2];
    float color[4];
    float uvs[3];

    float objectP[16];
};

static const char * vertShader_2D =




"attribute highp  vec2  pos;\n"
"attribute highp  vec4  color;\n"
"attribute highp  vec3  uvs;\n"


"attribute highp  vec4  objectA;\n"
"attribute highp  vec4  objectB;\n"
"attribute highp  vec4  objectC;\n"
"attribute highp  vec4  objectD;\n"




"uniform float contextWidth;\n"
"uniform float contextHeight;\n"
"uniform mat4 contextTransform;\n"

"varying lowp  vec4  fragColor;\n"
"varying highp vec3  fragTexCoord3;\n\n"
// applies a full transform for 2D coordinates

"void main(void) {\n"
"   highp mat4 m = mat4(objectA, objectB, objectC, objectD);\n"
"   highp vec4 tfPos = m * vec4(pos.x, pos.y, 0.0, 1.0);\n"
"   highp vec4 p = contextTransform * tfPos;\n"



// output to normalized coordinates

"   gl_Position.x = (p.x / contextWidth) * 2.0 - 1.0;\n"
"   gl_Position.y = -1.0*((p.y / contextHeight) * 2.0 - 1.0);\n"
"   gl_Position.z = 0.0;\n"
"   gl_Position.w = 1.0;\n"

"   fragColor     = color;\n"
"   fragTexCoord3 = uvs;\n"



"   \n"
"}\n";




static const char * fragShader_2D =

"varying lowp vec4   fragColor;\n"
"varying highp vec3  fragTexCoord3;\n"



"uniform sampler2D fragTex_slots[16];\n"
"lowp vec4 texture2D3(highp vec3 tex) {\n"
"   int index = int(tex.z);\n"
"   if (index == 0) return texture2D(fragTex_slots[0], tex.xy);\n"
"   if (index == 1) return texture2D(fragTex_slots[1], tex.xy);\n"
"   if (index == 2) return texture2D(fragTex_slots[2], tex.xy);\n"
"   if (index == 3) return texture2D(fragTex_slots[3], tex.xy);\n"
"   if (index == 4) return texture2D(fragTex_slots[4], tex.xy);\n"
"   if (index == 5) return texture2D(fragTex_slots[5], tex.xy);\n"
"   if (index == 6) return texture2D(fragTex_slots[6], tex.xy);\n"
"   if (index == 7) return texture2D(fragTex_slots[7], tex.xy);\n"
"   if (index == 8) return texture2D(fragTex_slots[8], tex.xy);\n"
"   if (index == 9) return texture2D(fragTex_slots[9], tex.xy);\n"
"   if (index == 10) return texture2D(fragTex_slots[10], tex.xy);\n"
"   if (index == 11) return texture2D(fragTex_slots[11], tex.xy);\n"
"   if (index == 12) return texture2D(fragTex_slots[12], tex.xy);\n"
"   if (index == 13) return texture2D(fragTex_slots[13], tex.xy);\n"
"   if (index == 14) return texture2D(fragTex_slots[14], tex.xy);\n"
"   if (index == 15) return texture2D(fragTex_slots[15], tex.xy);\n"
"}\n"


"void main(void) {\n"
"   if (fragTexCoord3.z > -.5) {\n"
"       lowp vec4 temp = texture2D3(fragTexCoord3);\n"
"       gl_FragColor.r = temp.r * fragColor.r;\n"
"       gl_FragColor.g = temp.g * fragColor.g;\n"
"       gl_FragColor.b = temp.b * fragColor.b;\n"
"       gl_FragColor.a = temp.a * fragColor.a;\n"

"   } else {\n"
"       gl_FragColor = fragColor;\n"
"   }\n"


"}\n";




struct Object2D_Reference {
    std::vector<uint32_t> vertices;
    Renderer::Render2DObjectParameters params;
};

struct Vertex2D_Reference {
    Vertex2D_Reference() 
        : object(UINT32_MAX)
    {
    }
    uint32_t object;
    float localUVx;
    float localUVy;
    int   localTexture;
};


class Renderer2DData {
  public:

    std::vector<Object2D_Reference> objects;
    std::vector<Vertex2D_Reference> vertices;

    std::stack<uint32_t> deadObjects;
    std::stack<uint32_t> deadVertices;


    uint32_t * queued;
    uint32_t   queuedSize;
    uint32_t   queuedAllocated;

    RenderBuffer * mainVBO;
    GLint programHandle;
    Texture_ES * texture;





    void CreateProgram();
    std::string GetProgramVertexLog() const;
    std::string GetProgramFragmentLog() const;
    std::string GetProgramLinkLog() const;
    


    uint32_t vboVertexCount;    
    uint32_t vboVertexIndex;
    uint32_t NewSystemVertex() {
        if (vboVertexIndex < vboVertexCount) {
            vboVertexIndex++;
            return vboVertexIndex-1;
        }

        uint32_t newSize = vboVertexCount*1.4;
        float * copy = (float*)malloc(sizeof(float)*25*newSize);
        mainVBO->GetData(copy, 0, 25*vboVertexCount);
        mainVBO->Define(copy, 25*newSize);
        vboVertexCount = newSize;
        return NewSystemVertex();
    }

    

    

    void GetSystemVertex(uint32_t index, InterleavedRenderer2DVertex * target) {
        mainVBO->GetData(
            (float*)target,
            index*25,
            25
        );
    }


    void SetSystemVertex(uint32_t index, const InterleavedRenderer2DVertex * data) {
        mainVBO->UpdateData(
            (float*)data,
            index*25,
            25
        );

    }


    static void OnRebaseTextures(void * data) {
        Renderer2DData * ES = (Renderer2DData *)data;
        auto refs = ES->vertices;
        
        uint32_t numV = refs.size();
        Vertex2D_Reference * vtx;
        InterleavedRenderer2DVertex * vData;
        
        InterleavedRenderer2DVertex * dataCopy = (InterleavedRenderer2DVertex*)malloc(sizeof(InterleavedRenderer2DVertex)*numV);
        ES->mainVBO->GetData((float*)dataCopy, 0, numV*25);

        for(uint32_t i = 0; i < numV; ++i) {
            vtx = &refs[i];
            if (vtx->localTexture >= 0) {
                vData = dataCopy+i;
                vData->uvs[0] = vtx->localUVx;
                vData->uvs[1] = vtx->localUVy;

                ES->texture->TranslateCoords(
                    vData->uvs+0,
                    vData->uvs+1,
                    vtx->localTexture
                );

                vData->uvs[2] = ES->texture->GetSlotForTexture(vtx->localTexture);

                /*printf(
                    "%d (on %f) -> %d %d\n",
                
                    vtx->localTexture,
                    vData->uvs[2],
                    ES->texture->GetSubTextureBounds(vtx->localTexture)[2],
                    ES->texture->GetSubTextureBounds(vtx->localTexture)[3]
                );*/

            }
   
        }
        ES->mainVBO->UpdateData((float*)dataCopy, 0, numV*25);
        free(dataCopy);

        printf("Texture atlas changed. Rebased %d vertices.\n", numV);
    }


};



Renderer2D::Renderer2D(Texture_ES * textureSrc) {

    data = new Renderer2DData;
    data->mainVBO = new RenderBuffer();
    data->mainVBO->Define(nullptr, 1024*25);
    data->vboVertexCount = 1024;
    data->vboVertexIndex = 0;
    data->texture = textureSrc;
    
    data->CreateProgram();

    data->texture->AddRebaseCallback(
        Renderer2DData::OnRebaseTextures,
        data
    );
    
    if (!data->programHandle) {
        std::cout << "Uh oh! 2D rendering engine failed!" << std::endl;
        exit(0);
    }

    data->queuedSize = 0;
    data->queuedAllocated = 1024;
    data->queued = (uint32_t*)malloc(sizeof(uint32_t)*1024);
}


uint32_t Renderer2D::Add2DObject() {
    if (!data->deadObjects.empty()) {
        uint32_t object = data->deadObjects.top();
        data->deadObjects.pop();
        return object;
    }
    data->objects.push_back({});
    return data->objects.size()-1;
}

void Renderer2D::Remove2DObject(uint32_t id) {
    data->deadObjects.push(id);
    Object2D_Reference & ref = data->objects[id];
    uint32_t len = ref.vertices.size();
    Vertex2D_Reference * vtx;
    for(uint32_t i = 0; i < len; ++i) {
        vtx = &data->vertices[ref.vertices[i]];
        vtx->object = UINT32_MAX;        
    }
    ref.vertices.clear();
}

void Renderer2D::Set2DObjectParameters(uint32_t object, Renderer::Render2DObjectParameters params) {
    if(object >= data->objects.size()) return;

    Object2D_Reference & ref = data->objects[object];
    ref.params = params;
    uint32_t len = ref.vertices.size();
    InterleavedRenderer2DVertex vertex;
    for(uint32_t i = 0; i < len; ++i) {
        data->GetSystemVertex(ref.vertices[i], &vertex);
        memcpy(vertex.objectP, params.data, sizeof(float)*16);
        data->SetSystemVertex(ref.vertices[i], &vertex);
    }

}


uint32_t Renderer2D::Add2DVertex() {
    if (!data->deadVertices.empty()) {
        uint32_t object = data->deadVertices.top();
        data->deadVertices.pop();
        return object;
    }
    data->vertices.push_back({});
    data->NewSystemVertex(); // should match in index value
    return data->vertices.size()-1;
}

void Renderer2D::Remove2DVertex(uint32_t id) {
    data->deadVertices.push(id);
    // objects may still holds dead refs to vertices. shouldnt be an issue
    //
}

void Renderer2D::Set2DVertex(uint32_t vertex, Renderer::Vertex2D params) {
    if(vertex >= data->vertices.size()) return;
    Vertex2D_Reference * refVertex = &data->vertices[vertex];
        
    // remove old object's reference to the vertex
    uint32_t oldObj = refVertex->object;
    refVertex->object = (uint32_t)params.object;

    if (oldObj == refVertex->object) {
    } else {
        data->objects[refVertex->object].vertices.push_back(vertex);


        if (oldObj != UINT32_MAX) {
            Object2D_Reference & ref = data->objects[oldObj];
            uint32_t len = ref.vertices.size();
            for(uint32_t i = 0; i < len; ++i) {
                if (ref.vertices[i] == vertex) {
                    ref.vertices.erase(ref.vertices.begin()+i);
                    break;
                }
            }
        }
    }

    refVertex->localUVx = params.texX;
    refVertex->localUVy = params.texY;
    refVertex->localTexture = (int)params.useTex;

    Object2D_Reference & ref = data->objects[refVertex->object];
    InterleavedRenderer2DVertex vData;
    data->GetSystemVertex(vertex, &vData);
    vData.pos[0] = params.x;
    vData.pos[1] = params.y;
    vData.color[0] = params.r;
    vData.color[1] = params.g;
    vData.color[2] = params.b;
    vData.color[3] = params.a;
    vData.uvs[0] = params.texX;
    vData.uvs[1] = params.texY;
    vData.uvs[2] = data->texture->GetSlotForTexture(refVertex->localTexture); 

    // get real coordinates for the texture from the atlas   
    if (refVertex->localTexture >= 0) {
        data->texture->TranslateCoords(
            vData.uvs+0, 
            vData.uvs+1,
            refVertex->localTexture
        );
    } else {
        vData.uvs[2] = (float)-1;
    }

    memcpy(vData.objectP, ref.params.data, sizeof(float)*16);
    data->SetSystemVertex(vertex, &vData);

}


Renderer::Vertex2D Renderer2D::Get2DVertex(uint32_t vertex) {
    if(vertex >= data->vertices.size()) return {};
    Vertex2D_Reference * refVertex = &data->vertices[vertex];

    InterleavedRenderer2DVertex vData;
    data->GetSystemVertex(vertex, &vData);    
    Renderer::Vertex2D params;
    params.x = vData.pos[0];
    params.y = vData.pos[1];
    params.r = vData.color[0];
    params.g = vData.color[1];
    params.b = vData.color[2];
    params.a = vData.color[3];
    params.texX = refVertex->localUVx;
    params.texY = refVertex->localUVy;
    params.useTex = (float)refVertex->localTexture;
    params.object = (float)refVertex->object;
    return params;
}



void Renderer2D::Queue2DVertices(const uint32_t * indices, uint32_t count) {
    while (data->queuedAllocated <= data->queuedSize+count) {
        uint32_t newSize = (data->queuedSize+count)*1.4;
        uint32_t * newData = (uint32_t*)malloc(newSize*sizeof(uint32_t));
        printf("Resized from %d to %d\n", data->queuedAllocated, newSize);
        memcpy(newData, data->queued, data->queuedAllocated*sizeof(uint32_t));
        free(data->queued);
        data->queuedAllocated = newSize;
        data->queued = newData;

    }
    memcpy(data->queued+data->queuedSize, indices, count*sizeof(uint32_t));
    data->queuedSize += count;
}

static int ACTIVE_SLOTS[128];
static int ACTIVE_IDS  [128];

uint32_t Renderer2D::Render2DVertices(GLenum drawMode, const Renderer::Render2DStaticParameters & params) {
    if (!data->queuedSize) return 0;
    /*
    if (data->lastW != data->textureSrc->GetTextureW() ||
        data->lastH != data->textureSrc->GetTextureH()) {
        data->RebaseTextures();
        data->lastW = data->textureSrc->GetTextureW();
        data->lastH = data->textureSrc->GetTextureH();
    }
    */
    

    int active;
    GLint activeID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeID);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active);

    glUseProgram(data->programHandle);  

    int slots = data->texture->GetActiveTextureSlots(ACTIVE_SLOTS, ACTIVE_IDS);
    for(uint32_t i = 0; i < slots; ++i) {
        glActiveTexture(ACTIVE_SLOTS[i]);
        glBindTexture(GL_TEXTURE_2D, ACTIVE_IDS[i]);
        ACTIVE_SLOTS[i] -= GL_TEXTURE0;
    }
    glUniform1iv(UNIFORM_SLOT__FRAG_TEX_I, slots, ACTIVE_SLOTS);
    





    


    


    GLuint currentVbo = data->mainVBO->GenerateBufferID();
    glBindBuffer(GL_ARRAY_BUFFER, currentVbo);
    

    // TODO find a way to not need to query every fram ewhether an attrib pointer is opted out
    // TODO combine object into a single matrix    


    if (VBO_SLOT__POS>=0)      glVertexAttribPointer(VBO_SLOT__POS,       2, GL_FLOAT, GL_FALSE, sizeof(float)*25, (void*)0);
    if (VBO_SLOT__COLOR>=0)    glVertexAttribPointer(VBO_SLOT__COLOR,     4, GL_FLOAT, GL_FALSE, sizeof(float)*25, (void*)(sizeof(GLfloat) * 2));
    if (VBO_SLOT__UVS>=0)      glVertexAttribPointer(VBO_SLOT__UVS,       3, GL_FLOAT, GL_FALSE, sizeof(float)*25, (void*)(sizeof(GLfloat) * 6));
    if (VBO_SLOT__OBJECT_A>=0) glVertexAttribPointer(VBO_SLOT__OBJECT_A,   4, GL_FLOAT, GL_FALSE, sizeof(float)*25, (void*)(sizeof(GLfloat) * 9));
    if (VBO_SLOT__OBJECT_B>=0) glVertexAttribPointer(VBO_SLOT__OBJECT_B,   4, GL_FLOAT, GL_FALSE, sizeof(float)*25, (void*)(sizeof(GLfloat) * 13));
    if (VBO_SLOT__OBJECT_C>=0) glVertexAttribPointer(VBO_SLOT__OBJECT_C,   4, GL_FLOAT, GL_FALSE, sizeof(float)*25, (void*)(sizeof(GLfloat) * 17));
    if (VBO_SLOT__OBJECT_D>=0) glVertexAttribPointer(VBO_SLOT__OBJECT_D,   4, GL_FLOAT, GL_FALSE, sizeof(float)*25, (void*)(sizeof(GLfloat) * 21));
    


    
    
    if (VBO_SLOT__POS>=0)      glEnableVertexAttribArray(VBO_SLOT__POS);
    if (VBO_SLOT__COLOR>=0)    glEnableVertexAttribArray(VBO_SLOT__COLOR);
    if (VBO_SLOT__UVS>=0)      glEnableVertexAttribArray(VBO_SLOT__UVS);
    if (VBO_SLOT__OBJECT_A>=0) glEnableVertexAttribArray(VBO_SLOT__OBJECT_A);
    if (VBO_SLOT__OBJECT_B>=0) glEnableVertexAttribArray(VBO_SLOT__OBJECT_B);
    if (VBO_SLOT__OBJECT_C>=0) glEnableVertexAttribArray(VBO_SLOT__OBJECT_C);
    if (VBO_SLOT__OBJECT_D>=0) glEnableVertexAttribArray(VBO_SLOT__OBJECT_D);
    
    glUniform1f(UNIFORM_SLOT__CONTEXT_HEIGHT_F, params.contextHeight);
    glUniform1f(UNIFORM_SLOT__CONTEXT_WIDTH_F, params.contextWidth);
    glUniformMatrix4fv(UNIFORM_SLOT__CONTEXT_TRANSFORM_MAT4, 1, true, params.contextTransform);    

    
    glDrawElements(
        drawMode, 
        data->queuedSize,
        GL_UNSIGNED_INT,
        data->queued
    );



    

 

    
    if (VBO_SLOT__POS>=0)      glDisableVertexAttribArray(VBO_SLOT__POS);
    if (VBO_SLOT__COLOR>=0)    glDisableVertexAttribArray(VBO_SLOT__COLOR);
    if (VBO_SLOT__UVS>=0)      glDisableVertexAttribArray(VBO_SLOT__UVS);
    if (VBO_SLOT__OBJECT_A>=0) glDisableVertexAttribArray(VBO_SLOT__OBJECT_A);
    if (VBO_SLOT__OBJECT_B>=0) glDisableVertexAttribArray(VBO_SLOT__OBJECT_B);
    if (VBO_SLOT__OBJECT_C>=0) glDisableVertexAttribArray(VBO_SLOT__OBJECT_C);
    if (VBO_SLOT__OBJECT_D>=0) glDisableVertexAttribArray(VBO_SLOT__OBJECT_D);


    glUseProgram(0);  

    

    assert(glGetError() == GL_NO_ERROR);
    uint32_t size = data->queuedSize;
    Clear2DQueue();

    glActiveTexture(active);
    glBindTexture(GL_TEXTURE_2D, activeID);

    
    return size;
}

void Renderer2D::Clear2DQueue() {
    data->queuedSize = 0;
}


void Renderer2DData::CreateProgram() {
    programHandle = glCreateProgram();

    GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint vertID = glCreateShader(GL_VERTEX_SHADER);


    std::string vSrc;
    std::string fSrc;
    std::string header;

    header = "";
    vSrc = header + vertShader_2D;
    fSrc = header + fragShader_2D;


    const char * vPtr = vSrc.c_str();
    const char * fPtr = fSrc.c_str();

    glShaderSource(fragID, 1, &fPtr, NULL);
    glShaderSource(vertID, 1, &vPtr, NULL);



    glAttachShader(programHandle, fragID);
    glAttachShader(programHandle, vertID);

    glCompileShader(fragID);
    glCompileShader(vertID);

    assert(glGetError() == GL_NO_ERROR);

    int success =0;
    glGetShaderiv(fragID, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        std::cout << "[Dynacoe::Renderer2D]: Fragment shader failed to compile:" << std::endl;
        int logLength;
        glGetShaderiv(fragID, GL_INFO_LOG_LENGTH, &logLength);
        char * log = (char*)malloc(logLength+1);
        glGetShaderInfoLog(fragID, logLength, &logLength, log);
        std::cout << log << std::endl;
        free(log);
        assert(0);
    }


    glGetShaderiv(vertID, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        std::cout << "[Dynacoe::Renderer2D]: Vertex shader failed to compile:" << std::endl;
        int logLength;
        glGetShaderiv(vertID, GL_INFO_LOG_LENGTH, &logLength);
        char * log = (char*)malloc(logLength+1);
        glGetShaderInfoLog(vertID, logLength, &logLength, log);
        std::cout << log << std::endl;
        free(log);
        assert(0);
    }

    // Binding is always applied fo rth next link
    glLinkProgram(programHandle);

    VBO_SLOT__POS   = glGetAttribLocation(programHandle, "pos");
    VBO_SLOT__COLOR = glGetAttribLocation(programHandle, "color");
    VBO_SLOT__UVS   = glGetAttribLocation(programHandle, "uvs");
    VBO_SLOT__OBJECT_A = glGetAttribLocation(programHandle, "objectA");
    VBO_SLOT__OBJECT_B = glGetAttribLocation(programHandle, "objectB");
    VBO_SLOT__OBJECT_C = glGetAttribLocation(programHandle, "objectC");
    VBO_SLOT__OBJECT_D = glGetAttribLocation(programHandle, "objectD");

    UNIFORM_SLOT__CONTEXT_HEIGHT_F = glGetUniformLocation(programHandle, "contextHeight");
    UNIFORM_SLOT__CONTEXT_WIDTH_F  = glGetUniformLocation(programHandle, "contextWidth");
    UNIFORM_SLOT__CONTEXT_TRANSFORM_MAT4 = glGetUniformLocation(programHandle, "contextTransform");
    UNIFORM_SLOT__FRAG_TEX_I = glGetUniformLocation(programHandle, "fragTex_slots");

    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        std::cout << "[Dynaoce::OpenGL]: Linking for program failed" << std::endl;

        int logLength;
        glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);
        char * log = (char*)malloc(logLength+1);
        glGetProgramInfoLog(programHandle, logLength, &logLength, log);
        std::cout << log << std::endl;
        free(log);
        assert(0);
    }
    assert(glGetError() == GL_NO_ERROR);
}

#endif


















