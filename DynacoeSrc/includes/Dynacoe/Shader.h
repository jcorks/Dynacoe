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

#ifndef H_DC_SHADER_INCLUDED
#define H_DC_SHADER_INCLUDED


#include <Dynacoe/Backends/Renderer/Renderer.h>

namespace Dynacoe {
    
    
/// \brief Allows you to specify rendering programs usually on your GPU.
///
/// Shaders are programs compiled and run during runtime to process graphics data. On 
/// backends that support them, ithey will usually run on dedicated hardware. 
/// Since the actual source language of the Shader depends on the Renderer backend, 
/// The rules and behavior of how the shader source will vary greatly.
///
/// For the stock OpenGL backend, the shader language will be a variant of GLSL.
/// See [https://jcorks.github.io/Dynacoe/shaders-glsl.html](https://jcorks.github.io/Dynacoe/shaders-glsl.html) for more info.
class Shader{
  public:
    Shader();
    ~Shader();
    
    /// \brief The stages that sources can be attached to.
    ///
    enum class Stage {
        VertexProcessor,   ///< First stage of the shader. Processes each vertex
        FragmentProcessor,
        NotAStage
    };
   
    /// \brief The stauts of the program.
    ///
    enum class Status {
        Invalid,     ///< Compile() was called, but the compilation failed. AddStage() and Copmile() may be called again to fix the bad stage(s).
        Success,     ///< Compile() was successful and the Shader may be used in rendering. See Material.
        NotCompiled  ///< Compile has not yet been called.
    };
    
    /// \brief Adds a stage to the shader program.
    /// If the stage already exists and the program has not been compiled succesfully, 
    /// this call replaces the old source with this source.
    ///
    void AddStage(Stage, const std::string &);
    
    /// \brief Finalizes the programs stages. After this call,
    /// GetStatus() will either return Success, meaning the program is now usable,
    /// or Invalid, meaning compilation failed. In the case of failure,
    /// GetLog() may contain more information as to why the compilation failed.
    ///
    void Compile();
    
    /// \brief Returns any additional logging information from the shader program.
    /// If the status Invalid, typically the log will contain information on why.
    ///
    std::string GetLog();
    
    /// \brief Returns the current program status.
    ///
    Status GetStatus();
    
    /// \brief Returns a string containing the shader language recognized by the Shaders. This 
    /// is dependent on the parameters of the Renderer used by Graphics.
    ///
    std::string GetShaderLanguage();

    /// \brief Returns a Renderer::ProgramID refering to this shader.
    /// 
    ProgramID GetID(){return id;}
    
  private:
    ProgramID id;
    Status status;
    struct StageData {
        std::string src;
        Stage stage;
    };
    std::string log;
    StageData stages[(int)Stage::NotAStage];
    
};
    
    
}


#endif