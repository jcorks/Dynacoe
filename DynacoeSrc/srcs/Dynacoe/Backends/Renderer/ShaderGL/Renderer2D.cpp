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

#include <Dynacoe/Backends/Renderer/ShaderGL/GLVersionQuery.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/ShaderProgram.h>
#include <cstring>
#include <iostream>
#include <cassert>
#include <Dynacoe/Util/TransformMatrix.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer_Tex1D.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer_GL2_1.h>

using namespace Dynacoe;

const int resize_block_addition_elements = 16*10*10;


const int VBO_SLOT__POS    = 0;
const int VBO_SLOT__COLOR  = 4;
const int VBO_SLOT__UVS    = 5;
const int VBO_SLOT__OBJECT = 6;


static const char * vertShader_2D =






"in  vec2  pos;\n"
"in  vec4  color;\n"
"in  vec3  uvs;\n"
"in  float object;\n"
"uniform float contextWidth;\n"
"uniform float contextHeight;\n"
"uniform mat4 contextTransform;\n"
"uniform sampler1D objectData;\n"
"uniform float atlasWidth;\n"
"uniform float atlasHeight;\n"
"uniform float objectSizeUnits;\n"

"out float fragUseTex;\n"
"out vec4  fragColor;\n"
"out vec2  fragTexCoord;\n\n"
// applies a full transform for 2D coordinates

"void main(void) {\n"
"   vec4 objectP1 = texelFetch(objectData, int(object*4.f), 0);\n"
"   vec4 objectP2 = texelFetch(objectData, int(object*4.f+1), 0);\n"
"   vec4 objectP3 = texelFetch(objectData, int(object*4.f+2), 0);\n"
"   vec4 objectP4 = texelFetch(objectData, int(object*4.f+3), 0);\n"
"   mat4 m = mat4(objectP1, objectP2, objectP3, objectP4);\n"
"   vec4 position = contextTransform * (m * vec4(pos.x, pos.y, 0, 1.f));\n"
// output to normalized coordinates


"   gl_Position = vec4((position.x / contextWidth) * 2.0 - 1.0, -1*((position.y / contextHeight) * 2.0 - 1.0), 0, 1);\n"

"   fragColor    = color;\n"
"   fragUseTex   = uvs.z;\n"
"   fragTexCoord = uvs.xy;\n"
"   \n"
"}\n";




static const char * fragShader_2D =

"in  float fragUseTex;\n"
"in  vec4  fragColor;\n"
"in  vec2  fragTexCoord;\n"



"out vec4  outColor;\n"
"uniform sampler2D fragTex;\n"
"void main(void) {\n"
"   if (fragUseTex > -.5) {\n"
"       vec4 temp = texture(fragTex, fragTexCoord);\n"
"           outColor.r = temp.r * fragColor.r;\n"
"           outColor.g = temp.g * fragColor.g;\n"
"           outColor.b = temp.b * fragColor.b;\n"
"           outColor.a = temp.a * fragColor.a;\n"

//"       outColor = vec4(1.f, 0.f, 1.f, 1.f);\n"
"   } else {\n"
"       outColor = fragColor;\n"
"   }\n"
"}\n";



struct UserVertexData {
    float texX;
    float texY;
};

struct UserObjectData {
   float theta;    
};

class Renderer2DData {
  public:
    RenderBuffer_Tex1D * objectData;
    RenderBuffer_GL2_1 * vertexData;
    ShaderProgram      * program;
    TextureManager     * textureSrc;
    
    std::vector<UserVertexData> userVertexData;
    std::vector<UserObjectData> userObjectData;

    std::stack<uint32_t> deadObjects;
    std::stack<uint32_t> deadVertices;
    std::vector<uint32_t> queued;
    
    
    uint32_t objectID;
    uint32_t vertexID;
    
    int lastW;
    int lastH;
    
    
    void RebaseTextures();
};


