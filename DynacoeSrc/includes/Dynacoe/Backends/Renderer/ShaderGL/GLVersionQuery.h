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

#ifdef DC_SUBSYSTEM_X11        
    #ifndef GLEW_STATIC
    #define GLEW_STATIC
    #endif
    #include <GL/glew.h>
    #include <GL/glx.h>
#endif


#ifdef DC_SUBSYSTEM_WIN32
    #ifndef GLEW_STATIC
    #define GLEW_STATIC
    #endif
    #ifdef _MBCS 
        #include <GL/glew.h>    
        #include <GL/wglew.h>
    #else
        #include <glew.h>
        #include <wglew.h>
    #endif

    #include <GL/gl.h>
    #include <windows.h>

#endif
    
#ifndef GLVERSIONQUERY_H_INCLUDED
#define GLVERSIONQUERY_H_INCLUDED




namespace Dynacoe {

enum {
    GL_Version3_0           =0b10000,
    GL_Version3_1           =0b01000,
    GL_Version2_1           =0b00100,
    GL_UniformBufferObject  =0b00010,
    GL_FramebufferObject    =0b00001
};

// initializes the versioning
bool GLVersionInit();

bool GLVersionQuery(int mask);


    
}



#endif
#endif
