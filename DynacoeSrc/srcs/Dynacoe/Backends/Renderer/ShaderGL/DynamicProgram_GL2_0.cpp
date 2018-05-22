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
#include <Dynacoe/Backends/Renderer/ShaderGL/DynamicProgram_GL2_0.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/GLVersionQuery.h>
#include <cstring>
#include <cassert>
#include <Dynacoe/Util/TransformMatrix.h>

using namespace Dynacoe;


const int min_supported_texture_size = 1024;




class Dynacoe::TransformManager {

  public:

      struct float16 { float data[16]; };

    // queues a new transform to upload
    int PrepareNew(float *);

    // retrieves the transform with the given ID
    const float16 & Get(int) const;

    void Clear();


  private:
    std::vector<float16> data;


};



/* TODO



  for implementing dynamic transforms:


  TRY 1 : buffer change instructions

    - change the vertex size fo 17*4 bytes
    - insert transform changes as degenerate primitives
        - first 16 fields are matrix values
        - the newTransform field (last float field) to what matrix this
            - 1.f == transform
            - 2.f == projection
    - BUMMERS
        - to guarantee degenerate primitives,
          we need to insert 6 fake vertices
        - HOW DO I SAVE THE RESULTS


  TRY 2 : hueg buffer object
    -- each draw, a uniform buffer (transfroms) is reset
    - Each new transforms storage, append to the transforms buffer and tag its index
    - pass index as last member of vertex

    - BUMMERS
        - extrememly limited number of transforms per draw: tops out at around 256 4x4 matrices...


  TRY 3: Texture Buffer!
    - Same as TRY 2, but instead of uniform buffer, just store it in a 1D float
      texture array packed tightly. 4 pixels = 1 matrix.
    - BUMMERS:
        - minimum 1D tex size appears to be around 8192, but we can fill multiple textures,
          and swap on what we sample. each vertex would then need a TEX key and a TEX index
          to get the data
        NOTE: we can use this model for an arbitrary large buffer interface!

        - *Texel fetch accuracy has shown some issues on slower intel machines
        - use texelFetch for data lookup in shader



  TRY 4: Transform per vertex
    - Similar to 1, but more dumb: each vertex simply has a transform with it.
    - simple design, works within gl parameters
    - no limit

    - BUMMERS:
        - a lot more data per vertex. Though, i think in most situations this okay.
        - Immediate;y drawn objects tend to be smaller in volume anyway

*/



static const char * vertShader_Dynamic =

"in  vec3  VTex;\n"
"in  vec4  passColor;\n"
"in  vec2  texCoords;\n"
"in  float useTex;\n"

"in  vec4 transform1;\n"
"in  vec4 transform2;\n"
"in  vec4 transform3;\n"
"in  vec4 transform4;\n"


// projection * view
//"uniform mat4 projview;"


"out float fragUseTex;\n"
"out vec4  fragColor;\n"
"out vec2  fragTexCoord;\n"


"void main(void) {\n"

"   mat4 transform;\n"

"   transform[0] = transform1;\n"
"   transform[1] = transform2;\n"
"   transform[2] = transform3;\n"
"   transform[3] = transform4;\n"


"   gl_Position  = (vec4(VTex, 1.f) * transform);\n"      // for if you want gl to handle transformation with matrices

"   fragColor    = passColor;\n"
"   fragUseTex   = useTex;\n"
"   fragTexCoord = texCoords;\n"
"   \n"
"}\n";


static const char * vertShader_Dynamic_21 =

"attribute  vec3  VTex;\n"
"attribute  vec4  passColor;\n"
"attribute  vec2  texCoords;\n"
"attribute  float useTex;\n"

"attribute  vec4 transform1;\n"
"attribute  vec4 transform2;\n"
"attribute  vec4 transform3;\n"
"attribute  vec4 transform4;\n"

// projection * view
//"uniform mat4 projview;"

"varying float fragUseTex;\n"
"varying vec4  fragColor;\n"
"varying vec2  fragTexCoord;\n"


"void main(void) {\n"

"   mat4 transform;\n"

"   transform[0] = transform1;\n"
"   transform[1] = transform2;\n"
"   transform[2] = transform3;\n"
"   transform[3] = transform4;\n"


"   gl_Position  = vec4(VTex, 1.f) * transform);\n"      // for if you want gl to handle transformation with matrices

"   fragColor    = passColor;\n"
"   fragUseTex   = useTex;\n"
"   fragTexCoord = texCoords;\n"
"   \n"
"}\n";




static const char * fragShader_Dynamic =

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


static const char * fragShader_Dynamic_21 =

"varying  float fragUseTex;\n"
"varying  vec4  fragColor;\n"
"varying  vec2  fragTexCoord;\n"



"uniform sampler2D fragTex;\n"
"void main(void) {\n"
"   vec4 outColor;\n"
"   if (fragUseTex > -.5) {\n"

"       vec4 temp = texture2D(fragTex, fragTexCoord);\n"
"           outColor.r = temp.r * fragColor.r;\n"
"           outColor.g = temp.g * fragColor.g;\n"
"           outColor.b = temp.b * fragColor.b;\n"
"           outColor.a = temp.a * fragColor.a;\n"

