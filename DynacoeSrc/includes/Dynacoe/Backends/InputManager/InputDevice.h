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
#ifndef H_DC_INPUTDEVICE
#define H_DC_INPUTDEVICE

#include <Dynacoe/Backends/InputManager/InputManager.h>


    /* Input devices are devies that the user interacts with via
       buttons (boolean input) or axes (ranged / discrete input) */

namespace Dynacoe {
class InputDevice_Impl;
class InputDevice {
  public:
    enum class Class {
        Keyboard,
        Gamepad,
        Pointer
    };


    struct Event {
        UserInput id;         // input mapping
        float state;          // state
        int utf8; // unicode mapping to the key, if any
    };


    InputDevice(Class);
    ~InputDevice();

    // Adds a state change event for the given input within
    // this device
    void PushEvent(const Event &);


    // Gets the next state change for a device input
    bool PopEvent(Event &);


    // Returns the device type
    Class GetType() const;


    // Returns the number of button events pending
    int GetEventCount() const;

  private:
    InputDevice_Impl * self;
};
}


#endif