Renderer2D::Renderer2D(TextureManager * textureSrc) {
    data = new Renderer2DData;
    data->objectData = new RenderBuffer_Tex1D();
    data->vertexData = new RenderBuffer_GL2_1();
    data->textureSrc = textureSrc;
    data->vertexData->SetType(GL_ARRAY_BUFFER);
    
    data->objectID = 0;
    data->vertexID = 0;
    data->lastW = 0;
    data->lastH = 0;

    
    data->program = new ShaderProgram(
        vertShader_2D,
        fragShader_2D,
        
        vertShader_2D,
        fragShader_2D,

        {
            {VBO_SLOT__POS,    "pos"},
            {VBO_SLOT__COLOR,  "color"},
            {VBO_SLOT__UVS,    "uvs"},
            {VBO_SLOT__OBJECT, "object"}            
        }
    );
    
    
    if (!data->program->GetSuccess()) {
        std::cout << "Uh oh! 2D rendering engine failed!:" << std::endl 
                  << "_______Vertex_______\n\n"
                  << data->program->GetVertexLog() << "\n"
                  << "_______Fragment_____\n\n"
                  << data->program->GetFragmentLog() << "\n" 
                  << "_______Linking______\n\n"
                  << data->program->GetLinkLog() << std::endl;
                  
        exit(0);
    }

}






uint32_t Renderer2D::Add2DVertex() {
    if (!data->deadVertices.empty()) {
        uint32_t i = data->deadVertices.top();
        data->deadVertices.pop();
        return i;
    }

    // already have enough allocated, so return a new ID safely.
    if (data->vertexID < data->vertexData->Size()/sizeof(Renderer::Vertex2D)) {
        return data->vertexID++;
    }
    
    // we have to resize the buffer. We do so in blocks
    uint32_t newSize = data->vertexData->Size()+resize_block_addition_elements*sizeof(float);
    float * dataCopy = new float[newSize/sizeof(float)];
    data->vertexData->GetData(dataCopy, 0, data->vertexData->Size()/sizeof(float));
    data->vertexData->Define(dataCopy, newSize/sizeof(float));
    return data->vertexID++;
}


void Renderer2D::Remove2DVertex(uint32_t datum) {
    data->deadVertices.push(datum);
}






uint32_t Renderer2D::Add2DObject() {
    if (!data->deadObjects.empty()) {
        uint32_t i = data->deadObjects.top();
        data->deadObjects.pop();
        return i;
    }

    // already have enough allocated, so return a new ID safely.
    if (data->objectID < data->objectData->Size()/sizeof(Renderer::Render2DObjectParameters)) {
        return data->objectID++;
    }
    
    // we have to resize the buffer. We do so in blocks
    uint32_t newSize = data->objectData->Size()+resize_block_addition_elements*sizeof(float);
    float * dataCopy = new float[newSize/sizeof(float)];
    data->objectData->GetData(dataCopy, 0, data->objectData->Size()/sizeof(float));
    data->objectData->Define(dataCopy, newSize/sizeof(float));
    return data->objectID++;
}


void Renderer2D::Remove2DObject(uint32_t datum) {
    data->deadObjects.push(datum);
}


void Renderer2D::Set2DObjectParameters(uint32_t object, Renderer::Render2DObjectParameters params) {

    data->objectData->UpdateData((float*)&params, object*16, 16);
}






void Renderer2D::Set2DVertex(uint32_t object, Renderer::Vertex2D params) {
    while(data->userVertexData.size() <= object) {
        data->userVertexData.push_back({});
    }
    
    // user's tex coords are in local texture space and need to be converted to atlas space.

    if (params.useTex >= 0.f) {
        UserVertexData user;
        user.texX = params.texX;
        user.texY = params.texY;
        data->userVertexData[object] = user;

        params.texX = data->textureSrc->MapTexCoordsToRealCoordsX(params.texX, (int)params.useTex); 
        params.texY = data->textureSrc->MapTexCoordsToRealCoordsY(params.texY, (int)params.useTex); 
    }
    data->vertexData->UpdateData((float*)&params, object*10, 10);
}

Renderer::Vertex2D Renderer2D::Get2DVertex(uint32_t vertex) {
    Renderer::Vertex2D vt;
    data->vertexData->GetData((float*)&vt, vertex*10, 10);
    // get user-provided parameters
    vt.texX  = data->userVertexData[vertex].texX;
    vt.texY  = data->userVertexData[vertex].texY;
    return vt;
}

