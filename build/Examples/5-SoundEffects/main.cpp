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

/*  An example showing how to
    play some sounds.

    2018, Johnathan Corkery
*/

#include <Dynacoe/Library.h>
using namespace Dynacoe;

// This is the first example that will employ the use of multiple entities.
// This helper entity will, when setup, play a sound 
// when the given key is pressed.
//
// When the sound is playing, the visual will be active
class Instrument : public Entity {
  public:
      
    Instrument() {
        // each instrument will have a Shape2D visual associated with it
        shape = AddComponent<Shape2D>();
        
        // The visual this time is a circle. Because of the nature of 
        // vertex-based graphics setup that Dynacoe uses, circles are approximated 
        // with a number of finite sides. This has advantages, such as being able to use 
        // this same function to produce small geometric primitives, such as pentagons
        // and triangles. Here we want a circle, though, so we use 40 iterations,
        // much more than we can make out with our eyes, so it looks quite smooth.
        shape->FormCircle(50, 40);
        
    }


    // This is a custom function we're adding so that we can give the instrument 
    // specific information, such as the sample to play and the key to use.
    void Setup(
        const std::string & samplePath, 
        Keyboard keyIn,
        Color color) {
            
        // We also delay setting the name until we have the info that 
        // makes it behave differently. We use the sample as part of the name
        SetName("Instrument: " + samplePath);

        
        // We also want it visually to differ, so we accept a color for it.
        shape->color = color;
        
        // This loads the given sound file path as a file asset, much like in 
        // the Image example. The asset is used to play sounds later. Playing sounds 
        // does not require an component, though.
        sample = Assets::Load("wav", samplePath);

        // We save this key to detect whether it's pressed later.
        key = keyIn;
        
        // Also!!! We want to make the visual a little fancy, so 
        // we will scale it dynamically based on if its sample is 
        // actively playing. We will need 2 variables to do this,
        // and we set them to 1 to start.
        scaleTarget = scaleReal = 1.0;
        
        
    }
    
    void OnStep() {
        // First, lets check to see if the designated key is pressed.
        if (Input::IsPressed(key)) {
            
            // If it is, play the sound sample assigned to this instrument.
            // Any time a sample is played, a special ActiveSound object is created.
            // ActiveSounds can be used to manipulate and track the sound 
            // while its playing. This allows for features like active volume,
            // panning, and halting.
            lastSound = Sound::PlayAudio(sample);
        }
        
        
        
        // If an active sound is around and currently playing,
        // make the scale of the visual larger than normal, else 
        // revert the scale back to normal (1.0).
        if (lastSound.Valid()) {
            scaleTarget = 2.0;
        } else {
            scaleTarget = 1.0;
        }


        // Mutator::StepTowards eases the real scale used for the visual 
        // toward a target scale one step at a time, creating a 
        // transition to a desired scale.
        scaleReal = Mutator::StepTowards(scaleReal, scaleTarget, .1);


        // Now that we have an transitioning value, we need to remember
        // to use and apply it
        shape->Node().Scale() = {
            scaleReal, scaleReal
        };

    }
    
  private:
      
    // Reference to the circle
    Shape2D * shape;

    // Asset reference
    AssetID sample;

    // Playback sound of the sample asset
    ActiveSound lastSound;


    // Desired scale of the visual itself
    float scaleTarget;
    
    // Real scale of the visual itself
    float scaleReal;
    
    // Key to listen for to see if we should take action
    Keyboard key;
};





// As mentioned before, this is the first example that employs the use 
// of multiple entities. Here we want 2 instruments: one to play a snare 
// drum, and one to play a kick drum
int main() {
    
    // Start up as usual
    Engine::Startup();    
    ViewManager::NewMain("Sound Example");


    // This time we create an empty root entity. This isnt visible
    // but serves as a container for the other entities we want to add.
    // Entities are organized in a hierarchy where each entity has one parent 
    // and 0 or more children. The root entity is the exception, which has 
    // no parent.
    Engine::Root() = Entity::Create();

    // Create 2 independent entities, one for a snare and one for a kick.
    Instrument * snare  = Entity::CreateReference<Instrument>();
    Instrument * kick   = Entity::CreateReference<Instrument>();

    // Setup each entity with its own sample, key trigger, and color
    snare->Setup("snare.wav", Keyboard::Key_q, Color("violet"));
    kick ->Setup("kick.wav",  Keyboard::Key_w, Color("green"));


    // Position them on the screep. The snare is a 1/4 of the screen 
    // width from left and the kick is 3/4s of the screen width from 
    // the left
    snare->Node().Position() = {
        Graphics::GetRenderCamera().Width() * .25,
        Graphics::GetRenderCamera().Height()  / 2.f
    };

    kick->Node().Position() = {
        Graphics::GetRenderCamera().Width() * .75,
        Graphics::GetRenderCamera().Height()  / 2.f
    };



    // Then we have to remember to attach them to the entity 
    // hierarchy! We access the root entity and add the instruments as children
    Engine::Root().Identify()->Attach(snare->GetID());
    Engine::Root().Identify()->Attach(kick->GetID());


    // Finally, run the engine.
    Engine::Run(); 
    
    return 0;
}

