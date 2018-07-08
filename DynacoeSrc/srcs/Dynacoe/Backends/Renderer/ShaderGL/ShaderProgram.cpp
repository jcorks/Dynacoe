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
#include <Dynacoe/Backends/Renderer/ShaderGL/ShaderProgram.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/GLVersionQuery.h>
#include <cstring>
#include <cassert>
#include <Dynacoe/Util/TransformMatrix.h>

using namespace Dynacoe;


struct Dynacoe::ShaderProgramData {
    GLuint program;
    std::string vertexLog;
    std::string fragmentLog;
    std::string linkLog;
    
    bool success;
    
};


ShaderProgram::ShaderProgram(
    const std::string & vertShader,
    const std::string & fragShader,

    const std::string & vertShader_21,
    const std::string & fragShader_21,

    const std::vector<std::pair<GLuint, std::string>> & bindAttributeLocations

) {
    
    data = new ShaderProgramData();
    
    
    
    data->program = glCreateProgram();


    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
    int uniformIndex;

    std::string header;
    std::string fragSrc, vertSrc;
    if (GLVersionQuery(GL_Version3_1)) {
        header = "#version 140\n";
        fragSrc = header + std::string(fragShader);
        vertSrc = header + std::string(vertShader);
    } else if (GLVersionQuery(GL_Version3_0 | GL_UniformBufferObject)) {
        header = "#version 130\n"
                 "#extension GL_ARB_uniform_buffer_object: require\n";
        fragSrc = header + std::string(fragShader);
        vertSrc = header + std::string(vertShader);
    } else {
        header = "#version 120\n";
        fragSrc = header + std::string(fragShader_21);
        vertSrc = header + std::string(vertShader_21);
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
    data->success = true;
    if (!success) {
        glGetShaderiv(fragShaderID, GL_INFO_LOG_LENGTH, &logLength);
        std::string log;  
        log.resize(logLength);

        glGetShaderInfoLog(fragShaderID, logLength, &logLength, &log[0]);
        data->fragmentLog = log;
        data->success &= success;
    }


    glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderiv(vertShaderID, GL_INFO_LOG_LENGTH, &logLength);
        std::string log;
        log.resize(logLength);

        glGetShaderInfoLog(vertShaderID, logLength, &logLength, &log[0]);
        data->vertexLog = log;
        data->success &= success;
    }

    if (!data->success) return; 



    glAttachShader(data->program, fragShaderID);
    glAttachShader(data->program, vertShaderID);

    

    for(uint32_t i = 0; i < bindAttributeLocations.size(); ++i) {
        glBindAttribLocation(
            data->program,
            bindAttributeLocations[i].first,
            &(bindAttributeLocations[i].second)[0]
        );
    }




    glLinkProgram(data->program);

    glGetProgramiv(data->program, GL_LINK_STATUS, &success);
    glGetProgramiv(data->program, GL_INFO_LOG_LENGTH, &logLength);
    if (!success) {
        std::string l;
        l.resize(logLength);

        glGetProgramInfoLog(data->program, logLength, &logLength, &l[0]);
        
        data->linkLog = l;
        data->success &= success;
    }


}

const std::string & ShaderProgram::GetVertexLog() {
    return data->vertexLog;
}

const std::string & ShaderProgram::GetFragmentLog() {
    return data->fragmentLog;
}

const std::string & ShaderProgram::GetLinkLog() {
    return data->linkLog;
}

bool ShaderProgram::GetSuccess() {
    return data->success;
}

void ShaderProgram::UpdateUniform(const std::string & name, int value) {
    GLint locID   = glGetUniformLocation(data->program, name.c_str());
    glUniform1i(locID, value);
    assert(locID >= 0);
}

void ShaderProgram::UpdateUniform(const std::string & name, float value) {
    GLint locID   = glGetUniformLocation(data->program, name.c_str());
    glUniform1f(locID, value);
    assert(locID >= 0);
}

void ShaderProgram::UpdateUniform(const std::string & name, float * value) {
    GLint locID   = glGetUniformLocation(data->program, name.c_str());
    glUniformMatrix4fv(locID, 1, true, value);
    assert(locID >= 0);
}



GLuint ShaderProgram::GetHandle() {
    return data->program;
}


#endif