//"       outColor = vec4(1.f, 0.f, 1.f, 1.f);\n"
"   } else {\n"
"       outColor = fragColor;\n"
"   }\n"
"   gl_FragColor = outColor;\n"
"}\n";




DynamicProgram_GL2_0::DynamicProgram_GL2_0() {
    PrepareShader();


    // bind the draw buffer
    glBindBuffer(GL_ARRAY_BUFFER, totalVbo);

    // upon every resolve request, reform buffer.
    // Might be better to just allocate on every resize and glMapSubBuffer
    glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(DynamicVertexEx) * 2,
		nullptr,
		GL_DYNAMIC_DRAW
	);


    // now that it's uploaded, assign each index the piece of the data that it needs
    // since the bound buffer target is nonzero, the last paramter is interpreted as a byte offset
    // in the buffers' data


    glVertexAttribPointer(vboVertices ,      3, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *) 0);
    glVertexAttribPointer(vboColors   ,      4, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(vboTexCoords,      2, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(GLfloat) * 7));
    glVertexAttribPointer(vboUseTex   ,      1, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(GLfloat) * 9));
    //glVertexAttribPointer(vboTransformIndex, 1, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(Renderer::DynamicVertex)));
    //glVertexAttribPointer(vboTransformLayer, 1, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(Renderer::DynamicVertex) + sizeof(GLfloat)));
    glVertexAttribPointer(vboTransform1,     4, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(Renderer::DynamicVertex)+0));
    glVertexAttribPointer(vboTransform2,     4, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(Renderer::DynamicVertex)+sizeof(GLfloat)*4));
    glVertexAttribPointer(vboTransform3,     4, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(Renderer::DynamicVertex)+sizeof(GLfloat)*8));
    glVertexAttribPointer(vboTransform4,     4, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *)(sizeof(Renderer::DynamicVertex)+sizeof(GLfloat)*12));

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    texture = nullptr;
    transforms = new TransformManager();
    vcountActive = 0;

}


void DynamicProgram_GL2_0::Queue(
    const Renderer::DynamicVertex * v, uint32_t num, DynamicTransformID transform
) {
    if (num + vcountActive >=  vertices.size()) vertices.resize((num+vcountActive)*1.1);

    DynamicVertexEx * vertex = &vertices[vcountActive];



    for(uint32_t i = 0; i < num; ++i, ++vcountActive) {
        memcpy(&vertex->vertex, v+i, sizeof(Renderer::DynamicVertex));
        if (v->useTex > 0) {
            vertex->vertex.texX = texture->MapTexCoordsToRealCoordsX(vertex->vertex.texX, (int) vertex->vertex.useTex);
            vertex->vertex.texY = texture->MapTexCoordsToRealCoordsY(vertex->vertex.texY, (int) vertex->vertex.useTex);
            //std::cout << i << "@ px " << vertex->vertex.texX * texture->GetTextureW()
            //          << ", " << vertex->vertex.texY * texture->GetTextureH() << std::endl;
        }
        *((TransformManager::float16*)vertex->transform) = transforms->Get(transform);
        vertex++;
    }

}




DynamicTransformID DynamicProgram_GL2_0::NewTransform(float * matr) {
    int out = transforms->PrepareNew(matr);


    return out;
}








uint32_t DynamicProgram_GL2_0::Render(GLenum drawMode) {
    assert(glGetError() == GL_NO_ERROR);
    if (!vcountActive) return 0;

    uint32_t vcountActiveCopy = vcountActive;






    // bind the draw buffer
    glBindBuffer(GL_ARRAY_BUFFER, totalVbo);
    /*
    for(int i = 0; i < vcountActive; ++i) {
        std::cout << "x:" << vertices[i].vertex.x
                  << " y:" << vertices[i].vertex.y
                  << " z:" << vertices[i].vertex.z
                  << " r:" << vertices[i].vertex.r
                  << " g:" << vertices[i].vertex.g
                  << " b:" << vertices[i].vertex.b
                  << " a:" << vertices[i].vertex.a
                  << " tLayer: " << (float)vertices[i].transformLayer
                  << " tIndex: " << vertices[i].transformIndex  << std::endl;
    }
    */

    // upon every resolve request, reform buffer. Might be better to just allocate on every resize and glMapSubBuffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(DynamicVertexEx) * vcountActive, &vertices[0], GL_DYNAMIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(DynamicVertexEx) * 6, &testV[0], GL_DYNAMIC_DRAW);


    glActiveTexture(TextureManager::GetActiveTextureSlot());
    glBindTexture(GL_TEXTURE_2D, texture->GetTexture());

    // Dummy attribute required for rendering on some systems (ATI!)
    // keep it interleaved though to not affect performance too much
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DynamicVertexEx), (void *) 0);

    glEnableVertexAttribArray(0); // required for some systems
    glEnableVertexAttribArray(vboVertices);
    glEnableVertexAttribArray(vboTexCoords);
    glEnableVertexAttribArray(vboColors);
    glEnableVertexAttribArray(vboUseTex);

    glEnableVertexAttribArray(vboTransform1);
    glEnableVertexAttribArray(vboTransform2);
    glEnableVertexAttribArray(vboTransform3);
    glEnableVertexAttribArray(vboTransform4);


    glUseProgram(program);

    assert(glGetError() == GL_NO_ERROR);


    glDrawArrays(drawMode, 0, vcountActive);
    //glDrawArrays(drawMode, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    assert(glGetError() == GL_NO_ERROR);


    glDisableVertexAttribArray(vboVertices);
    glDisableVertexAttribArray(vboTexCoords);
    glDisableVertexAttribArray(vboColors);
    glDisableVertexAttribArray(vboUseTex);

    glDisableVertexAttribArray(vboTransform1);
    glDisableVertexAttribArray(vboTransform2);
    glDisableVertexAttribArray(vboTransform3);
    glDisableVertexAttribArray(vboTransform4);
    glDisableVertexAttribArray(0); // required for some systems



    //Clear();

    glActiveTexture(TextureManager::GetActiveTextureSlot());
    glBindTexture(GL_TEXTURE_2D, 0);


    return vcountActiveCopy;

}




