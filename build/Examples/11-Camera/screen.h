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


#ifndef H_SCREEN_INCLUDED
#define H_SCREEN_INCLUDED

#include <Dynacoe/Library.h>
#include "shadercube.h"

// the screen with a 3d rendered visual in it
//
// The main ideas: We have this flat surface with a 3D visual on it, so we need 
// - The flat surface object 
// - The 3D visual to be embedded 
// 
class Screen : public Dynacoe::Entity {
  public:
    // The embedded 3D object
    ShaderCube * scene;
    
    // The camera to store the rendered results of the 3D object
    Entity::ID   camera;
    
    // The surface to display the rendered results
    Dynacoe::RenderMesh * surface;

    Screen() {
        // Adds the surface, but as a component ot be updated *AFTER* the entity's Draw/Step
        surface = AddComponent<Dynacoe::RenderMesh>(UpdateClass::After);
        
        // Surface is now a square
        surface->AddMesh(Dynacoe::Mesh::Basic_Square());

        // Basic transform info
        node.Rotation().x = 90;
        node.Scale() = {4, 4, 4};


        // Creates the shader object as a child of the camera so that it always
        // follows the camera independent of it
        scene = Dynacoe::Graphics::GetCamera3D().CreateChild<ShaderCube>();

        // We only want to draw it when we want to update the visual displayed 
        // on the surface, so we disable stepping and drawing automatically.
        scene->draw = false;
        scene->step = false;

        // Creates a custom camera that will hold the rendered visual
        camera = Dynacoe::Engine::Root().Identify()->CreateChild<Dynacoe::Camera>()->GetID();

        // The surface will just hold the texture, so we dont need anything fancy...
        surface->Material().SetProgram(Dynacoe::Material::CoreProgram::Basic);
        
        // ...Except this. THis call explicitly tells the Material for the surface 
        // to always use the custom camera's Framebuffer (the image representing rendered results)
        // as a texture.
        surface->Material().SetFramebufferSource(*camera.IdentifyAs<Dynacoe::Camera>());

    }



    void OnDraw() {
        Entity::ID current = Dynacoe::Graphics::GetRenderCamera().GetID();

        // We take our custom camera and use it as the rendering target
        Dynacoe::Graphics::SetRenderCamera(*camera.IdentifyAs<Dynacoe::Camera>());
        camera.IdentifyAs<Dynacoe::Camera>()->Refresh();



        // Update the camera 
        camera.Identify()->Step();
        camera.Identify()->Draw();


        // Re-enable the shader object with the funky visual and draw it manually
        scene->draw = true;
        scene->step = true;

        scene->Step();
        scene->Draw();

        scene->draw = false;
        scene->step = false;


        // Return the render camera back to the normal one.
        Dynacoe::Graphics::SetRenderCamera(*current.IdentifyAs<Dynacoe::Camera>());
    }

    
    void OnStep() {
        // rotate the surface to make it more clear that 
        // the surface is indepent of the visual.
        node.Rotation().z += .3;
    }


};


#endif
