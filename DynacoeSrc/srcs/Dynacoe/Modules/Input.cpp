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


#include <Dynacoe/Modules/Input.h>
#include <Dynacoe/Backends/InputManager/InputDevice.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Modules/ViewManager.h>
#include <algorithm>
#include <cstring>


using namespace Dynacoe;
using namespace std;

InputManager * Input::manager;

std::map<std::string, Keyboard> Input::stringMapKeyboard;
std::map<std::string, MouseButtons> Input::stringMapMouse;
std::map<std::string, std::pair<PadID, PadButtons>> Input::stringMapPad;
std::vector<Input::ButtonList *> Input::buttonLists;
static std::map<ButtonListener*, Keyboard> keyCallbackMap;
static std::map<ButtonListener*, MouseButtons> mouseCallbackMap;
static std::map<ButtonListener*, std::pair<PadID, PadButtons>> padCallbackMap;
static std::map<ButtonListener*, std::string> strCallbackMap;


bool Input::inputLocked;
int Input::lastUnicode;

Input::InputState Input::thisState;
Input::InputState Input::prevState;


static int MouseXDeviceToWorld2D(int);
static int MouseYDeviceToWorld2D(int);
static ViewID focusID;



void Input::Init() {
    manager = (InputManager *)Backend::CreateDefaultInputManager();
    inputLocked = false;

    thisState.devices = new InputDevice * [(int)InputManager::DefaultDeviceSlots::NumDefaultDevices];
    thisState.numDevices = (int)InputManager::DefaultDeviceSlots::NumDefaultDevices;

    prevState.devices = new InputDevice * [(int)InputManager::DefaultDeviceSlots::NumDefaultDevices];
    prevState.numDevices = (int)InputManager::DefaultDeviceSlots::NumDefaultDevices;


    /* initialize devices */
    for(int i = 0; i < thisState.numDevices; ++i) {
        thisState.devices[i] = manager->QueryDevice(i);
        prevState.devices[i] = (thisState.devices[i]?thisState.devices[i]->GetCopy():nullptr);
    }


}

Backend * Input::GetBackend() {
    return manager;
}

void Input::InitAfter(){}



void Input::RunBefore() {

    if (inputLocked) return;


    for(int i = 0; i < thisState.numDevices; ++i) {
        if (thisState.devices[i])
            thisState.devices[i]->CopyInto(prevState.devices[i]); //TODO
    }


    // Set the focus to the display that has input focus (which
    // isnt necessarily the main display);
    std::vector<ViewID> dpys = ViewManager::ListViews();
    Display * focus = nullptr;
    for(uint32_t i = 0; i < dpys.size(); ++i) {
        focus = ViewManager::Get(dpys[i]);
        if (focus && focus->HasInputFocus()) {
            focusID = dpys[i];
            break;
        }
        focusID = ViewID();
        focus = nullptr;
    }
    manager->SetFocus(focus);


    bool updated = manager->HandleEvents();
    
    if (keyCallbackMap.size()) {
        for(auto i = keyCallbackMap.begin(); i != keyCallbackMap.end(); ++i) {
            if (Input::IsPressed(i->second))
                i->first->OnPress();
            if (Input::IsHeld(i->second))
                i->first->OnHold();
            if (Input::IsReleased(i->second))
                i->first->OnRelease();
        }
    }

    if (mouseCallbackMap.size()) {
        for(auto i = mouseCallbackMap.begin(); i != mouseCallbackMap.end(); ++i) {
            if (Input::IsPressed(i->second))
                i->first->OnPress();
            if (Input::IsHeld(i->second))
                i->first->OnHold();
            if (Input::IsReleased(i->second))
                i->first->OnRelease();
        }
    }
    
    /*
    // TODO: for pad support
    if (padCallbackMap.size()) {
        for(auto i = padCallbackMap.begin(); i != padCallbackMap.end(); ++i) {
            if (Input::IsPressed(i->second.first, i->second.second))
                i->first->OnPress();
            if (Input::IsHeld(i->second.first, i->second.second))
                i->first->OnHold();
            if (Input::IsReleased(i->second.first, i->second.second))
                i->first->OnRelease();
        }
    }
    */

    if (strCallbackMap.size()) {
        for(auto i = strCallbackMap.begin(); i != strCallbackMap.end(); ++i) {
            if (Input::IsPressed(i->second))
                i->first->OnPress();
            if (Input::IsHeld(i->second))
                i->first->OnHold();
            if (Input::IsReleased(i->second))
                i->first->OnRelease();
        }
    }

    if (updated) {
        getUnicode();
    }
}

