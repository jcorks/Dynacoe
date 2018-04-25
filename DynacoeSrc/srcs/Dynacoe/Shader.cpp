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

#include <Dynacoe/Shader.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>

using namespace Dynacoe;

Shader::Shader() {
    status = Status::NotCompiled;
}

Shader::~Shader() {
    
}

void Shader::AddStage(Stage stage, const std::string & str) {
    if (status == Status::Success) return;
    stages[(int)stage].src = str;
}


void Shader::Compile() {
    if (status == Status::Success) return;
    Renderer * r = Graphics::GetRenderer();
    if (!r) return;
    // create a program if it DNE
    if (id == ProgramID()) {
        id = r->ProgramAdd(
            stages[0].src,
            stages[1].src,
            log
        );
    }
    if (id == ProgramID()) {
        status = Status::Invalid;
    } else {
        status = Status::Success;
    }
}

std::string Shader::GetLog() {
    return log;
}

Shader::Status Shader::GetStatus() {
    return status;
}

std::string Shader::GetShaderLanguage() {
    return Graphics::GetRenderer()->ProgramGetLanguage();
}
