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

#ifdef ANDROID 
#ifndef H_DC_INPUT_ANDROID_BRIDGE
#define H_DC_INPUT_ANDROID_BRIDGE

#include <Dynacoe/Backends/InputManager/InputManager.h>


namespace Dynacoe {


/* Interacts with device drivers to retrieve the requested input information. */
class AndroidBridge_Input_Data;
class AndroidBridge_Input : public InputManager {
  public:
    AndroidBridge_Input();
    ~AndroidBridge_Input();


    // Returns if the given InputType is supported
    // If an input type is not supported, querying the associated device 
    // will result in nullptr 
    bool IsSupported(InputType){return true;}


    // Updates the state of registered devices.
    // Returns whether or not there were new input events that were pulled  
    bool HandleEvents();

    // Returns a reference to the internally maintained input device.
    // Do not free or modify the contents of the device. The first few slots 
    // up to DefaultDeviceSlots::NumDefaultDevices will match the device expected
    InputDevice * QueryDevice(int ID);
    InputDevice * QueryDevice(DefaultDeviceSlots);
   
    // Returns the number of additional devices available.
    // up to MaxDevices(). Typically, any overflow devices that weren't able to 
    // fit in the first 4 slots will be put here.
    // Given an array of sise MaxDevices(), IDs will be filled 
    // with the index to a device available through QueryDevice that isnt 
    // a default device
    int QueryAuxiliaryDevices(int * IDs);


    // Maximum number of supported. Will always be at least NumDefaultDevices in count
    int MaxDevices();

    // Sets the focus on which to query input from.
    // On multi-display systems, such as a desktop environment,
    // input is based around the context of the focused display.
    // On some implementations, such a distinction (and as a result, this function)  is not necessary
    // may not be necessary, but this will almost 
    // always be necessary for modern environments.
    // Passing in nullptr should disable input until a 
    // valid display is given.
    void SetFocus(Display *);

    // Returns the current focus. The default is nullptr;
    Display * GetFocus();
    
    
    
    // Versioning
    std::string Name() {return "AndroidBridge: Input";}
    std::string Version() {return "1.0";}
    bool Valid(){return true;}
    


  private:
    AndroidBridge_Input_Data * ABI;

};

}

#endif
#endif
