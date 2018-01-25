/*

Copyright (c) 2016, Johnathan Corkery. (jcorkery@umich.edu)
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


#ifndef H_LISTENER_INCLUDED
#define H_LISTENER_INCLUDED

#include <Dynacoe/Library.h>

class Listener {
  public:

    // plays a sound as if the camera was the listener.
    // lets just pretend we can hear it in space
    static void PlaySoundProximity(const Dynacoe::Point & location, Dynacoe::AssetID asset) {
        float distance = pos.Distance(location);
        float volume = 1.f - distance/(Dynacoe::Graphics::GetRenderCamera().Width()/2.f);
        
        float panning  = (((1.f - volume)*.5) * (location.x < pos.x ? -1 : 1)) + .5;     

        Dynacoe::Sound::PlayAudio(asset, 0, volume, panning);
    }


    static void SetPosition(const Dynacoe::Point & p) {
        pos = p;
    }


  private:

    static Dynacoe::Point pos;
};

Dynacoe::Point Listener::pos;

#endif
