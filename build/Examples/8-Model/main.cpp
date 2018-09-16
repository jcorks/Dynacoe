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


#include <Dynacoe/Library.h>


/*  A Basic example that shows how to
    load and draw a model.

    To keep this example simple, no lighting, 
    texturing, or material work is done. Only a default material is applied

    Johnathan Corkery, 2015
 */


using namespace Dynacoe;



// Loads a pre-assigned model and draws it onscreen
// at its position.
class ModelViewer : public Entity {
  public:
    ModelViewer() {
        // First, let's just load in the mesh. Its going to
        // look for a file names "test.obj" and try to load
        // it in as an "obj" file. Although it is
        // best to verify the state of a loaded object,
        // it is not necessary. If loading fails,
        // any handler of the object will notify you if it is invalid.
        target = Vector();
        id = Assets::Load("obj", "test.obj", false);
        if (!id.Valid()) {
            Console::Error() << "Could not load model :(\n";
            return;
        }

        // Now that the Mesh itself is loaded,
        // We need to attach the mesh to this entity.
        // Loaded meshes are a bit differne that loaded images, in that a
        // mesh file isn't restricted to a single mesh and could hold 
        // a multitude of 3d objects with unique material definitions.
        // So, when we work with models, we actually use them to create 
        // pre-assembled entities that, when drawn, produce the model visual onscreen.
        Model * model = &Assets::Get<Model>(id); 
        Entity::ID modelEntity = model->Create();
        Attach(modelEntity);



        // Lets back the camera up too so we can see the model.
        target = {0, 0, 3};


    }

    void OnStep() {
        // controls the camera with input
        controlCamera();
        Node().Rotation() += Vector(0, .01, 0);
        
    }



  private:


    Vector target;
    Vector camPos;
    AssetID id;

    RenderLight * light;
    RenderLight * light2;
    Material m;

    // Utility function that lets you look around with WASD, QE.
    void controlCamera() {

        Camera * cam = &Graphics::GetCamera3D();

        if (Input::GetState(Keyboard::Key_w)) {
            target.y += .4f;
        }

        if (Input::GetState(Keyboard::Key_s)) {
            target.y -= .4f;
        }

        if (Input::GetState(Keyboard::Key_a)) {
            target.x += .4f;
        }

        if (Input::GetState(Keyboard::Key_d)) {
            target.x -= .4f;
        }

        if (Input::GetState(Keyboard::Key_q)) {
            target.z += .4f;
        }

        if (Input::GetState(Keyboard::Key_e)) {
            target.z -= .4f;
        }


        // This uses the easing technique used in the Sound example for the 
        // circle visual scaling. Here, it is applied to the position in XYZ 
        camPos.x = Mutator::StepTowards(camPos.x, target.x, .1);
        camPos.y = Mutator::StepTowards(camPos.y, target.y, .1);
        camPos.z = Mutator::StepTowards(camPos.z, target.z, .1);

        cam->Node().Position() = camPos;

    }
  

}; 



// Basic driver once again!
int main() {
    Engine::Startup();    
    ViewManager::NewMain("Basic Model");


    Engine::Root() = Entity::Create<ModelViewer>();
    Engine::Run(); 
    
    return 0;
}

