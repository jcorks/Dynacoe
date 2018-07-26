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


#include "sound.h"


using namespace Dynacoe;


// THis is a pretty simple example that uses the same concepts as the 
// previous ones but call sound-related functions.

 SoundEffects::SoundEffects() {
    kick  = AddComponent<Dynacoe::Shape2D>();
    snare = AddComponent<Dynacoe::Shape2D>();





    // Load resources
    // Dynacoe by default can recognize WAVs and OGGs.
    kickSound  = Assets::Load("wav", "kick.wav");
    snareSound = Assets::Load("wav", "snare.wav");

    // We're also going to draw some visuals
    kickImage  = Assets::Load("png", "kick.png");
    snareImage = Assets::Load("png", "snare.png");
    baseImage  = Assets::Load("png", "base.png");



    kick->node.Position() = {
        Graphics::GetRenderCamera().Width()  * .1, 
        Graphics::GetRenderCamera().Height()  * .2
    };


    snare->node.Position() = {
        Graphics::GetRenderCamera().Width()  * .5, 
        Graphics::GetRenderCamera().Height()  * .2
    };


    Console::Info() << "Press Q or W!" << "\n";

}


void SoundEffects::OnStep() {





    // To actually play a sound, we can call PlayAudio() to immediately hear it. 
    // The Sound modules handles all audio.
    if (Input::IsPressed(Keyboard::Key_q))
        Sound::PlayAudio(kickSound);

    if (Input::IsPressed(Keyboard::Key_w))
        Sound::PlayAudio(snareSound);

    
    // It is also possible to get whether the sound is currently playing.
    kick->FormImage(Sound::IsPlaying(kickSound)?
            kickImage : baseImage);

    snare->FormImage(Sound::IsPlaying(snareSound)?
            snareImage : baseImage);

}



