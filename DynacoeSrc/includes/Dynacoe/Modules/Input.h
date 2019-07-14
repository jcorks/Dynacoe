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

#ifndef H_DC_INPUT
#define H_DC_INPUT  

#include <Dynacoe/Backends/InputManager/InputTypes.h>
#include <Dynacoe/Backends/InputManager/InputDevice.h>
#include <Dynacoe/Backends/InputManager/InputManager.h>
#include <Dynacoe/Util/Table.h>
#include <string>
#include <map>
#include <set>




namespace Dynacoe {

/// \brief Identifies an input pad.
///
using PadID = Dynacoe::LookupID;

/// \brief Callback functor base class.
/// To implement a callback signal, create a class that
/// inherits from this and give it to Input::AddListener()
class ButtonListener {
  public:
    /// \brief Function called upon pressing of the button.
    ///
    virtual void OnPress(){};
    
    /// \brief Function called upon holding the button.
    virtual void OnHold(){};

    /// \brief Function called upon releasing the button.
    ///
    virtual void OnRelease(){};
};

/// \brief Callbacck functor for unicode values from a user's keyboard.
/// On a US keyboard, the following special key signals exist:
/// 17  -> left
/// 18  -> up
/// 19  -> right 
/// 20  -> down
///
/// For all keyboards, the following special characters exist:
/// 32  -> newline 
/// 8   -> backspace
///
///
class UnicodeListener {
  public:    
    /// \brief An incoming unicode value.
    ///
    virtual void OnNewUnicode(int unicodeValue){};

    /// \brief A unicode value is being requested to be 
    /// processed multiple times in a row.
    virtual void OnRepeatUnicode(int unicodeValue){};
};

/// \brief Singleton class for querying input devices
///
namespace Input {

 
    /// \name IsPressed()
    /// Returns whether or not the input was pressed.
    ///    
    /// \{
    bool IsPressed(Keyboard);
    bool IsPressed(MouseButtons);
    bool IsPressed(PadID, PadButtons);
    bool IsPressed(const std::string &);
    ///\}

    /// \name GetState()
    /// Returns the current state of the device button, where true means the 
    /// the button is activated and false returns if otherwise.
    /// 
    /// \{    
    bool GetState(Keyboard);
    bool GetState(MouseButtons);
    bool GetState(PadID, PadButtons);
    bool GetState(const std::string &);
    /// \}

    /// \brief Returns the last Ascii code from keyboard.
    ///
    /// Once this is called, it will return 0 unless another
    /// unicode key is pressed on the keyboard input
    int GetLastUnicode();

    /// \brief Adds a UnicodeListener object to receive events when 
    /// incoming unicode characters are processed.
    ///
    void AddUnicodeListener(UnicodeListener *, bool pausable=true);

    /// \brief Removes an added unicode listener.
    ///
    void RemoveUnicodeListener(UnicodeListener *);

    /// \name  IsHeld
    /// Returns whether or not the input is currently being held.
    ///
    /// \{
    bool IsHeld(Keyboard);
    bool IsHeld(MouseButtons);
    bool IsHeld(PadID, PadButtons);
    bool IsHeld(const std::string &);
    /// \}

    // \name  IsReleased
    // Returns whether or not the input was released this update.
    //
    bool IsReleased(Keyboard);
    bool IsReleased(MouseButtons);
    bool IsReleased(PadID, PadButtons);
    bool IsReleased(const std::string &);

    /// \brief Retrieves the a list of all valid Gamepads.
    ///
    std::vector<PadID> QueryPads();

    /// \name MapInput()
    /// Maps a data value to an input. The value may be queried to the specified input.
    /// \{    
    void MapInput(const std::string & id, Keyboard);
    void MapInput(const std::string & id, MouseButtons);
    void MapInput(const std::string & id, PadID, PadButtons);
    ///\}

    /// \brief Disassociates a string with its corresponding InputID if any
    ///
    void UnmapInput(const std::string & id);

    /// \brief Maps an InputCallback to the specifed key.
    /// 
    /// Only one InputCallback may be associated 
    /// with a given InputID.
    void AddListener(ButtonListener *, Keyboard);
    void AddListener(ButtonListener *, MouseButtons);
    void AddListener(ButtonListener *, PadID, PadButtons);
    void AddListener(ButtonListener *, const std::string &);
    

    /// \brief Disassociates the corresponding InputCallback if any.
    ///
    void RemoveListener(ButtonListener *);


    /// \name Mouse Convenience Functions
    /// \{

    /// \brief Current X position of the Mouse.
    ///
    int MouseX();

    /// \brief Current Y position of the Mouse.
    int MouseY();

    /// \brief Returns the change in X position since last update.
    ///
    int MouseXDelta();

    /// \brief Returns the change in Y position since the last update.    
    int MouseYDelta();

    /// \brief Returns the current state of the mouse wheel.
    ///
    /// If the wheel is neutral, 0 is returned. If scrolling up, 1 is returned,
    /// and if scrolling down, -1.
    int MouseWheel();
    ///\}


    /// \brief Returns the axis tilt amount.
    /// The range is from -1 to 1
    ///
    float GetPadAxis(PadID, PadAxes);


    /// \brief Prevents the updating of Input until the time expires
    ///
    bool LockInput(float sec);


    
    /// \brief Update the state of input. This is done for you.
    ///
    void Update();

    /// \brief Attempts to open a virtual keyboard that belongs to the system
    /// if any is available.
    ///
    void ShowVirtualKeyboard(bool);
    
    
    /// \brief Returns the system-dependent object that handles input requests.
    /// 
    InputManager * GetManager();


    void RunBefore();
    void Init();
    
};
}


#endif
