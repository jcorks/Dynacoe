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


#ifndef H_DC_INPUT_TYPES
#define H_DC_INPUT_TYPES



namespace Dynacoe {


/** 
 * \brief Enum containing all valid keyboard key inputs.
 * 
 * Each individual enum refers to the standard US keyboard keys
 */
enum class Keyboard {
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
    NumButtons ///< Number of buttons
};

/// \brief Mouse Button inputs.
///
enum class MouseButtons {
    Left, ///< Left click
    Right, ///< Right click
    Middle, ///< Middle click
    NumButtons 
};

/// \brief Axes of movements for the mouse.
///
enum class MouseAxes {
    X, ///< Horizontal axis
    Y, ///< Vertical axis
    Wheel, ///< Mouse wheel.
    NumAxes
};

/// \brief Valid inputs for an arbitrary input pad.
///
/// Only the first 32 buttons are registered for a pad.
enum class PadButtons {
    Pad_b0, ///< Button 0
    Pad_b1, ///< Button 1
    Pad_b2, ///< Button 2
    Pad_b3, ///< Button 3
    Pad_b4,///< Button 4
    Pad_b5,///< Button 5
    Pad_b6,///< Button 6
    Pad_b7,///< Button 7
    Pad_b8,///< Button 8
    Pad_b9,///< Button 9
    Pad_b10,///< Button 10
    Pad_b11,    ///< Button 11
    Pad_b12,///< Button 12
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

    NumButtons
};

/// \brief All the valid axes that an inputad can enter.
///
enum class PadAxes {


    Pad_x, ///< X button
    Pad_y, ///< Y button
    Pad_z, ///< Z button
    Pad_x2,///< X2 button 
    Pad_y2,///< Y2 button
    Pad_z2,///< Z2 button
    Pad_x3,///< X3 button
    Pad_y3,///< Y3 button
    Pad_z3,///< Z3 button
    Pad_x4,///< X4 button
    Pad_y4,///< Y4 button
    Pad_z4,///< Z4 button
    
    NumAxes
};



/// \brief All valid pointers.
///
enum class Pointers {
    Mouse, ///< The mouse pointer.
    Touch, ///< For touchscreens, this is the touch position.

    NumPointers

};
}



#endif
