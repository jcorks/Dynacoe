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
#if (defined DC_BACKENDS_GAINPUTX11 || defined DC_BACKENDS_GAINPUTWIN32)

#ifndef DC_GAINPUT_MULTI_INCLUDED
#define DC_GAINPUT_MULTI_INCLUDED

#include <gainput/gainput.h>
#include <Dynacoe/Backends/InputManager/InputDevice.h>
#include <Dynacoe/Backends/InputManager/InputManager.h>
#include <vector>

/* Gainput backend for Dynacoe


   Gainput License:


Copyright (c) 2013 Johannes Kuhlmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons 
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

namespace Dynacoe {
class Display;
class GainputManager : public InputManager {
  public:
    GainputManager();

    // standard interface
    bool IsSupported(InputType);
    bool HandleEvents();
    InputDevice * QueryDevice(int ID);
    InputDevice * QueryDevice(DefaultDeviceSlots);
    int QueryAuxiliaryDevices(int * IDs);
    int MaxDevices();

    std::string Name();
    std::string Version();
    bool Valid();

    void SetFocus(Display*);
    Display * GetFocus();

    std::string RunCommand(const std::string & command, uint8_t * data = nullptr);

  private:


    gainput::InputManager devMan;


    /* a gainput device is qualified by a dynacoe device and
       a gainput device manager */
    struct GainputDevice {
        GainputDevice(gainput::DeviceId _id, InputDevice * _dev, 
                      gainput::InputMap * _buttonMap, gainput::InputMap * _axesMap):
            id(_id),
            dev(_dev),
            buttonMap(_buttonMap),
            axesMap(_axesMap){}

        // updates the device 
        void update();

        gainput::DeviceId id;
        InputDevice * dev;
        gainput::InputMap * buttonMap;
        gainput::InputMap * axesMap;
    };




    std::vector<GainputDevice*> devices;
    

    void mapKeys();
    void clearInput();
    void updateInternalStates();
    void updateDevice(InputDevice);

    float mouseX, mouseY; //IDs 0, 1
    float touchX, touchY; //IDs 2, 3

    int mouseAxesWheelDown = 100;
    
    Display * displayHandle;

    

};
}


#endif
#endif
