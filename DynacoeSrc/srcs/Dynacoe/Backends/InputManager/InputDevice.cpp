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

#include <Dynacoe/Backends/InputManager/InputDevice.h>
#include <cstring>
#include <cstdlib>  
#include <iostream>


using namespace std;
using namespace Dynacoe;

const int default_action_queue_size = 1024;

class InputDevice_Impl {
  public:
    InputDevice_Impl(InputDevice::Class type_) : 
            allocated(0),
            iterFront(0),
            iterBack(0),
            type(type_)
        {
        data = (AxisEvent*)calloc(sizeof(AxisEvent), default_action_queue_size);
        allocated = default_action_queue_size;
    }

    ~InputDevice_Impl() {
        free(data);
    }


    void AddEvent(const InputDevice::Event & evt) {
        if (iterBack >= allocated) {
            // move to the front
            if (iterFront > 0) {
                memmove(data, data+iterFront, iterBack*sizeof(AxisEvent));
                iterBack -= iterFront;
                iterFront = 0;
            }

            if (iterBack >= allocated) {
                allocated *= 1.4;
                realloc(data, allocated*sizeof(AxisEvent));
            }
        }

        data[iterBack++] = ev;
    }

    
    bool PopEvent(InputDevice::Event & ev) {
        if (iterFront >= iterBack) return false;
        ev = data[iterFront++];
        return true;
    }

    InputDevice::Class GetType() const {
        return type;
    }

    int GetEventCount() const {
        return iterBack - iterFront;
    }

  private:
    InputDevice::Event * data;
    int allocated;
    int iterFront;
    int iterBack;
    InputDevice::Class type;
};


InputDevice::InputDevice(InputDevice::Class t) {
    self = new InputDevice_Impl(t);
}

InputDevice::~InputDevice() {
    delete self;
}

void InputDevice::AddEvent(const InputDevice::Event & ev) {
    self->AddEvent(ev);
}

bool InputDevice::PopEvent(InputDevice::Event & ev) {
    return self->PopEvent(ev);
}

InputDevice::Class InputDevice::GetType() const {
    return self->GetType();
}

int InputDevice::GetEventCount() const {
    return self->GetEventCount();
}







