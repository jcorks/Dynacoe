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


#ifndef SHADER_EXAMPLE_H_INCLUDED
#define SHADER_EXAMPLE_H_INCLUDED

#include <Dynacoe/Library.h>
#include <Dynacoe/Components/Mutator.h>

class ShaderExample : public Dynacoe::Entity {
  public:

    // The aspect member holds the image we are going to draw.
    Dynacoe::RenderMesh renderer;
    Dynacoe::Shader     program;
    Dynacoe::Material   mat;

    Dynacoe::Vector target;

    ShaderExample() {
        SetName("ShaderExample");

        AddComponent(&renderer);
        


        // We're going to just draw some rectangles
        renderer.AddMesh(Dynacoe::Mesh::Basic_Cube());        

        // We are going to read the vertex and fragment shaders into 
        // InputBuffers. (Open them in text files to see notes on their implementation)
        Dynacoe::InputBuffer vertexSrc;   vertexSrc.Open  ("vertex.glsl");
        Dynacoe::InputBuffer fragmentSrc; fragmentSrc.Open("fragment.glsl");

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
        mat.SetProgram(program.GetID());        
        mat.AddTexture(Dynacoe::Material::TextureSlot::Color,  Dynacoe::Assets::Load("png", "pic.png"));    
        mat.AddTexture(Dynacoe::Material::TextureSlot::Normal, Dynacoe::Assets::Load("png", "normal.png"));        
        //mat.SetFramebufferSource(Dynacoe::Graphics::GetActiveCamera());
        renderer.Material() = mat;


        
        
        // Lets put our camera a good distance so we can see the rectangles.
        target = {0, .4, 1.3f};

        Dynacoe::RenderLight * light = Dynacoe::Graphics::GetCamera3D().BindComponent<Dynacoe::RenderLight>();
        light->FormLight(Dynacoe::RenderLight::Light::Directional);
        light->state.position = {-.2, -1, -.4};
        light->state.intensity = 2;
        light->state.color = {.8f, .9f, 1.f};
    }



    void OnDraw() {
        renderer.node.Rotation().z += .1;
        renderer.node.Rotation().x += .1;
        renderer.node.Rotation().y += .1;
    }


    void Destruct() {
        // nothing to destroy!
    }

    
    

    void OnStep() {
        // This function controls the camera movement logic. It
        // is not the focus, but it is available below.
        // The camera controls are 'W'A'S'D'Q'E' for XYZ camera movement    
        controlCamera();
    }    

    void controlCamera() {

        Dynacoe::Camera & cam = Dynacoe::Graphics::GetCamera3D();

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_w)) {
            target.y -= .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_s)) {
            target.y += .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_a)) {
            target.x += .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_d)) {
            target.x -= .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_q)) {
            target.z -= .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_e)) {
            target.z += .1f;
        }

        // ease towards position
        cam.node.Rotation().x = Dynacoe::Mutator::StepTowards(cam.node.GetRotation().x, target.x, .1);
        cam.node.Rotation().y = Dynacoe::Mutator::StepTowards(cam.node.GetRotation().y, target.y, .1);
        cam.node.Rotation().z = Dynacoe::Mutator::StepTowards(cam.node.GetRotation().z, target.z, .1);

    }





};




#endif // IMAGE_EXAMPLE_H_INCLUDED