void DynamicProgram_GL2_0::Clear() {
    vcountActive = 0;
    transforms->Clear();
}








void DynamicProgram_GL2_0::PrepareShader() {


    program = glCreateProgram();


    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
    int uniformIndex;

    std::string header;
    std::string fragSrc, vertSrc;
    if (GLVersionQuery(GL_Version3_1)) {
        header = "#version 140\n";
        fragSrc = header + std::string(fragShader_Dynamic);
        vertSrc = header + std::string(vertShader_Dynamic);
    } else if (GLVersionQuery(GL_Version3_0 | GL_UniformBufferObject)) {
        header = "#version 130\n"
                 "#extension GL_ARB_uniform_buffer_object: require\n";
        fragSrc = header + std::string(fragShader_Dynamic);
        vertSrc = header + std::string(vertShader_Dynamic);
    } else {
        header = "#version 120\n";
        fragSrc = header + std::string(fragShader_Dynamic_21);
        vertSrc = header + std::string(vertShader_Dynamic_21);
    }


    const char * fPtr = fragSrc.c_str();
    const char * vPtr = vertSrc.c_str();

    glShaderSource(fragShaderID, 1, &fPtr, NULL);
    glShaderSource(vertShaderID, 1, &vPtr, NULL);

    glCompileShader(fragShaderID);
    glCompileShader(vertShaderID);

    int success;
    glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &success);
    int logLength;
    if (!success) {
        glGetShaderiv(fragShaderID, GL_INFO_LOG_LENGTH, &logLength);
        std::string log;
        log.resize(logLength);

        glGetShaderInfoLog(fragShaderID, logLength, &logLength, &log[0]);
        std::cout << log << std::endl;
        assert(0);
    }


    glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderiv(fragShaderID, GL_INFO_LOG_LENGTH, &logLength);
        std::string log;
        log.resize(logLength);

        glGetShaderInfoLog(fragShaderID, logLength, &logLength, &log[0]);
        std::cout << log << std::endl;
        assert(0);
    }




    glAttachShader(program, fragShaderID);
    glAttachShader(program, vertShaderID);

    glGenBuffers(1, &totalVbo);


    vboVertices       = 3;
    vboTexCoords      = 4;
    vboColors         = 5;
    vboUseTex         = 6;

    vboTransform1     = 7;
    vboTransform2     = 8;
    vboTransform3     = 9;
    vboTransform4     = 10;

    // bind locations
    glBindAttribLocation(program, vboVertices,       "VTex");
    glBindAttribLocation(program, vboTexCoords,      "texCoords");
    glBindAttribLocation(program, vboColors,         "passColor");
    glBindAttribLocation(program, vboUseTex,         "useTex");

    glBindAttribLocation(program, vboTransform1,      "transform1");
    glBindAttribLocation(program, vboTransform2,      "transform2");
    glBindAttribLocation(program, vboTransform3,      "transform3");
    glBindAttribLocation(program, vboTransform4,      "transform4");




    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (!success) {
        std::string l;
        l.resize(logLength);

        glGetProgramInfoLog(program, logLength, &logLength, &l[0]);

        std::cout << std::string(l) << std::endl;;

        assert(0);
    }





    // send sampler id to frag shader


    GLint locID   = glGetUniformLocation(program, "fragTex");
    //GLint transID = glGetUniformLocation(program, "transformData");
    //std::cout << std::endl << "TransformData location: " << transID << std::endl;

    // bind the modelview / projection matrices






    glUseProgram(program);



    //cout << "uniform location: " << locID << endl;
    glUniform1i(locID,   TextureManager::GetActiveTextureSlot()   - GL_TEXTURE0);
}



int TransformManager::PrepareNew(float * d) {
    data.push_back(*(float16*)d);
    return data.size()-1;
}

const TransformManager::float16 & TransformManager::Get(int i) const {
    return data[i];
}

void TransformManager::Clear() {
    data.clear();
}















#endif
