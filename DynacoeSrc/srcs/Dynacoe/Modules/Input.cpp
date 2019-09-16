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
#include <Dynacoe/Dynacoe.h>
#include <algorithm>
#include <cstring>


using namespace Dynacoe;
using namespace std;



static bool IsShiftMod();

static void getUnicode();






static InputManager * manager;
static int lastUnicode;

//static std::map<std::string, ButtonList*> stringMap;
static std::map<std::string, UserInput> stringMapInput;
static std::map<std::string, std::pair<PadID, PadButtons>> stringMapPad;
static std::vector<ButtonList*> buttonLists;






static bool inputLocked;



static int mouseX = 0;
static int mouseY = 0;


static bool lockCallbackMaps = false;
static std::vector<InputListener*> deletedListeners;





struct DeviceState {
    DeviceState() {
        thisState = calloc(sizeof(float), (int)UserInput::Count);
        prevState = calloc(sizeof(float), (int)UserInput::Count);
        memset(listeners, sizeof(void*)*((int)UserInput::Count), 0);
        device = nullptr;
    }

    ~DeviceState() {
        free(thisState);
        free(prevState);
    }

    void Update() {
        InputDevice::Event ev;
        while(device->GetDeviceCount()) {
            device->PopEvent(ev);
            prevState[ev.id] = thisState[ev.id];
            thisState[ev.id] = ev.state;

            if (listeners.size()) {
                auto inst = listeners[ev.id];
                if (inst) {
                    for(size_t i = 0; i < inst->size(); ++i) {
                        if (!prevState[ev.id].state && thisState[ev.id].state) {
                            (*inst)[i]->OnPress();
                        } else if (prevState[ev.id].state && thisState[ev.id].state) {
                            (*inst)[i]->OnHold();
                        } else if (prevState[ev.id].state && !thisState[ev.id].state) {
                            (*inst)[i]->OnRelease();
                        }
                    }
                }
            }
        }
    }

    
    InputDevice * device;
    float * prevState;
    float * thisState;
    std::vector<InputListener*> * listeners[(int)UserInput::Count];
}

static DeviceState devices[(int)InputManager::DefaultDeviceSlots::NumDefaultDevices];


static int MouseXDeviceToWorld2D(int);
static int MouseYDeviceToWorld2D(int);
static ViewID focusID;



void Input::Init() {
    manager = (InputManager *)Backend::CreateDefaultInputManager();
    inputLocked = false;


    /* initialize devices */


}

void Input::ShowVirtualUserInput(bool b) {
    manager->ShowVirtualUserInput(b);
}




void Input::RunBefore() {

    if (inputLocked) return;
    lockCallbackMaps = true;



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

    

    // pool raw events
    bool updated = manager->HandleEvents();


    // process raw events
    for(int i = 0; i < (int)InputManager::DefaultDeviceSlots::NumDefaultDevices; ++i) {
        devices[i]->device = manager->QueryDevice(i);
        devices[i]->Update();
    }


    mouseY = MouseYDeviceToWorld2D(
        devices[
            (int)InputManager::DefaultDeviceSlots::Mouse
        ]->thisState[
            (int)MouseAxes::Y
        ]
    );

    mouseX = MouseXDeviceToWorld2D(
        devices[
            (int)InputManager::DefaultDeviceSlots::Mouse
        ]->thisState[
            (int)MouseAxes::X
        ]
    );

    lockCallbackMaps = false;
    for(uint32_t i = 0; i < deletedListeners.size(); ++i) {
        auto b = deletedListeners[i];
        auto pFind = padCallbackMap.find(b);
        if (pFind != padCallbackMap.end()) padCallbackMap.erase(pFind);


        auto kFind = inputCallbackMap.find(b);
        if (kFind != inputCallbackMap.end()) inputCallbackMap.erase(kFind);

        auto sFind = strCallbackMap.find(b);
        if (sFind != strCallbackMap.end()) strCallbackMap.erase(sFind);
    }
    deletedListeners.clear();

    //if (updated) {
        getUnicode();
    //}
}

std::vector<int> Input::QueryPads() {
    std::vector<int> out;
    for(int i = (int) InputManager::DefaultDeviceSlots::Pad1; 
            i<= (int) InputManager::DefaultDeviceSlots::Pad4;
            ++i) {
        if (thisState.devices[i]->numButtons ||
            thisState.devices[i]->numAxes) {
            out.push_back(i);
        }
    }

    return out;
}












bool Input::IsHeld(UserInput k) {
    InputDevice * self = nullptr;
    InputDevice * prev = nullptr;
    if (k < UserInput::Pointer_0) {
        prev = prevState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard];
        self = thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard];
    } else {
        prev = prevState.devices[(int)InputManager::DefaultDeviceSlots::Mouse];
        self = thisState.devices[(int)InputManager::DefaultDeviceSlots::Mouse];
    } 
    if (!prev) return;
    return (prev->buttons[(int)k] &&
            self->buttons[(int)k]);


}

