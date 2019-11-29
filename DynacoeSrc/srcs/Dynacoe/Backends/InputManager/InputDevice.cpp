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
#include <Dynacoe/Util/Chain.h>
#include <cstring>
#include <cstdlib>  
#include <iostream>
#include <unordered_map>


using namespace std;
using namespace Dynacoe;

const int default_action_queue_size = 1024;

class Dynacoe::InputDevice_Impl {
  public:
    InputDevice_Impl(InputDevice::Class type_) : 
            allocated(0),
            iterFront(0),
            iterBack(0),
            type(type_)
        {
        data = (InputDevice::Event*)calloc(sizeof(InputDevice::Event), default_action_queue_size);
        allocated = default_action_queue_size;
    }

    ~InputDevice_Impl() {
        free(data);
    }


    void PushEvent(const InputDevice::Event & evt) {
        if (iterBack >= allocated) {
            // move to the front
            if (iterFront > 0) {
                memmove(data, data+iterFront, (iterBack-iterFront)*sizeof(InputDevice::Event));
                iterBack -= iterFront;
                iterFront = 0;
            }

            if (iterBack >= allocated) {
                allocated *= 1.4;
                realloc(data, allocated*sizeof(InputDevice::Event));
            }
        }

        data[iterBack++] = evt;

        // deadzone calculation
        float deadz = deadzone[evt.id];
        if (deadz > .001) {
            float val = data[iterBack-1].state; 
            if (val > deadz) {
                val = (val - deadz) / (1 - deadz);
                data[iterBack-1].state = val;

            } else if (val < -deadz) {

                val = (val + 1) / (-deadz + 1);
                data[iterBack-1].state = val-1;
            } else {
                data[iterBack-1].state = 0;
            }
        }
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

    void SetDeadzone(int id, float zone) {
        deadzone[id] = zone;
    }

  private:
    std::unordered_map<int, float> deadzone;
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

void InputDevice::PushEvent(const InputDevice::Event & ev) {
    self->PushEvent(ev);
}

bool InputDevice::PopEvent(InputDevice::Event & ev) {
    return self->PopEvent(ev);
}

void InputDevice::SetDeadzone(int id, float zone) {
    self->SetDeadzone(id, zone);
}

InputDevice::Class InputDevice::GetType() const {
    return self->GetType();
}

int InputDevice::GetEventCount() const {
    return self->GetEventCount();
}





static const char * builtIn[] = {
    "NotAnInput",
    "Key_0", ///< 0
    "Key_1", ///< 1
    "Key_2", ///< 2
    "Key_3", ///< 3
    "Key_4", ///< 4
    "Key_5", ///< 5
    "Key_6", ///< 6
    "Key_7", ///< 7
    "Key_8", ///< 8
    "Key_9", ///< 9
    "Key_a", ///< a
    "Key_b", ///< b
    "Key_c", ///< c
    "Key_d", ///< d
    "Key_e", ///< e
    "Key_f", ///< f
    "Key_g", ///< g
    "Key_h", ///< h
    "Key_i", ///< i
    "Key_j", ///< j
    "Key_k", ///< k
    "Key_l", ///< l
    "Key_m", ///< m
    "Key_n", ///< n
    "Key_o", ///< o
    "Key_p", ///< p
    "Key_q", ///< q
    "Key_r", ///< r
    "Key_s", ///< s
    "Key_t", ///< t
    "Key_u", ///< u
    "Key_v", ///< v
    "Key_w", ///< w
    "Key_x", ///< x
    "Key_y", ///< y
    "Key_z", ///< z
    "Key_lshift", ///< Left shift key
    "Key_rshift", ///< Right shift key
    "Key_lctrl",  ///< Left control key
    "Key_rctrl",  ///< Right control key
    "Key_lalt",   ///< Left alt key
    "Key_ralt",   ///< Right alt key
    "Key_tab",    ///< Tab
    "Key_F1",     ///< F1
    "Key_F2",     ///< F2
    "Key_F3",     ///< F3
    "Key_F4",     ///< F4
    "Key_F5",     ///< F5
    "Key_F6",     ///< F6
    "Key_F7",     ///< F7
    "Key_F8",     ///< F8
    "Key_F9",     ///< F9
    "Key_F10",    ///< F10
    "Key_F11",    ///< F11
    "Key_F12",    ///< F12
    "Key_up",     ///< Up arrow
    "Key_down",   ///< Down arrow
    "Key_left",   ///< Left arrow
    "Key_right",  ///< Right arrow
    "Key_minus",  ///< -
    "Key_equal",  ///< = 
    "Key_backspace",  ///< Backspace
    "Key_grave",  ///< `
    "Key_esc",    ///< Escape
    "Key_home",   ///< Home key
    "Key_pageUp", ///< Page up key
    "Key_pageDown",  ///< Page down key
    "Key_end",    ///< End key
    "Key_backslash", ///< '\'
    "Key_lbracket", ///< [
    "Key_rbracket", ///< ]
    "Key_semicolon", ///< ;
    "Key_apostrophe", ///< '
    "Key_frontslash", ///< /
    "Key_enter", ///< Enter
    "Key_delete", ///< Delete
    "Key_numpad0", ///< Numpad 0
    "Key_numpad1", ///< Numpad 1
    "Key_numpad2", ///< Numpad 2
    "Key_numpad3", ///< Numpad 3
    "Key_numpad4", ///< Numpad 4
    "Key_numpad5", ///< Numpad 5
    "Key_numpad6", ///< Numpad 6
    "Key_numpad7", ///< Numpad 7
    "Key_numpad8", ///< Numpad 8
    "Key_numpad9", ///< Numpad 9
    "Key_prtscr", ///< Print screen button
    "Key_lsuper", ///< Left Super key (Windows key)
    "Key_rsuper", ///< Right Super key (Windows key)
    "Key_space",  ///< Space
    "Key_insert", ///< Insert key
    "Key_comma", ///< ,
    "Key_period", ///< .

    "Pointer_0", ///< Left click
    "Pointer_1", ///< Right click
    "Pointer_2", ///< Middle click

    "Pointer_X", ///< Horizontal axis. Usually for the X axis of the pointer
    "Pointer_Y", ///< Horizontal axis. Usually for the X axis of the pointer
    "Pointer_Wheel", ///< Mouse wheel.

    "Pad_a",     ///< Button 0
    "Pad_b",     ///< Button 1
    "Pad_c",     ///< Button 2
    "Pad_x",     ///< Button 3
    "Pad_y",     ///< Button 4
    "Pad_r",     ///< Button 5
    "Pad_l",     ///< Button 6
    "Pad_r2",    ///< Button 7
    "Pad_l2",    ///< Button 8
    "Pad_r3",    ///< Button 9
    "Pad_l3",    ///< Button 10
    "Pad_start",    ///< Button 11
    "Pad_select",///< Button 12
    "Pad_b13",///< Button 13
    "Pad_b14",///< Button 14
    "Pad_b15",///< Button 15
    "Pad_b16",///< Button 16
    "Pad_b17",///< Button 17
    "Pad_b18",///< Button 18
    "Pad_b19",///< Button 19
    "Pad_b20",///< Button 20
    "Pad_b21",///< Button 21
    "Pad_b22",///< Button 22
    "Pad_b23",///< Button 23
    "Pad_b24",///< Button 24
    "Pad_b25",///< Button 25
    "Pad_b26",///< Button 26
    "Pad_b27",///< Button 27
    "Pad_b28",///< Button 28
    "Pad_b29",///< Button 29
    "Pad_b30",///< Button 30
    "Pad_b31",///< Button 31
    "Pad_b32",///< Button 32

    "Pad_axisX", ///< X button
    "Pad_axisY", ///< Y button
    "Pad_axisZ", ///< Z button
    "Pad_axisX2",///< X2 button 
    "Pad_axisY2",///< Y2 button
    "Pad_axisZ2",///< Z2 button
    "Pad_axisX3",///< X3 button
    "Pad_axisY3",///< Y3 button
    "Pad_axisZ3",///< Z3 button
    "Pad_axisX4",///< X4 button
    "Pad_axisY4",///< Y4 button
    "Pad_axisZ4",///< Z4 button
    "Pad_axisX5",///< X4 button
    "Pad_axisY5",///< Y4 button
    "Pad_axisZ5",///< Z4 button
    "Pad_axisX6",///< X4 button
    "Pad_axisY6",///< Y4 button
    "Pad_axisZ6",///< Z4 button

    "Pad_axisR",    
    "Pad_axisL",    
    "Pad_axisR2",    
    "Pad_axisL2",    
    "Pad_axisR3",    
    "Pad_axisL3",    
    "Pad_axisR4",    
    "Pad_axisL4",    

    "Pad_options"
};
    

std::string InputManager::IDToString(int id) {
    if (id < Dynacoe::UserInput::Count) {
        return builtIn[id];
    }

    return Dynacoe::Chain() << "Input" << id;
}