void Renderer2D::Queue2DVertices(const uint32_t * indices, uint32_t count) {
    for(uint32_t i = 0; i < count; ++i) {
        data->queued.push_back(indices[i]);
    }
}

void Renderer2DData::RebaseTextures() {
    Renderer::Vertex2D * copy = new Renderer::Vertex2D[userVertexData.size()];
    vertexData->GetData((float*)copy, 0, userVertexData.size()*10);

    for(uint32_t i = 0; i < userVertexData.size(); ++i) {
        if (copy[i].useTex < 0.f) continue;
        UserVertexData tex = userVertexData[i];
        copy[i].texX = textureSrc->MapTexCoordsToRealCoordsX(tex.texX, copy[i].useTex);
        copy[i].texY = textureSrc->MapTexCoordsToRealCoordsY(tex.texY, copy[i].useTex);
    }
    
    vertexData->UpdateData((float*)copy, 0, userVertexData.size());
    delete[] copy; 
}

uint32_t Renderer2D::Render2DVertices(GLenum drawMode, const Renderer::Render2DStaticParameters & params) {
    if (!data->queued.size()) return 0;
    if (data->lastW != data->textureSrc->GetTextureW() ||
        data->lastH != data->textureSrc->GetTextureH()) {
        data->RebaseTextures();
        data->lastW = data->textureSrc->GetTextureW();
        data->lastH = data->textureSrc->GetTextureH();
    }
    
    glUseProgram(data->program->GetHandle());
    data->program->UpdateUniform("fragTex",   TextureManager::GetActiveTextureSlot()- GL_TEXTURE0);
    data->program->UpdateUniform("objectData", 10);
    
    data->program->UpdateUniform("contextTransform", params.contextTransform);
    data->program->UpdateUniform("contextWidth",  params.contextWidth);
    data->program->UpdateUniform("contextHeight", params.contextHeight);

    float sizeTexData = data->objectData->Size()/(sizeof(float)*4);
    //data->program->UpdateUniform("objectSizeUnits", sizeTexData);

    GLuint currentVbo = data->vertexData->GenerateBufferID();
    glBindBuffer(GL_ARRAY_BUFFER, currentVbo);
    
    glVertexAttribPointer(VBO_SLOT__POS,   2, GL_FLOAT, GL_FALSE, sizeof(Renderer::Vertex2D), (void*)0);
    glVertexAttribPointer(VBO_SLOT__COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Renderer::Vertex2D), (void*)(sizeof(GLfloat) * 2));
    glVertexAttribPointer(VBO_SLOT__UVS,   3, GL_FLOAT, GL_FALSE, sizeof(Renderer::Vertex2D), (void*)(sizeof(GLfloat) * 6));
    glVertexAttribPointer(VBO_SLOT__OBJECT,1, GL_FLOAT, GL_FALSE, sizeof(Renderer::Vertex2D), (void*)(sizeof(GLfloat) * 9));
    


    glActiveTexture(GL_TEXTURE0 + 10);
    glBindTexture(GL_TEXTURE_1D, data->objectData->GenerateBufferID());


    glActiveTexture(TextureManager::GetActiveTextureSlot());
    glBindTexture(GL_TEXTURE_2D, data->textureSrc->GetTexture());



    
    
    glEnableVertexAttribArray(VBO_SLOT__POS);
    glEnableVertexAttribArray(VBO_SLOT__COLOR);
    glEnableVertexAttribArray(VBO_SLOT__UVS);
    glEnableVertexAttribArray(VBO_SLOT__OBJECT);
    
    
    glDrawElements(
        drawMode, 
        data->queued.size(),
        GL_UNSIGNED_INT,
        &data->queued[0]
    );
    
    glDisableVertexAttribArray(VBO_SLOT__POS);
    glDisableVertexAttribArray(VBO_SLOT__COLOR);
    glDisableVertexAttribArray(VBO_SLOT__UVS);
    glDisableVertexAttribArray(VBO_SLOT__OBJECT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    uint32_t size = data->queued.size();
    data->queued.clear();
    return size;
}

void Renderer2D::Clear2DQueue() {
    data->queued.clear();
}



















