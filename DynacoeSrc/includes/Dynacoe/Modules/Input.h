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
#include <Dynacoe/Modules/Module.h>
#include <string>
#include <map>
#include <set>




namespace Dynacoe {

/// \brief Identifies an input pad.
///
using PadID = Dynacoe::LookupID;

/// \brief Callback functor base class.
/// To implement a callback signal, create a class that
/// inherits from InputCallback
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

/// \brief Singleton class for querying input devices
///
class Input : public Module {
  public:       


 
    /// \name IsPressed()
    /// Returns whether or not the input was pressed.
    ///    
    /// \{
    static bool IsPressed(Keyboard);
    static bool IsPressed(MouseButtons);
    static bool IsPressed(PadID, PadButtons);
    static bool IsPressed(const std::string &);
    ///\}

    /// \name GetState()
    /// Returns the current state of the device button, where true means the 
    /// the button is activated and false returns if otherwise.
    /// 
    /// \{    
    static bool GetState(Keyboard);
    static bool GetState(MouseButtons);
    static bool GetState(PadID, PadButtons);
    static bool GetState(const std::string &);
    /// \}

    /// \brief Returns the last Ascii code from keyboard.
    ///
    /// Once this is called, it will return 0 unless another
    /// unicode key is pressed on the keyboard input
    static int GetLastUnicode();

    /// \name  IsHeld
    /// Returns whether or not the input is currently being held.
    ///
    /// \{
    static bool IsHeld(Keyboard);
    static bool IsHeld(MouseButtons);
    static bool IsHeld(PadID, PadButtons);
    static bool IsHeld(const std::string &);
    /// \}

    // \name  IsReleased
    // Returns whether or not the input was released this update.
    //
    static bool IsReleased(Keyboard);
    static bool IsReleased(MouseButtons);
    static bool IsReleased(PadID, PadButtons);
    static bool IsReleased(const std::string &);

    /// \brief Retrieves the a list of all valid Gamepads.
    ///
    static std::vector<PadID> QueryPads();

    /// \name MapInput()
    /// Maps a data value to an input. The value may be queried to the specified input.
    /// \{    
    static void MapInput(const std::string & id, Keyboard);
    static void MapInput(const std::string & id, MouseButtons);
    static void MapInput(const std::string & id, PadID, PadButtons);
    ///\}

    /// \brief Disassociates a string with its corresponding InputID if any
    ///
    static void UnmapInput(const std::string & id);

    /// \brief Maps an InputCallback to the specifed key.
    /// 
    /// Only one InputCallback may be associated 
    /// with a given InputID.
    static void AddListener(ButtonListener *, Keyboard);
    static void AddListener(ButtonListener *, MouseButtons);
    static void AddListener(ButtonListener *, PadID, PadButtons);
    static void AddListener(ButtonListener *, const std::string &);
    

    /// \brief Disassociates the corresponding InputCallback if any.
    ///
    static void RemoveListener(ButtonListener *);


    /// \name Mouse Convenience Functions
    /// \{

    /// \brief Current X position of the Mouse.
    ///
    static int MouseX();

    /// \brief Current Y position of the Mouse.
    static int MouseY();

    /// \brief Returns the change in X position since last update.
    ///
    static int MouseXDelta();

    /// \brief Returns the change in Y position since the last update.    
    static int MouseYDelta();

    /// \brief Returns the current state of the mouse wheel.
    ///
    /// If the wheel is neutral, 0 is returned. If scrolling up, 1 is returned,
    /// and if scrolling down, -1.
    static int MouseWheel();
    ///\}


    /// \brief Returns the axis tilt amount.
    /// The range is from -1 to 1
    ///
    static float GetPadAxis(PadID, PadAxes);


    /// \brief Prevents the updating of Input until the time expires
    ///
    static bool LockInput(float sec);


    
    /// \brief Update the state of input. This is done for you.
    ///
    static void Update();

  private:

    
    static void initBase();
    static bool IsShiftMod();

    static void getUnicode();
    
    struct InputState {
        
        
        InputDevice ** devices;
        int numDevices;
    };


    
    static InputManager * manager;
    static int lastUnicode;


    // Aggregate button state
    struct ButtonList {
        std::vector<Keyboard> keys;
        std::vector<MouseButtons> mouseButtons;
        std::vector<std::pair<PadID, std::vector<PadButtons>>> padButtons;

        void addButton(Keyboard);
        void addButton(MouseButtons);
        void addButton(PadID, PadButtons);
        bool GetState();
        bool IsPressed();
        bool IsHeld();
        bool IsReleased();
    };
    
    //static std::map<std::string, ButtonList*> stringMap;
    static std::map<std::string, Keyboard> stringMapKeyboard;
    static std::map<std::string, MouseButtons> stringMapMouse;
    static std::map<std::string, std::pair<PadID, PadButtons>> stringMapPad;
    static std::vector<ButtonList*> buttonLists;






    static bool inputLocked;

    static InputState thisState;
    static InputState prevState;






    
    
    
  public:
    std::string GetName() {return "Input";}
    void Init(); void InitAfter(); void RunBefore(); void RunAfter(); void DrawBefore(); void DrawAfter();
    Backend * GetBackend();
};
}


#endif
