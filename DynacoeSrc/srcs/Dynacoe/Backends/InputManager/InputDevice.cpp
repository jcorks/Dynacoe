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


InputDevice::InputDevice(int _nButtons, int _nAxes) :
    numButtons(_nButtons),
    numAxes(_nAxes),
    axes(nullptr),
    buttons(nullptr) {


    if (_nButtons > 0) {
        buttons = new bool[numButtons];
        memset(buttons, 0, sizeof(bool)*numButtons);
    }
    
    if (_nAxes > 0) {
        axes = new float[numAxes];
        memset(axes, 0, sizeof(float)*numAxes);
    }
            
}



InputDevice::~InputDevice() {
    if (axes) delete[] axes;
    if (buttons) delete[] buttons;
}



InputDevice * InputDevice:: GetCopy() {
    // TODO: this is pretty wasteful
    InputDevice * out = new InputDevice(numButtons, numAxes);
    memcpy(out->axes, axes, sizeof(float)*numAxes);
    memcpy(out->buttons, buttons, sizeof(bool)*numButtons);
    return out;
}

void InputDevice::CopyInto(InputDevice *& m) {
    if (m)  delete m;
    m = GetCopy();
}


