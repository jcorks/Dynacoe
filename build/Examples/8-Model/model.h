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


#ifndef H_MODEL_VIEWER_INCLUDED
#define H_MODEL_VIEWER_INCLUDED

#include <Dynacoe/Library.h>
#include <Dynacoe/Components/Mutator.h>
#include <Dynacoe/Model.h>

class ModelViewer : public Dynacoe::Entity {
  public:
    ModelViewer() {
        // First, let's just load in the mesh. Its going to
        // look for a file names "test.obj" and try to load
        // it in as an "obj" file. Although it is
        // best to verify the state of a loaded object,
        // it is not necessary. If loading fails,
        // any handler of the object will notify you if it is invalid.
        target = Dynacoe::Vector();
        id = Dynacoe::Assets::Load("obj", "test.obj", false);
        if (!id.Valid()) {
            Dynacoe::Console::Error() << "Could not load model :(\n";
            return;
        }

        // Now that the Mesh itself is loaded,
        // We need to attach the mesh to an Aspect,
        // since Aspects are the only things that can be drawn.
        Dynacoe::Model * model = &Dynacoe::Assets::Get<Dynacoe::Model>(id); 
        Entity::ID modelEntity = model->Create();
        Attach(modelEntity);



        // Lets back the camera up too so we can see the model.
        target = {0, 0, 3};

        light.FormLight(Dynacoe::RenderLight::Light::Point);
        light.state.position = {-.5, -.5, -.5};

        light2.FormLight(Dynacoe::RenderLight::Light::Directional);
        light2.state.position = {5, .5, 5};

        node.Rotation() = {270, 0, 0};


    }

    void OnStep() {
        // controls the camera with input
        controlCamera();

        
        /*for(uint32_t i = 0; i < Model->GetMesh().NumChildren(); ++i) {
            Model->GetMesh().GetChild(i).SetRotation(
                Model->GetMesh().GetChild(i).GetRotation() + Dynacoe::Vector(.01, .3, .001)
            );
        }*/
        
        node.Rotation() += Dynacoe::Vector(0, .01, 0);
        
    }



  private:


    Dynacoe::Vector target;
    Dynacoe::Vector camPos;
    Dynacoe::AssetID id;
    Dynacoe::RenderMesh mesh;

    Dynacoe::RenderLight light;
    Dynacoe::RenderLight light2;
    Dynacoe::Material m;

    // Utility function that lets you look around with WASD, QE.
    void controlCamera() {

        Dynacoe::Camera * cam = &Dynacoe::Graphics::GetCamera3D();

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_w)) {
            target.y += .4f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_s)) {
            target.y -= .4f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_a)) {
            target.x += .4f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_d)) {
            target.x -= .4f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_q)) {
            target.z += .4f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_e)) {
            target.z -= .4f;
        }



        camPos.x = Dynacoe::Mutator::StepTowards(camPos.x, target.x, .1);
        camPos.y = Dynacoe::Mutator::StepTowards(camPos.y, target.y, .1);
        camPos.z = Dynacoe::Mutator::StepTowards(camPos.z, target.z, .1);

        cam->node.Position() = camPos;

    }
  

}; 


#endif
