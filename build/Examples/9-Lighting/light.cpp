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


#include "light.h"
#include <Dynacoe/Components/Mutator.h>

using namespace Dynacoe;


const Color dark    (.05f, .05f, .05f, 1.f);
const Color dim     (.4f, .4f, .4f, .4f);
const Color bright  (.8f, .8f, .9f, .5f);


const Color softBlue(50, 90, 120, 255);



// TODO: replace with CUBE primitive and swap material on that? Would be a lot simpler...
Light::Light() {
    SetName("Light Example");

    // Like before, let's load the model. This is the same as in the previous example.
    //id = Assets::Load("obj", "test.obj");
    //Entity::ID modelObject = Assets::Get<Model>(id).Create();
    //Attach(modelObject);

    Entity::ID cube = Entity::CreateChild<Entity>()->GetID();
    cube.Identify()->SetName("CUBE");
    cube.Identify()->BindComponent<RenderMesh>();
    cube.Query<RenderMesh>()->AddMesh(Dynacoe::Mesh::Basic_Cube());


    /* Material Setup */

    // Here, we are trying to form a sort of rough-metallic looking material,
    // almost like metallic paint. Dynacoe's basic lighting is based off of
    // the Phong reflection model, so all colors produced by lighting is
    // spawned from these three colors:
    // the ambient, diffuse, and specular.

    // The ambient color persists regardless of the amount or proximity of light.
    // Often, it makes the most sense to keep this color as dark as possible.
    mat.state.ambient = dark;


    // The diffuse color is the color seen when at a wide angle of reflection between
    // the light source and the camera. It's called "diffuse" light betcause it's more scattered
    // than its counterpart, specular light. It is independent of the camera position.
    mat.state.diffuse = dim;

    // The specular color is the color added onto the diffuse color when the light "lines up" closest
    // with the camera's view. Because this reflection model is additive, this often appears bright,
    // as it will usually stack with the Diffuse light.
    mat.state.specular = bright;

    // Shininess controls how intense the specular highlights are.
    mat.state.shininess = 16;


    // If we want to use lighting, we need to remember to set this or else the material
    // will default to the default program which doesn't use lighting.
    mat.SetProgram(Material::CoreProgram::Lighting);


    // When models are loaded, they often have subgroups of smaller meshes. So,
    // we need to apply the material to those as well.







    /* Light Setup */

    // Now we actual form the lights in the scene.
    // When using a lighting shader, only the ambient color is visible
    // if theres no light. That's kind of really boring, so lets make some lights.
    // Lights are formed by another type of aspect, called an RenderLight.
    // Unlike AspectMesh, RenderLights do not need to be drawn. Rather, they affect
    // things that are going to be drawn.

    // To set it up, it is much light other aspects. The first argument denotes which kind of light it is.
    // We will use point lights, which are sensitive to proximity. The second argument is how intense it is
    // and the third is what color it should appear as.
    light.FormLight(RenderLight::Light::Point);
    light.state.intensity = 20.f;
    light.state.color = "white";
    //light.state.position = {0, 1.2, 0};

    // We will set up another light, just for fun.
    // This time, though, it will be a directional light.
    // Directional lights are different in that their position is a relative direction
    // showing where the light is coming from. Directional lights can
    // be thought of as lights that are at a very large distance away.

    skyLight.FormLight(RenderLight::Light::Directional);
    skyLight.state.intensity = 2.0f;
    skyLight.state.color = softBlue;
    skyLight.state.position = {0, -1, -.4};







    target = {0, 0, -3.2f};
    //AddComponent(&aspect);
    AddComponent(&skyLight);

    Entity * ent = CreateChild<Entity>();
    ent->AddComponent(&light);
    RenderMesh * r = ent->BindComponent<RenderMesh>();
    r->AddMesh(Mesh::Basic_Cube());
    ent->SetName("LIGHT POINT");
    ent->node.Position() = {1, 1, 1};
    r->node.Scale() = {.1, .1, .1};

    cube.Query<RenderMesh>()->Material() = mat;
    /*
    auto children = GetAllSubEntities();
    for(uint32_t i = 0; i < children.size(); ++i) {
        RenderMesh * m = children[i].Query<RenderMesh>();
        if (m) {
            Console::Info() << "Replaced for " << children[i].Identify()->GetName() << "\n";
            m->Material() = mat;
        }
    }
    */


    RenderMesh * ground = BindComponent<RenderMesh>();
    ground->AddMesh(Mesh::Basic_Square());
    ground->node.Position() = {0, -2, 0};
    ground->node.Scale() = {1000, 1000, 1000};
    ground->Material() = mat;



    node.Position() = {0, 0, -10};
}

void Light::OnStep() {
    // This function controls the camera movement logic. It
    // is not the focus, but it is available below.
    // The camera controls are 'W'A'S'D'Q'E' for XYZ camera movement
    controlCamera();

    // To make that white light more interesting,
    // we update its position to the camera's current position.
    //light.SetPosition(Position);

    // We slowly rotate the loaded model to make it more interesting.
    node.Rotation().y += .7f;
    //node.local.rotation.x += .3f;
    //node.local.rotation.z += .8f;

}






//// PRIVATE ////

// Same as previous example. Just gives you the ability to move around
void Light::controlCamera() {

   Camera * cam = &Graphics::GetCamera3D();

    if (Input::GetState(Keyboard::Key_w)) {
        target.y += .4f;
    }

    if (Input::GetState(Keyboard::Key_s)) {
        target.y -= .4f;
    }

    if (Input::GetState(Keyboard::Key_a)) {
        target.x -= .4f;
    }

    if (Input::GetState(Keyboard::Key_d)) {
        target.x += .4f;
    }

    if (Input::GetState(Keyboard::Key_q)) {
        target.z -= .4f;
    }

    if (Input::GetState(Keyboard::Key_e)) {
        target.z += .4f;
    }

    float lightMod = 0;
    if (Input::GetState(Keyboard::Key_o)) {
        lightMod -= .1;
    }

    if (Input::GetState(Keyboard::Key_p)) {
        lightMod += .1;
    }

    if (lightMod != 0.0) {
        light.state.intensity += lightMod;
    }



    // ease towards position
    cam->node.Position().x = Mutator::StepTowards(cam->node.GetPosition().x, target.x, .1);
    cam->node.Position().y = Mutator::StepTowards(cam->node.GetPosition().y, target.y, .1);
    cam->node.Position().z = Mutator::StepTowards(cam->node.GetPosition().z, target.z, .1);


}
