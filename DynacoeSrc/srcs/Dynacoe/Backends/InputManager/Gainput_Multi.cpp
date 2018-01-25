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

#include <Dynacoe/Backends/InputManager/Gainput_Multi.h>
#include <Dynacoe/Backends/Display/Display.h>
#include <stack>
#include <cstdlib>
#include <cfloat>


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


using namespace std;




#ifdef DC_BACKENDS_GAINPUTX11
#include <X11/X.h>
#include <X11/Xlib.h>
typedef Display X11Display; 
#endif

#ifdef DC_BACKENDS_GAINPUTWIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif



bool Dynacoe::GainputManager::IsSupported(InputType){return true;}

bool Dynacoe::GainputManager::HandleEvents() {




    // Need the window's display to deal with input
    
    if (!displayHandle) {return false;}



    devMan.SetDisplaySize(
        displayHandle->Width(),
        displayHandle->Height()
    );
        

    

    #ifdef DC_BACKENDS_GAINPUTX11

        stack<XEvent> configureEvents;
        X11Display * disp;
        devMan.Update();
        //disp = (X11Display*)displayHandle->GetSystemHandle();

        //if (!disp) return false;
        

        /*
        XEvent event;
        while(XPending(disp)) {
            
            XNextEvent(disp, &event);
            if (event.type == ConfigureNotify)
                configureEvents.push(event);
            devMan.HandleEvent(event);
        }

        // We want to put back configure events for resizing 
        while(configureEvents.size()) {
            XPutBackEvent(disp, &configureEvents.top());
            configureEvents.pop();
        }
        */
        std::vector<XEvent> * ev = (std::vector<XEvent>*)displayHandle->GetLastSystemEvent();
        for(uint32_t i = 0; i < ev->size(); ++i) {
            devMan.HandleEvent((*ev)[i]);            
        }



    #elif defined DC_BACKENDS_GAINPUTWIN32

        MSG msg;
        devMan.Update();
        std::vector<MSG> * ev = (std::vector<MSG>*)displayHandle->GetLastSystemEvent();
        for(uint32_t i = 0; i < ev->size(); ++i) {
            devMan.HandleMessage((*ev)[i]);            
        }
    

    #endif
    
    updateInternalStates();    

    return true;
}



void Dynacoe::GainputManager::clearInput() {
}




int Dynacoe::GainputManager::MaxDevices() {
    return (int)DefaultDeviceSlots::NumDefaultDevices;
}

Dynacoe::InputDevice * Dynacoe::GainputManager::QueryDevice(int i) {
    if (i < 0 || i >= (int)devices.size()) return nullptr;

    return devices[i]->dev;
}

Dynacoe::InputDevice * Dynacoe::GainputManager::QueryDevice(DefaultDeviceSlots s) {
    return QueryDevice((int)s);
}


int Dynacoe::GainputManager::QueryAuxiliaryDevices(int * IDs) {
    return -1;
}


Dynacoe::GainputManager::GainputManager() {

    // keyboard
    devices.push_back(new GainputDevice(
        devMan.CreateDevice<gainput::InputDeviceKeyboard>(),
        new InputDevice((int)Dynacoe::Keyboard::NumButtons, 0),
        new gainput::InputMap(devMan),
        new gainput::InputMap(devMan)

    ));

    // mouse
    
    devices.push_back(new GainputDevice(
        devMan.CreateDevice<gainput::InputDeviceMouse>(),
        new InputDevice((int)MouseButtons::NumButtons, 
                        (int)MouseAxes::NumAxes),
        new gainput::InputMap(devMan),
        new gainput::InputMap(devMan)
    ));
    


    // Uhhh

    mapKeys();
    displayHandle = nullptr;

}

void Dynacoe::GainputManager::updateInternalStates() {

    

    for(int i = 0; i < devices.size(); ++i) {
        devices[i]->update();
    }



    // correct mouse states
    if (devices[1]) {
        devices[1]->dev->axes[(int)MouseAxes::X] *= displayHandle->Width();
        devices[1]->dev->axes[(int)MouseAxes::Y] *= displayHandle->Height();
        devices[1]->dev->axes[(int)MouseAxes::Wheel] += (devices[1]->axesMap->GetBool(mouseAxesWheelDown))?-1.f:0.f;
    }
}

static void map___(gainput::InputMap * mp, int k, gainput::DeviceId i, gainput::DeviceButtonId b) {
    int res = mp->MapBool(k, i, b);
    //if (!res) {
    //    std::cout << "Could not map" << k << "\n";
    //}
}

