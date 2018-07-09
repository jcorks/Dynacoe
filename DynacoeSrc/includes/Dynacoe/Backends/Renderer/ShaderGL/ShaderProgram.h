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
#ifndef H_DC_BACKENDS_GL_SHADERPROGRAM_INCLUDED
#define H_DC_BACKENDS_GL_SHADERPROGRAM_INCLUDED


#include <string>
#include <vector>
#include <cstdint>


namespace Dynacoe {
class ShaderProgramData;
class ShaderProgram {
  public:
    ShaderProgram(
        const std::string & vertexSource_GL3,
        const std::string & fragmentSource_GL3,

        const std::string & vertexSource_GL2_1,
        const std::string & fragmentSource_GL2_1,
        
        const std::vector<std::pair<uint32_t, std::string>> & bindAttributeLocations
    );
    ~ShaderProgram();
    
    
    
    const std::string & GetVertexLog();
    const std::string & GetFragmentLog();
    const std::string & GetLinkLog();
    
    void UpdateUniform(const std::string &, int);
    void UpdateUniform(const std::string &, float);
    void UpdateUniform(const std::string &, float *);

    uint32_t GetHandle();
    
    bool GetSuccess();
      
  private: 
    ShaderProgramData * data;
};
}

#endif
#endif