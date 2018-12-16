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

#ifndef H_SHADERCUBE_INCLUDED
#define H_SHADERCUBE_INCLUDED

#include <Dynacoe/Library.h>


// Similar to the previous shader example.
// A spinnign cube with a funky shader
class ShaderCube : public Dynacoe::Entity {
  public:


    ShaderCube() {
        SetName("ShaderCube");
        renderer = AddComponent<Dynacoe::RenderMesh>();
        


        // We're going to just draw a nice cube
        renderer->AddMesh(Dynacoe::Mesh::Basic_Cube());        

        // We are going to read the vertex and fragment shaders into 
        // InputBuffers. (Open them in text files to see notes on their implementation)
        Dynacoe::InputBuffer vertexSrc;
        Dynacoe::InputBuffer fragmentSrc; 

        if (Dynacoe::Graphics::GetRenderer()->ProgramGetLanguage() == "GLSL ES 1.0") {
            vertexSrc.Open  ("shaders/cube_es_vertex.glsl");
            fragmentSrc.Open("shaders/cube_es_fragment.glsl");
        
        } else { // desktop glsl
            vertexSrc.Open  ("shaders/cube_vertex.glsl");
            fragmentSrc.Open("shaders/cube_fragment.glsl");
        }


        // How we assemble the shader. The shader consists of 2 stages:
        // the vertex processing stage and the fragment stage
        program.AddStage(
            Dynacoe::Shader::Stage::VertexProcessor,
            vertexSrc.ReadString(vertexSrc.Size())
        );

        program.AddStage(
            Dynacoe::Shader::Stage::FragmentProcessor,
            fragmentSrc.ReadString(fragmentSrc.Size())
        );
        
        
        // After passing them in, we need to compile the program.
        program.Compile();
        
        // Also should check for success or else we'll get a bad program.
        if (program.GetStatus() != Dynacoe::Shader::Status::Success) {
            Dynacoe::Console::Error() << program.GetLog().c_str();
            return;
        }
        


        // Seems alright, lets attach it to a material 
        // and attach the material to the program.
        Dynacoe::Material & mat = renderer->Material();
        mat.SetProgram(program.GetID());        


        Node().Scale() = {3, 3, 3};

    }


    void OnStep() {
        // Rotate it all around
        Node().Rotation().y += .1;
        Node().Rotation().z += .1;
        Node().Rotation().x += .1;

    }

  private:
    Dynacoe::RenderMesh * renderer;
    Dynacoe::Shader       program;

};



#endif