/* ALL mapped keys are done so from 0 to numbuttons/axes */
void Dynacoe::GainputManager::mapKeys() {


    /* base keyboard! */

    gainput::InputMap * keyboardMap = devices[0]->buttonMap;
    gainput::DeviceId & keyboardDev = devices[0]->id;



    for(int i = 0; i < 1+ ((int) Dynacoe::Keyboard::Key_9) - (int)Dynacoe::Keyboard::Key_0; ++i) {
        map___(keyboardMap, (int)Dynacoe::Keyboard::Key_0+i, (int)keyboardDev, gainput::Key0+i);
    }
    for(int i = 0; i < 1+ ((int) Dynacoe::Keyboard::Key_9) - (int)Dynacoe::Keyboard::Key_0; ++i) {
        map___(keyboardMap, (int)Dynacoe::Keyboard::Key_numpad0+i, (int)keyboardDev, gainput::Key0+i);
    }
    for(int i = 0; i < 1+ ((int) Dynacoe::Keyboard::Key_z) - (int)Dynacoe::Keyboard::Key_a; ++i) {
        map___(keyboardMap, (int)Dynacoe::Keyboard::Key_a+i, keyboardDev, gainput::KeyA+i);
    }
    for(int i = 0; i < 1+ ((int) Dynacoe::Keyboard::Key_F12) - (int)Dynacoe::Keyboard::Key_F1; ++i) {
        map___(keyboardMap, (int)Dynacoe::Keyboard::Key_F1+i, keyboardDev, gainput::KeyF1+i);
    } 
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_lshift, keyboardDev, gainput::KeyShiftL);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_rshift, keyboardDev, gainput::KeyShiftR);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_lctrl, keyboardDev, gainput::KeyCtrlL);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_rctrl, keyboardDev, gainput::KeyCtrlR);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_lalt, keyboardDev, gainput::KeyAltL);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_ralt, keyboardDev, gainput::KeyAltR);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_tab, keyboardDev, gainput::KeyTab);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_up, keyboardDev, gainput::KeyUp);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_down, keyboardDev, gainput::KeyDown);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_left, keyboardDev, gainput::KeyLeft);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_right, keyboardDev, gainput::KeyRight);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_minus, keyboardDev, gainput::KeyMinus);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_equal, keyboardDev, gainput::KeyEqual);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_backspace, keyboardDev, gainput::KeyBackSpace);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_grave, keyboardDev, gainput::KeyGrave);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_esc, keyboardDev, gainput::KeyEscape);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_home, keyboardDev, gainput::KeyHome);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_pageUp, keyboardDev, gainput::KeyPageUp);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_pageDown, keyboardDev, gainput::KeyPageDown);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_end, keyboardDev, gainput::KeyEnd);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_backslash, keyboardDev, gainput::KeyBackslash);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_lbracket, keyboardDev, gainput::KeyBracketLeft);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_rbracket, keyboardDev, gainput::KeyBracketRight);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_semicolon, keyboardDev, gainput::KeySemicolon);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_apostrophe, keyboardDev, gainput::KeyApostrophe);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_frontslash, keyboardDev, gainput::KeySlash);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_enter, keyboardDev, gainput::KeyReturn);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_delete, keyboardDev, gainput::KeyDelete);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_prtscr, keyboardDev, gainput::KeyPrint);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_lsuper, keyboardDev, gainput::KeySuperL);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_rsuper, keyboardDev, gainput::KeySuperR);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_space, keyboardDev, gainput::KeySpace);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_insert, keyboardDev, gainput::KeyInsert);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_period, keyboardDev, gainput::KeyPeriod);
    map___(keyboardMap, (int)Dynacoe::Keyboard::Key_comma, keyboardDev, gainput::KeyComma);

    


    /* base mouse! */

    gainput::InputMap * mouseMap = devices[1]->axesMap;
    gainput::DeviceId & mouseDev = devices[1]->id;


    mouseMap->MapFloat((int)MouseAxes::X, mouseDev, gainput::MouseAxisX, 0, 1);
    mouseMap->MapFloat((int)MouseAxes::Y, mouseDev, gainput::MouseAxisY, 0, 1);
    mouseMap->MapFloat((int)MouseAxes::Wheel, mouseDev,  gainput::MouseButtonWheelUp);
    mouseMap->MapBool ((int)mouseAxesWheelDown, mouseDev, gainput::MouseButtonWheelDown);

    mouseMap = devices[1]->buttonMap;
    mouseMap->MapBool((int)MouseButtons::Left, mouseDev,  gainput::MouseButtonLeft);
    mouseMap->MapBool((int)MouseButtons::Right, mouseDev, gainput::MouseButtonRight);
    mouseMap->MapBool((int)MouseButtons::Middle, mouseDev, gainput::MouseButtonMiddle);
    


    


    
}

void Dynacoe::GainputManager::GainputDevice::update() {
    for(int i = 0; i < dev->numButtons; ++i) {
        dev->buttons[i] = buttonMap->GetBool(i);  
    }
    for(int i = 0; i < dev->numAxes; ++i) { // (pow(10, 10) / 5.f)*
        dev->axes[i] = axesMap->GetFloat(i);
    }
}

void Dynacoe::GainputManager::SetFocus(Dynacoe::Display * d) {
    displayHandle = d;
}

Dynacoe::Display * Dynacoe::GainputManager::GetFocus() {
    return displayHandle;
}


bool Dynacoe::GainputManager::Valid(){return true; }
std::string Dynacoe::GainputManager::Name() { return "Gainput (c) Johannes Kuhlmann, 2013"; }
std::string Dynacoe::GainputManager::Version() { return std::string("v1.0 (Gainput ver.: ") 
                                             //+ std::string(gainput::GetLibVersionString()) 
                                             + std::string(")"); }


std::string Dynacoe::GainputManager::RunCommand(const std::string &, uint8_t *) {
    return "";
}


#endif