void Input::RunAfter() {}
void Input::DrawBefore(){}
void Input::DrawAfter(){}



bool Input::IsPressed(Keyboard k) {
    return (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)k] &&
             thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)k]);
}

bool Input::IsPressed(MouseButtons k) {
    return (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->buttons[(int)k] &&
             thisState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->buttons[(int)k]);
}




bool Input::IsPressed(const std::string & s) {
    auto keyboardIter = stringMapKeyboard.find(s);
    if (keyboardIter != stringMapKeyboard.end() &&
        IsPressed(keyboardIter->second)) return true;

    auto mouseIter = stringMapMouse.find(s);
    if (mouseIter != stringMapMouse.end() &&
        IsPressed(mouseIter->second)) return true;

    /*
    auto padIter = stringMapPad.find(s);
    if (padIter != stringMapPad.end() &&
        IsPressed(padIter->second.first, padIter->second.second)) return true;
    */

    return false;
}



bool Input::IsReleased(Keyboard k) {
    return ( prevState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)k] &&
            !thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)k]);
}

bool Input::IsReleased(MouseButtons k) {
    return ( prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->buttons[(int)k] &&
            !thisState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->buttons[(int)k]);
}




bool Input::IsReleased(const std::string & s) {
    auto keyboardIter = stringMapKeyboard.find(s);
    if (keyboardIter != stringMapKeyboard.end() &&
        IsReleased(keyboardIter->second)) return true;

    auto mouseIter = stringMapMouse.find(s);
    if (mouseIter != stringMapMouse.end() &&
        IsReleased(mouseIter->second)) return true;

    /*
    auto padIter = stringMapPad.find(s);
    if (padIter != stringMapPad.end() &&
        IsPressed(padIter->second.first, padIter->second.second)) return true;
    */

    return false;
}




bool Input::GetState(Keyboard k) {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]) return false;
    return (
             thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)k]);
}

bool Input::GetState(MouseButtons k) {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]) return false;
    return (
             thisState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->buttons[(int)k]);
}

bool Input::GetState(const std::string & s) {
    auto keyboardIter = stringMapKeyboard.find(s);
    if (keyboardIter != stringMapKeyboard.end() &&
        GetState(keyboardIter->second)) return true;

    auto mouseIter = stringMapMouse.find(s);
    if (mouseIter != stringMapMouse.end() &&
        GetState(mouseIter->second)) return true;

    /*
    auto padIter = stringMapPad.find(s);
    if (padIter != stringMapPad.end() &&
        GetState(padIter->second.first, padIter->second.second)) return true;
    */

    return false;
}





bool Input::IsHeld(Keyboard k) {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]) return false;
    return (prevState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)k] &&
            thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)k]);
}

bool Input::IsHeld(MouseButtons k) {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]) return false;
    return (prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->buttons[(int)k] &&
            thisState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->buttons[(int)k]);
}

bool Input::IsHeld(const std::string & s) {
    auto keyboardIter = stringMapKeyboard.find(s);
    if (keyboardIter != stringMapKeyboard.end() &&
        IsHeld(keyboardIter->second)) return true;

    auto mouseIter = stringMapMouse.find(s);
    if (mouseIter != stringMapMouse.end() &&
        IsHeld(mouseIter->second)) return true;

    /*
    auto padIter = stringMapPad.find(s);
    if (padIter != stringMapPad.end() &&
        IsHeld(padIter->second.first, padIter->second.second)) return true;
    */

    return false;
}







int Input::MouseX() {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]) return 0;
    return
        MouseXDeviceToWorld2D(
        thisState.devices[
            (int)InputManager::DefaultDeviceSlots::Mouse
        ]->axes[
            (int)MouseAxes::X
        ]
    );
}

int Input::MouseY() {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]) return 0;
    return
        MouseYDeviceToWorld2D(
        thisState.devices[
            (int)InputManager::DefaultDeviceSlots::Mouse
        ]->axes[
            (int)MouseAxes::Y
        ]
    );
}

int Input::MouseXDelta() {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]) return false;
    return MouseXDeviceToWorld2D(thisState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->axes[(int)MouseAxes::X]) -
           MouseXDeviceToWorld2D(prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->axes[(int)MouseAxes::X]);


}

