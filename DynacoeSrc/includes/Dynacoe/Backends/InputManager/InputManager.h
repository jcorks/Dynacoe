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

#include <Dynacoe/Backends/Backend.h>
#include <cstring>


namespace Dynacoe {
class Display;
class InputDevice;


/* Interacts with device drivers to retrieve the requested input information. */

/// \defgroup All possible device inputs
/// @{

///  Enum containing all valid device inputs recognized by the engine
/// It's important to note that this is only the list of standard inputs, 
/// and that systems will be able to generate events outside of this 
/// list.
enum UserInput {
    NotAnInput,
    Key_0, ///< 0
    Key_1, ///< 1
    Key_2, ///< 2
    Key_3, ///< 3
    Key_4, ///< 4
    Key_5, ///< 5
    Key_6, ///< 6
    Key_7, ///< 7
    Key_8, ///< 8
    Key_9, ///< 9
    Key_a, ///< a
    Key_b, ///< b
    Key_c, ///< c
    Key_d, ///< d
    Key_e, ///< e
    Key_f, ///< f
    Key_g, ///< g
    Key_h, ///< h
    Key_i, ///< i
    Key_j, ///< j
    Key_k, ///< k
    Key_l, ///< l
    Key_m, ///< m
    Key_n, ///< n
    Key_o, ///< o
    Key_p, ///< p
    Key_q, ///< q
    Key_r, ///< r
    Key_s, ///< s
    Key_t, ///< t
    Key_u, ///< u
    Key_v, ///< v
    Key_w, ///< w
    Key_x, ///< x
    Key_y, ///< y
    Key_z, ///< z
    Key_lshift, ///< Left shift key
    Key_rshift, ///< Right shift key
    Key_lctrl,  ///< Left control key
    Key_rctrl,  ///< Right control key
    Key_lalt,   ///< Left alt key
    Key_ralt,   ///< Right alt key
    Key_tab,    ///< Tab
    Key_F1,     ///< F1
    Key_F2,     ///< F2
    Key_F3,     ///< F3
    Key_F4,     ///< F4
    Key_F5,     ///< F5
    Key_F6,     ///< F6
    Key_F7,     ///< F7
    Key_F8,     ///< F8
    Key_F9,     ///< F9
    Key_F10,    ///< F10
    Key_F11,    ///< F11
    Key_F12,    ///< F12
    Key_up,     ///< Up arrow
    Key_down,   ///< Down arrow
    Key_left,   ///< Left arrow
    Key_right,  ///< Right arrow
    Key_minus,  ///< -
    Key_equal,  ///< = 
    Key_backspace,  ///< Backspace
    Key_grave,  ///< `
    Key_esc,    ///< Escape
    Key_home,   ///< Home key
    Key_pageUp, ///< Page up key
    Key_pageDown,  ///< Page down key
    Key_end,    ///< End key
    Key_backslash, ///< '\'
    Key_lbracket, ///< [
    Key_rbracket, ///< ]
    Key_semicolon, ///< ;
    Key_apostrophe, ///< '
    Key_frontslash, ///< /
    Key_enter, ///< Enter
    Key_delete, ///< Delete
    Key_numpad0, ///< Numpad 0
    Key_numpad1, ///< Numpad 1
    Key_numpad2, ///< Numpad 2
    Key_numpad3, ///< Numpad 3
    Key_numpad4, ///< Numpad 4
    Key_numpad5, ///< Numpad 5
    Key_numpad6, ///< Numpad 6
    Key_numpad7, ///< Numpad 7
    Key_numpad8, ///< Numpad 8
    Key_numpad9, ///< Numpad 9
    Key_prtscr, ///< Print screen button
    Key_lsuper, ///< Left Super key (Windows key)
    Key_rsuper, ///< Right Super key (Windows key)
    Key_space,  ///< Space
    Key_insert, ///< Insert key
    Key_comma, ///< ,
    Key_period, ///< .

    Pointer_0, ///< Left click
    Pointer_1, ///< Right click
    Pointer_2, ///< Middle click

    Pointer_X, ///< Horizontal axis. Usually for the X axis of the pointer
    Pointer_Y, ///< Horizontal axis. Usually for the X axis of the pointer
    Pointer_Wheel, ///< Mouse wheel.

    Pad_a,     ///< Button 0
    Pad_b,     ///< Button 1
    Pad_c,     ///< Button 2
    Pad_x,     ///< Button 3
    Pad_y,     ///< Button 4
    Pad_r,     ///< Button 5
    Pad_l,     ///< Button 6
    Pad_r2,    ///< Button 7
    Pad_l2,    ///< Button 8
    Pad_r3,    ///< Button 9
    Pad_l3,    ///< Button 10
    Pad_start,    ///< Button 11
    Pad_select,///< Button 12
    Pad_b13,///< Button 13
    Pad_b14,///< Button 14
    Pad_b15,///< Button 15
    Pad_b16,///< Button 16
    Pad_b17,///< Button 17
    Pad_b18,///< Button 18
    Pad_b19,///< Button 19
    Pad_b20,///< Button 20
    Pad_b21,///< Button 21
    Pad_b22,///< Button 22
    Pad_b23,///< Button 23
    Pad_b24,///< Button 24
    Pad_b25,///< Button 25
    Pad_b26,///< Button 26
    Pad_b27,///< Button 27
    Pad_b28,///< Button 28
    Pad_b29,///< Button 29
    Pad_b30,///< Button 30
    Pad_b31,///< Button 31
    Pad_b32,///< Button 32

    Pad_axisX, ///< X button
    Pad_axisY, ///< Y button
    Pad_axisZ, ///< Z button
    Pad_axisX2,///< X2 button 
    Pad_axisY2,///< Y2 button
    Pad_axisZ2,///< Z2 button
    Pad_axisX3,///< X3 button
    Pad_axisY3,///< Y3 button
    Pad_axisZ3,///< Z3 button
    Pad_axisX4,///< X4 button
    Pad_axisY4,///< Y4 button
    Pad_axisZ4,///< Z4 button
    Pad_axisX5,///< X4 button
    Pad_axisY5,///< Y4 button
    Pad_axisZ5,///< Z4 button
    Pad_axisX6,///< X4 button
    Pad_axisY6,///< Y4 button
    Pad_axisZ6,///< Z4 button

    Pad_axisR,    
    Pad_axisL,    
    Pad_axisR2,    
    Pad_axisL2,    
    Pad_axisR3,    
    Pad_axisL3,    
    Pad_axisR4,    
    Pad_axisL4,    

    Pad_options,
    
    Count
};




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