bool Input::IsHeld(PadID b, UserInput k) {
    if (!prevState.devices[b]) return false;
    return (prevState.devices[b]->buttons[(int)k] &&
            thisState.devices[b]->buttons[(int)k]);
}


bool Input::IsHeld(const std::string & s) {
    auto keyboardIter = stringMapInput.find(s);
    if (keyboardIter != stringMapInput.end() &&
        IsHeld(keyboardIter->second)) return true;

    
    auto padIter = stringMapPad.find(s);
    if (padIter != stringMapPad.end() &&
        IsHeld(padIter->second.first, padIter->second.second)) return true;
    

    return false;
}







int Input::MouseX() {
    return mouseX;
}

int Input::MouseY() {
    return mouseY;
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

static std::vector<UnicodeListener*> unicodeListeners;
static std::vector<UnicodeListener*> unicodeListenersPausable;

void Input::AddUnicodeListener(UnicodeListener * listener, bool pausable) {
    if (pausable)
        unicodeListenersPausable.push_back(listener);
    else
        unicodeListeners.push_back(listener);
}

void Input::RemoveUnicodeListener(UnicodeListener * listener) {
    for(uint32_t i = 0; i < unicodeListeners.size(); ++i) {
        if (unicodeListeners[i] == listener) {
            unicodeListeners.erase(unicodeListeners.begin()+i);
            return;
        }
    }

    for(uint32_t i = 0; i < unicodeListenersPausable.size(); ++i) {
        if (unicodeListenersPausable[i] == listener) {
            unicodeListenersPausable.erase(unicodeListenersPausable.begin()+i);
            return;
        }
    }
}




/* input mapping */


void Input::MapInput(const std::string & id, UserInput key) {
    stringMapInput[id] = key;
}


void Input::MapInput(const std::string & id, PadID idpad, PadButtons key) {
    stringMapPad[id] = {idpad, key};
}














void Input::UnmapInput(const std::string & id) {
    stringMapInput.erase(id);
    stringMapPad.erase(id);
}



void Input::AddListener(InputListener * b, UserInput i) {
    inputCallbackMap[b] = i;
}


void Input::AddListener(InputListener * b, PadID id, PadButtons i) {
    padCallbackMap[b] = {id, i};
}

void Input::AddListener(InputListener * b, const std::string & i) {
    strCallbackMap[b] = i;
}


void Input::RemoveListener(InputListener * b) {
    deletedListeners.push_back(b);
}





// Private methods

/*
void ButtonList::addButton(UserInput k) {
    bool added = false;
    for(UserInput key : keys) {
        if (k == key) {
            added = true;
            break;
        }
    }

    if (!added) keys.push_back(k);
}

void ButtonList::addButton(UserInput k) {
    bool added = false;
    for(UserInput key : UserInput) {
        if (k == key) {
            added = true;
            break;
        }
    }

    if (!added) UserInput.push_back(k);
}

void ButtonList::addButton(PadID id, PadButtons k) {
    bool added = false;

    vector<PadButtons> * vec = nullptr;
    for(std::pair<PadID, vector<PadButtons>> & p : padButtons) { // TODO: could be yucky since it scales with controller count. Though, count is usually 4 max
        if (p.first == id) {
            vec = &p.second;
            for(PadButtons key : *vec) {
                if (k == key) {
                    added = true;
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








bool ButtonList::GetState() {
    bool out = false;
    for(UserInput k : keys) {
        out |= Input::GetState(k);
    }
    for(UserInput k : UserInput) {
        out |= Input::GetState(k);
    }
    
    for(std::pair<PadID, vector<PadButtons>> pr : padButtons) {
        for(PadButtons p : pr.second) {
            out |= Input::GetState(pr.first, p);
        }
    }
    
    return out;
}


bool ButtonList::IsPressed() {
    bool out = false;
    for(UserInput k : keys) {
        out |= Input::IsPressed(k);
    }
    for(UserInput k : UserInput) {
        out |= Input::IsPressed(k);
    }
    
    for(std::pair<PadID, vector<PadButtons>> pr : padButtons) {
        for(PadButtons p : pr.second) {
            out |= Input::IsPressed(pr.first, p);
        }
    }
    
    return out;
}


bool ButtonList::IsHeld() {
    bool out = false;
    for(UserInput k : keys) {
        out |= Input::IsHeld(k);
    }
    for(UserInput k : UserInput) {
        out |= Input::IsHeld(k);
    }
    
    for(std::pair<PadID, vector<PadButtons>> pr : padButtons) {
        for(PadButtons p : pr.second) {
            out |= Input::IsHeld(pr.first, p);
        }
    }
    
    return out;
}
*/








bool IsShiftMod() {
    return thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)UserInput::Key_lshift] ||
           thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard]->buttons[(int)UserInput::Key_rshift];
}

