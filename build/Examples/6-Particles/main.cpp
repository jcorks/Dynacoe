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
/*  An example for creating and displaying particle effects.

    2016, Johnathan Corkery
 */
 
#include <Dynacoe/Library.h>
using namespace Dynacoe;

// Particles
//
// Particle effects require a few things:
// 1) The particle you are going to display. This usually is done through loading a particle file as an asset.
// 2) A ParticleEmitter. ParticleEmitters are the managers for particles

// This example wraps both the emitter and the function of emitting.
class ParticlesExample : public Entity {
  public:
    // Once we load the particle, this will hold the reference to it.
    AssetID particle;

    // The emitter for the particles
    ParticleEmitter2D * particleEmitter;

    ParticlesExample() {

        // test.part in this directory holds the specification for the particle.
        // Try editing it to see what different effects you can achieve
        particle        = Assets::Load("part", "test.part");

        // Create and attach the emitter to this world
        particleEmitter = Entity::CreateChild<ParticleEmitter2D>(); 
        particleEmitter->SetName("ParticleExample");
    }

    void OnStep() {

        // Lets have the emitter always match the mouse's position
        particleEmitter->Node().Position() = {Input::MouseX(), Input::MouseY()};


        //... and let's have the emitter activate when the user clicks.
        if (Input::GetState(MouseButtons::Left)) {
            particleEmitter->EmitParticle(particle);
        }


        // Thats all we need! Below controls some extra options that could be helful to know.



        // Pressing 'tab' should toggle whether to enable texture filtering for particles.
        // When texture filtering is enabled, the pixels of the image of the particle are 
        // blended together. When it's disabled, it tends to give a grainy look, but 
        // will also run faster on some machines. The default when emitting is to use 
        // filtering.
        if (Input::IsPressed(Keyboard::Key_tab)) {
            static bool b = false;
            particleEmitter->EnableFiltering(b);
            Console::Info() << (b ? "Enabled" : "Disabled") << " filtering!" << Console::End;
            b = !b;
        }

        // Pressing the spacebar will toggle whether to draw the particles in a translicent manner.
        // In this context, "translucency" is equivalent to additive blending, where transparent colors
        // are added together. The event is layered, transparent images appear lighter in color to more 
        // are drawn in one place. The default when emitting is to draw translucently.
        if (Input::IsPressed(Keyboard::Key_space)) {
            static bool b = false;
            particleEmitter->EnableTranslucency(b);
            Console::Info() << (b ? "Enabled" : "Disabled") <<  " translucency!" << Console::End;
            b = !b;
        }

    }

};



// The driver as usual, back to just one entity.
int main() {
    Engine::Startup();    
    ViewManager::NewMain("Rectangles");

    
    Engine::Root() = Entity::Create<ParticlesExample>(); 
    Engine::Run();
    
    return 0;
}

