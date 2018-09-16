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
    set up a light.

    This example assumes you have the knowledge 
    to do the Model example.

    Use the WASD and QE keys to mvoe the camera.
    "o" and "p" control the camera light's brightness.
    As usaul, see the source for more information.

    Johnathan Corkery, 2015
 */


using namespace Dynacoe;


const Color dark    (.05f, .05f, .05f, 1.f);
const Color dim     (.4f, .4f, .4f, .4f);
const Color bright  (.8f, .8f, .9f, .5f);


const Color softBlue(50, 90, 120, 255);



class Light : public Dynacoe::Entity {
  public:
  Light() {
      SetName("Light Example");

      // Like before, let's load the model. This is the same as in the previous example.
      //id = Assets::Load("obj", "test.obj");
      //Entity::ID modelObject = Assets::Get<Model>(id).Create();
      //Attach(modelObject);

      Entity::ID cube = Entity::CreateChild<Entity>()->GetID();
      cube.Identify()->SetName("CUBE");
      cube.Identify()->AddComponent<RenderMesh>();
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
      Entity * ent = CreateChild<Entity>();







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
      light = ent->AddComponent<RenderLight>();
      light->FormLight(RenderLight::Light::Point);
      light->state.intensity = 20.f;
      light->state.color = "white";
      //light.state.position = {0, 1.2, 0};

      // We will set up another light, just for fun.
      // This time, though, it will be a directional light.
      // Directional lights are different in that their position is a relative direction
      // showing where the light is coming from. Directional lights can
      // be thought of as lights that are at a very large distance away.
      skyLight = AddComponent<RenderLight>();
      skyLight->FormLight(RenderLight::Light::Directional);
      skyLight->state.intensity = 2.0f;
      skyLight->state.color = softBlue;
      skyLight->state.position = {0, -1, -.4};




      RenderMesh * r = ent->AddComponent<RenderMesh>();
      r->AddMesh(Mesh::Basic_Cube());
      ent->SetName("LIGHT POINT");
      ent->Node().Position() = {1, 1, 1};
      r->Node().Scale() = {.1, .1, .1};
      cube.Query<RenderMesh>()->Material() = mat;




      // Flat surface to provide spacial context for the example. 
      // It also features reflection.
      RenderMesh * ground = AddComponent<RenderMesh>();
      ground->AddMesh(Mesh::Basic_Square());
      ground->Node().Position() = {0, -2, 0};
      ground->Node().Scale() = {1000, 1000, 1000};
      ground->Material() = mat;


      target = {0, 0, -3.2f};
      Node().Position() = {0, 0, -10};
    }
    void OnStep() {
        // This function controls the camera movement logic. It
        // is not the focus, but it is available below.
        // The camera controls are 'W'A'S'D'Q'E' for XYZ camera movement
        controlCamera();


        // We slowly rotate the loaded model to make it more interesting.
        Node().Rotation().y += .7f;

    }
  private:

    void controlCamera() {

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
            light->state.intensity += lightMod;
        }



        // ease towards position
        cam->Node().Position().x = Mutator::StepTowards(cam->Node().GetPosition().x, target.x, .1);
        cam->Node().Position().y = Mutator::StepTowards(cam->Node().GetPosition().y, target.y, .1);
        cam->Node().Position().z = Mutator::StepTowards(cam->Node().GetPosition().z, target.z, .1);


    }

    Dynacoe::Vector target;
    Dynacoe::AssetID id;
    Dynacoe::RenderLight * light;
    Dynacoe::RenderLight * skyLight;

    Dynacoe::Material mat;


  

}; 



int main() {
    Engine::Startup();    

    ViewManager::NewMain("Light Example");

    Engine::Root() = Entity::Create<Light>();
    Engine::Run(); 
    
    return 0;
}

