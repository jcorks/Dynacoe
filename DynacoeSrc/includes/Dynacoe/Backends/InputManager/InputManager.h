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

#ifndef H_DC_INPUT_MANAGER
#define H_DC_INPUT_MANAGER

#include <Dynacoe/Backends/InputManager/InputTypes.h>
#include <Dynacoe/Backends/Backend.h>
#include <cstring>


namespace Dynacoe {
class Display;
class InputDevice;


/* Interacts with device drivers to retrieve the requested input information. */

class InputManager : public Backend {
  public:
    virtual ~InputManager(){};
        
    enum InputType {
        Keyboard = 0x1,
        Gamepad  = 0x01,
        Mouse    = 0x001,
        Touchpad = 0x0001  
    };   



    enum class DefaultDeviceSlots {
        Keyboard,
        Mouse,
        Touchpad,
        Pad1,
        Pad2,
        Pad3,
        Pad4,
        NumDefaultDevices
    };


    // Returns if the given InputType is supported
    // If an input type is not supported, querying the associated device 
    // will result in nullptr 
    virtual bool IsSupported(InputType) = 0;


    // Updates the state of registered devices.
    // Returns whether or not there were new input events that were pulled  
    virtual bool HandleEvents() = 0;

    // Returns a reference to the internally maintained input device.
    // Do not free or modify the contents of the device. The first few slots 
    // up to DefaultDeviceSlots::NumDefaultDevices will match the device expected
    virtual InputDevice * QueryDevice(int ID) = 0;
    virtual InputDevice * QueryDevice(DefaultDeviceSlots) = 0;
   
    // Returns the number of additional devices available.
    // up to MaxDevices(). Typically, any overflow devices that weren't able to 
    // fit in the first 4 slots will be put here.
    // Given an array of sise MaxDevices(), IDs will be filled 
    // with the index to a device available through QueryDevice that isnt 
    // a default device
    virtual int QueryAuxiliaryDevices(int * IDs) = 0;


    // Maximum number of supported. Will always be at least NumDefaultDevices in count
    virtual int MaxDevices() = 0;

    // Sets the focus on which to query input from.
    // On multi-display systems, such as a desktop environment,
    // input is based around the context of the focused display.
    // On some implementations, such a distinction (and as a result, this function)  is not necessary
    // may not be necessary, but this will almost 
    // always be necessary for modern environments.
    // Passing in nullptr should disable input until a 
    // valid display is given.
    virtual void SetFocus(Display *) = 0;

    // Returns the current focus. The default is nullptr;
    virtual Display * GetFocus() = 0;


    // Attempts to open a virtual keyboard. This is intended to 
    // follow the same input detection as a normal keyboard, so no additional
    // behavior is required or guaranteed outside of the conceptual idea 
    // of the opening of a virtual keyboard. By default, this does nothing.
    virtual void ShowVirtualKeyboard(bool){};

};

}

#endif