void getUnicode() {
    // Go through a - z
    static int previousUnicode = 0;
    InputDevice * kb = thisState.devices[(int)InputManager::DefaultDeviceSlots::Keyboard];
    lastUnicode = 0;
    for(int i = (int)UserInput::Key_a; i < (int)UserInput::Key_z + 1; ++i) {
        if (Input::GetState((UserInput)i)) {
            lastUnicode = i - (int)UserInput::Key_a + 'a';
            if (IsShiftMod()) { lastUnicode += 'A' - 'a'; }
        }
    }
    for(int i = (int)UserInput::Key_0; i < (int)UserInput::Key_9 + 1; ++i) {

        if (Input::GetState((UserInput)i)) {
            lastUnicode = i - (int)UserInput::Key_0 + '0';
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
         if (Input::GetState(UserInput::Key_comma))     lastUnicode = ',';
    else if (Input::GetState(UserInput::Key_period))    lastUnicode = '.';
    else if (Input::GetState(UserInput::Key_semicolon)) lastUnicode = ';';
    else if (Input::GetState(UserInput::Key_apostrophe))lastUnicode = '\'';
    else if (Input::GetState(UserInput::Key_lbracket))  lastUnicode = '[';
    else if (Input::GetState(UserInput::Key_rbracket))  lastUnicode = ']';
    else if (Input::GetState(UserInput::Key_minus))     lastUnicode = '-';
    else if (Input::GetState(UserInput::Key_equal))     lastUnicode = '=';
    else if (Input::GetState(UserInput::Key_backslash)) lastUnicode = '\\';
    else if (Input::GetState(UserInput::Key_frontslash))lastUnicode = '/';
    else if (Input::GetState(UserInput::Key_grave))     lastUnicode = '`';

    } else {
         if (Input::GetState(UserInput::Key_comma))     lastUnicode = '<';
    else if (Input::GetState(UserInput::Key_period))    lastUnicode = '>';
    else if (Input::GetState(UserInput::Key_semicolon)) lastUnicode = ':';
    else if (Input::GetState(UserInput::Key_apostrophe))lastUnicode = '"';
    else if (Input::GetState(UserInput::Key_lbracket))  lastUnicode = '{';
    else if (Input::GetState(UserInput::Key_rbracket))  lastUnicode = '}';
    else if (Input::GetState(UserInput::Key_minus))     lastUnicode = '_';
    else if (Input::GetState(UserInput::Key_equal))     lastUnicode = '+';
    else if (Input::GetState(UserInput::Key_backslash)) lastUnicode = '|';
    else if (Input::GetState(UserInput::Key_frontslash))lastUnicode = '?';
    else if (Input::GetState(UserInput::Key_grave))     lastUnicode = '~';
    }

    if (Input::GetState(UserInput::Key_enter)) lastUnicode = '\n';
    if (Input::GetState(UserInput::Key_backspace)) lastUnicode = '\b';
    if (Input::GetState(UserInput::Key_space)) lastUnicode = ' ';
    if (Input::GetState(UserInput::Key_tab)) lastUnicode = '\t';

    if (Input::GetState(UserInput::Key_left))  lastUnicode = 17;
    if (Input::GetState(UserInput::Key_up))    lastUnicode = 18;
    if (Input::GetState(UserInput::Key_right)) lastUnicode = 19;
    if (Input::GetState(UserInput::Key_down))  lastUnicode = 20;


    static int counter = 0;
    if (!lastUnicode) {
        counter = 0;
    }
    

    // New press is detected
    if (lastUnicode != previousUnicode && lastUnicode) {
        counter = 0;
        for(uint32_t i = 0; i < unicodeListeners.size(); ++i) {
            unicodeListeners[i]->OnNewUnicode(lastUnicode);
        }
        
        if (!Engine::IsPaused()) {
            for(uint32_t i = 0; i < unicodeListenersPausable.size(); ++i) {
                unicodeListenersPausable[i]->OnNewUnicode(lastUnicode);
            }            
        }
    } 


    // Key has been held, implying a multiple key request (for us / latin keyboards)
    if (lastUnicode == previousUnicode && lastUnicode) {
        counter++;
        if ((counter > Dynacoe::Engine::GetMaxFPS() / 2) && counter % 3 == 0) {
                
            for(uint32_t i = 0; i < unicodeListeners.size(); ++i) {
                unicodeListeners[i]->OnRepeatUnicode(lastUnicode);
            }
            
            if (!Engine::IsPaused()) {
                for(uint32_t i = 0; i < unicodeListenersPausable.size(); ++i) {
                    unicodeListenersPausable[i]->OnRepeatUnicode(lastUnicode);
                }
            }
        }
    }

    previousUnicode = lastUnicode;
}

InputManager * Input::GetManager() {
    return manager;
}




//////////////////// Statics

// takes raw mouse coords and translates them to
// where it is actually contacting 2D space
int MouseXDeviceToWorld2D(int x) {
    if (!ViewManager::Get(focusID)) return 0;
    return
        Graphics::GetCamera2D().Node().GetPosition().x +
        x*(Graphics::GetRenderCamera().Width() / (float)ViewManager::Get(focusID)->Width());
}


int MouseYDeviceToWorld2D(int y) {
    if (!ViewManager::Get(focusID)) return 0;
    return
        Graphics::GetCamera2D().Node().GetPosition().y +
        y*(Graphics::GetRenderCamera().Height() / (float)ViewManager::Get(focusID)->Height());
}