int Input::MouseYDelta() {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]) return false;
    return MouseYDeviceToWorld2D(thisState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->axes[(int)MouseAxes::Y]) -
           MouseYDeviceToWorld2D(prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->axes[(int)MouseAxes::Y]);
}

int Input::MouseWheel() {
    if (!prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]) return false;
    return thisState.devices[(int)InputManager::DefaultDeviceSlots::Mouse]->axes[(int)MouseAxes::Wheel];
}

int Input::GetLastUnicode() {
    int out =  lastUnicode;
    lastUnicode = 0;
    return out;
}





/* input mapping */


void Input::MapInput(const std::string & id, Keyboard key) {
    stringMapKeyboard[id] = key;
}

void Input::MapInput(const std::string & id, MouseButtons key) {
    stringMapMouse[id] = key;
}

void Input::MapInput(const std::string & id, PadID idpad, PadButtons key) {
    stringMapPad[id] = {idpad, key};
}














void Input::UnmapInput(const std::string & id) {
    stringMapKeyboard.erase(id);
    stringMapMouse.erase(id);
    stringMapPad.erase(id);
}



void Input::AddListener(ButtonListener * b, Keyboard i) {
    keyCallbackMap[b] = i;
}

void Input::AddListener(ButtonListener * b, MouseButtons i) {
    mouseCallbackMap[b] = i;
}

void Input::AddListener(ButtonListener * b, PadID id, PadButtons i) {
    padCallbackMap[b] = {id, i};
}

void Input::AddListener(ButtonListener * b, const std::string & i) {
    strCallbackMap[b] = i;
}


void Input::RemoveListener(ButtonListener * b) {
    padCallbackMap.erase(padCallbackMap.find(b));
    mouseCallbackMap.erase(mouseCallbackMap.find(b));
    keyCallbackMap.erase(keyCallbackMap.find(b));
}



// Private methods


void Input::ButtonList::addButton(Keyboard k) {
    bool added = false;
    for(Keyboard key : keys) {
        if (k == key) {
            added == true;
            break;
        }
    }

    if (!added) keys.push_back(k);
}

void Input::ButtonList::addButton(MouseButtons k) {
    bool added = false;
    for(MouseButtons key : mouseButtons) {
        if (k == key) {
            added == true;
            break;
        }
    }

    if (!added) mouseButtons.push_back(k);
}

void Input::ButtonList::addButton(PadID id, PadButtons k) {
    bool added = false;

    vector<PadButtons> * vec = nullptr;
    for(std::pair<PadID, vector<PadButtons>> & p : padButtons) { // TODO: could be yucky since it scales with controller count. Though, count is usually 4 max
        if (p.first == id) {
            vec = &p.second;
            for(PadButtons key : *vec) {
                if (k == key) {
                    added == true;
                    break;
                }
            }
        }
    }

    if (!vec) {
        padButtons.push_back(make_pair(id, std::vector<PadButtons>(0)));
    }


    if (!added) vec->push_back(k);
}








bool Input::ButtonList::GetState() {
    bool out = false;
    for(Keyboard k : keys) {
        out |= Input::GetState(k);
    }
    for(MouseButtons k : mouseButtons) {
        out |= Input::GetState(k);
    }
    /*
    for(std::pair<PadID, vector<PadButtons>> pr : padButtons) {
        for(PadButtons p : pr.second) {
            out |= Input::GetState(pr.first, p);
        }
    }
    */
    return out;
}


bool Input::ButtonList::IsPressed() {
    bool out = false;
    for(Keyboard k : keys) {
        out |= Input::IsPressed(k);
    }
    for(MouseButtons k : mouseButtons) {
        out |= Input::IsPressed(k);
    }
    /*
    for(std::pair<PadID, vector<PadButtons>> pr : padButtons) {
        for(PadButtons p : pr.second) {
            out |= Input::IsPressed(pr.first, p);
        }
    }
    */
    return out;
}


bool Input::ButtonList::IsHeld() {
    bool out = false;
    for(Keyboard k : keys) {
        out |= Input::IsHeld(k);
    }
    for(MouseButtons k : mouseButtons) {
        out |= Input::IsHeld(k);
    }
    /*
    for(std::pair<PadID, vector<PadButtons>> pr : padButtons) {
        for(PadButtons p : pr.second) {
            out |= Input::IsHeld(pr.first, p);
        }
    }
    */
    return out;
}









bool Input::IsShiftMod() {
    return thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)Keyboard::Key_lshift] ||
           thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)Keyboard::Key_rshift];
}

