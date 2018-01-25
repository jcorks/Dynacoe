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
#include <Dynacoe/Backends/Renderer/ShaderGL/StaticProgram_GL2_1.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/GLVersionQuery.h>
    
using namespace Dynacoe;

StaticProgram * Dynacoe::CreateStaticProgram() {

    if (GLVersionQuery(GL_Version3_0 | GL_UniformBufferObject) ||
        GLVersionQuery(GL_Version3_1)) {
        return new StaticProgram_GL3_1();
    } 
    
    if (GLVersionQuery(GL_Version2_1)) {
        return new StaticProgram_GL2_1();
    }
    
    
    printf("StaticProgram : UNKNOWN???\n");
    return nullptr;
    
}



    

    
    
#endif
