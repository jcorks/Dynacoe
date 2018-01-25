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
#ifndef H_DC_BACKENDS_DYNAMIC_PROGRAM_GL2_0
#define H_DC_BACKENDS_DYNAMIC_PROGRAM_GL2_0

#include <Dynacoe/Backends/Renderer/ShaderGL/DynamicProgram.h>

namespace Dynacoe {



// Dynamic render manager: GL 3.1 variant
class DynamicProgram_GL2_0 : public DynamicProgram {
  public:
    DynamicProgram_GL2_0();


    // from    
    void Queue(const Renderer::DynamicVertex * v, uint32_t num, DynamicTransformID);

    // Adds a new transform to use. Expected to be an array of 16 floats
    DynamicTransformID NewTransform(float * matr);

    // render errthing
    uint32_t Render(GLenum polygon);

    // Undo errthng
    void Clear();


    void AttachTextureManager(TextureManager * t) {texture = t;}


  private:
    void PrepareShader();
    void GarbageCollect();




    std::vector<int> transformBuffers;
    std::vector<DynamicVertexEx> vertices;
    uint32_t vcountActive;

    GLuint totalVbo;
    GLint vboVertices;
    GLint vboTexCoords;
    GLint vboColors;
    GLint vboUseTex;

    
    GLint vboTransform1;
    GLint vboTransform2;
    GLint vboTransform3;
    GLint vboTransform4;
    
    GLint program; // rendererprogram

    TextureManager   * texture;
    TransformManager * transforms;

    
};
}

#endif
#endif