void Input::getUnicode() {
    // Go through a - z
    InputDevice * kb = thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard];

    for(int i = (int)Keyboard::Key_a; i < (int)Keyboard::Key_z + 1; ++i) {
        if (Input::IsPressed((Keyboard)i)) {
            lastUnicode = i - (int)Keyboard::Key_a + 'a';
            if (IsShiftMod()) { lastUnicode += 'A' - 'a'; }
        }
    }
    for(int i = (int)Keyboard::Key_0; i < (int)Keyboard::Key_9 + 1; ++i) {

        if (Input::IsPressed((Keyboard)i)) {
            lastUnicode = i - (int)Keyboard::Key_0 + '0';
            if (IsShiftMod()) {
                     if (lastUnicode == '1') lastUnicode = '!';
                else if (lastUnicode == '2') lastUnicode = '@';
                else if (lastUnicode == '3') lastUnicode = '#';
                else if (lastUnicode == '4') lastUnicode = '$';
                else if (lastUnicode == '5') lastUnicode = '%';
                else if (lastUnicode == '6') lastUnicode = '^';
                else if (lastUnicode == '7') lastUnicode = '&';
                else if (lastUnicode == '8') lastUnicode = '*';
                else if (lastUnicode == '9') lastUnicode = '(';
                else if (lastUnicode == '0') lastUnicode = ')';



            }
        }
    }

    if (!IsShiftMod()) {
         if (Input::IsPressed(Keyboard::Key_comma))     lastUnicode = ',';
    else if (Input::IsPressed(Keyboard::Key_period))    lastUnicode = '.';
    else if (Input::IsPressed(Keyboard::Key_semicolon)) lastUnicode = ';';
    else if (Input::IsPressed(Keyboard::Key_apostrophe))lastUnicode = '\'';
    else if (Input::IsPressed(Keyboard::Key_lbracket))  lastUnicode = '[';
    else if (Input::IsPressed(Keyboard::Key_rbracket))  lastUnicode = ']';
    else if (Input::IsPressed(Keyboard::Key_minus))     lastUnicode = '-';
    else if (Input::IsPressed(Keyboard::Key_equal))     lastUnicode = '=';
    else if (Input::IsPressed(Keyboard::Key_backslash)) lastUnicode = '\\';
    else if (Input::IsPressed(Keyboard::Key_frontslash))lastUnicode = '/';
    else if (Input::IsPressed(Keyboard::Key_grave))     lastUnicode = '`';

    } else {
         if (Input::IsPressed(Keyboard::Key_comma))     lastUnicode = '<';
    else if (Input::IsPressed(Keyboard::Key_period))    lastUnicode = '>';
    else if (Input::IsPressed(Keyboard::Key_semicolon)) lastUnicode = ':';
    else if (Input::IsPressed(Keyboard::Key_apostrophe))lastUnicode = '"';
    else if (Input::IsPressed(Keyboard::Key_lbracket))  lastUnicode = '{';
    else if (Input::IsPressed(Keyboard::Key_rbracket))  lastUnicode = '}';
    else if (Input::IsPressed(Keyboard::Key_minus))     lastUnicode = '_';
    else if (Input::IsPressed(Keyboard::Key_equal))     lastUnicode = '+';
    else if (Input::IsPressed(Keyboard::Key_backslash)) lastUnicode = '|';
    else if (Input::IsPressed(Keyboard::Key_frontslash))lastUnicode = '?';
    else if (Input::IsPressed(Keyboard::Key_grave))     lastUnicode = '~';
    }

    if (Input::IsPressed(Keyboard::Key_enter)) lastUnicode = '\n';
    if (Input::IsPressed(Keyboard::Key_backspace)) lastUnicode = '\b';
    if (Input::IsPressed(Keyboard::Key_space)) lastUnicode = ' ';

}




//////////////////// Statics

// takes raw mouse coords and translates them to
// where it is actually contacting 2D space
int MouseXDeviceToWorld2D(int x) {
    if (!ViewManager::Get(focusID)) return 0;
    return
        Graphics::GetCamera2D().node.local.position.x +
        x*(Graphics::GetRenderCamera().Width() / (float)ViewManager::Get(focusID)->Width());
}


int MouseYDeviceToWorld2D(int y) {
    if (!ViewManager::Get(focusID)) return 0;
    return
        Graphics::GetCamera2D().node.local.position.y +
        y*(Graphics::GetRenderCamera().Height() / (float)ViewManager::Get(focusID)->Height());
}
