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

static void getUnicode(float prevState, const InputDevice::Event &);






static InputManager * manager;
static int lastUnicode;

static std::map<std::string, UserInput> stringMapInput;
static std::map<std::string, std::pair<PadID, UserInput>> stringMapPad;






static bool inputLocked;



static int mouseX = 0;
static int mouseY = 0;


static bool lockCallbackMaps = false;
static std::vector<InputListener*> deletedListeners;





struct DeviceState {
    DeviceState() {
        thisState = (float*)calloc(sizeof(float), (int)UserInput::Count);
        prevState = (float*)calloc(sizeof(float), (int)UserInput::Count);
        memset(listeners, sizeof(void*)*((int)UserInput::Count), 0);
        device = nullptr;
    }

    ~DeviceState() {
        free(thisState);
        free(prevState);
    }

    void Update() {
        InputDevice::Event ev;
        if (!device) return;
        while(device->GetEventCount()) {
            device->PopEvent(ev);
            int index = (int)ev.id;
            prevState[index] = thisState[index];
            thisState[index] = ev.state;

            

            if (listeners[index] && listeners[index]->size()) {
                auto inst = listeners[index];
                if (inst) {
                    for(size_t i = 0; i < inst->size(); ++i) {
                        if (!prevState[index] && thisState[index]) {
                            (*inst)[i]->OnPress();
                        } else if (prevState[index] && thisState[index]) {
                            (*inst)[i]->OnHold();
                        } else if (prevState[index] && !thisState[index]) {
                            (*inst)[i]->OnRelease();
                        }
                    }
                }
            }

            if (device->GetType() == InputDevice::Class::Keyboard) {                
                getUnicode(prevState[index], ev);
            }
        }
    }

    
    InputDevice * device;
    float * prevState;
    float * thisState;
    std::vector<InputListener*> * listeners[(int)UserInput::Count];
};

static DeviceState devices[(int)InputManager::DefaultDeviceSlots::NumDefaultDevices];


static int MouseXDeviceToWorld2D(int);
static int MouseYDeviceToWorld2D(int);
static ViewID focusID;



void Input::Init() {
    manager = (InputManager *)Backend::CreateDefaultInputManager();
    inputLocked = false;


    /* initialize devices */


}

void Input::ShowVirtualKeyboard(bool b) {
    manager->ShowVirtualKeyboard(b);
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
        devices[i].device = manager->QueryDevice(i);
        devices[i].Update();
    }


    mouseY = MouseYDeviceToWorld2D(
        devices[
            (int)InputManager::DefaultDeviceSlots::Mouse
        ].thisState[
            (int)UserInput::Pointer_Y
        ]
    );

    mouseX = MouseXDeviceToWorld2D(
        devices[
            (int)InputManager::DefaultDeviceSlots::Mouse
        ].thisState[
            (int)UserInput::Pointer_X
        ]
    );

    lockCallbackMaps = false;

    /*
    for(uint32_t i = 0; i < deletedListeners.size(); ++i) {
        auto b = deletedListeners[i];
        auto pFind = padCallbackMap.find(b);
        if (pFind != padCallbackMap.end()) padCallbackMap.erase(pFind);


        auto kFind = inputCallbackMap.find(b);
        if (kFind != inputCallbackMap.end()) inputCallbackMap.erase(kFind);

        auto sFind = strCallbackMap.find(b);
        if (sFind != strCallbackMap.end()) strCallbackMap.erase(sFind);
    }
    */
    deletedListeners.clear();

}

std::vector<int> Input::QueryPads() {
    std::vector<int> out;
    for(int i = (int) InputManager::DefaultDeviceSlots::Pad1; 
            i<= (int) InputManager::DefaultDeviceSlots::Pad4;
            ++i) {
        if (devices[i].device) {
            out.push_back(i);
        }
    }

    return out;
}












bool Input::IsHeld(UserInput k) {
    float prev;
    float self;
    if (k < UserInput::Pointer_0) {
        prev = devices[(int)InputManager::DefaultDeviceSlots::Keyboard].prevState[(int)k];
        self = devices[(int)InputManager::DefaultDeviceSlots::Keyboard].thisState[(int)k];

    } else {
        prev = devices[(int)InputManager::DefaultDeviceSlots::Mouse].prevState[(int)k];
        self = devices[(int)InputManager::DefaultDeviceSlots::Mouse].thisState[(int)k];
    } 
    if (!prev) return false;
    return (prev &&
            self);


}

bool Input::IsHeld(PadID b, UserInput k) {
    if (!devices[b].prevState[(int)k]) return false;
    return (devices[b].thisState[(int)k] &&
            devices[b].prevState[(int)k]);
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

float Input::GetState(UserInput k) {
    if (k < UserInput::Pointer_0) {
        return devices[(int)InputManager::DefaultDeviceSlots::Keyboard].thisState[(int)k];

    } else {
        return devices[(int)InputManager::DefaultDeviceSlots::Mouse].thisState[(int)k];
    } 

}

float Input::GetState(PadID b, UserInput k) {
    if (!devices[b].prevState[(int)k]) return false;
    return (devices[b].thisState[(int)k]);
}


float Input::GetState(const std::string & s) {
    auto keyboardIter = stringMapInput.find(s);
    if (keyboardIter != stringMapInput.end() &&
        GetState(keyboardIter->second)) return true;

    
    auto padIter = stringMapPad.find(s);
    if (padIter != stringMapPad.end() &&
        GetState(padIter->second.first, padIter->second.second)) return true;
    

    return false;
}





int Input::MouseX() {
    return mouseX;
}

int Input::MouseY() {
    return mouseY;
}

int Input::MouseXDelta() {
    return MouseXDeviceToWorld2D(devices[(int)InputManager::DefaultDeviceSlots::Mouse].thisState[(int)UserInput::Pointer_X]) -
           MouseXDeviceToWorld2D(devices[(int)InputManager::DefaultDeviceSlots::Mouse].prevState[(int)UserInput::Pointer_X]);


}

int Input::MouseYDelta() {
    return MouseXDeviceToWorld2D(devices[(int)InputManager::DefaultDeviceSlots::Mouse].thisState[(int)UserInput::Pointer_Y]) -
           MouseXDeviceToWorld2D(devices[(int)InputManager::DefaultDeviceSlots::Mouse].prevState[(int)UserInput::Pointer_Y]);
}

int Input::MouseWheel() {
    return (int)devices[(int)InputManager::DefaultDeviceSlots::Mouse].thisState[(int)UserInput::Pointer_Wheel];
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


void Input::MapInput(const std::string & id, PadID idpad, UserInput key) {
    stringMapPad[id] = {idpad, key};
}














void Input::UnmapInput(const std::string & id) {
    stringMapInput.erase(id);
    stringMapPad.erase(id);
}



void Input::AddListener(InputListener * b, UserInput i) {
    if (i < UserInput::Pointer_0) {
        if (!devices[(int)InputManager::DefaultDeviceSlots::Keyboard].listeners[(int)i]) {
            devices[(int)InputManager::DefaultDeviceSlots::Keyboard].listeners[(int)i] = new std::vector<InputListener*>;
        }
        devices[(int)InputManager::DefaultDeviceSlots::Keyboard].listeners[(int)i]->push_back(b);
    } else {
        if (!devices[(int)InputManager::DefaultDeviceSlots::Mouse].listeners[(int)i]) {
            devices[(int)InputManager::DefaultDeviceSlots::Mouse].listeners[(int)i] = new std::vector<InputListener*>;
        }
        devices[(int)InputManager::DefaultDeviceSlots::Mouse].listeners[(int)i]->push_back(b);
    } 
}


void Input::AddListener(InputListener * b, PadID id, UserInput i) {
    if (!devices[id].listeners[(int)i]) {
        devices[id].listeners[(int)i] = new std::vector<InputListener*>;
    }
    devices[id].listeners[(int)i]->push_back(b);
}

void Input::AddListener(InputListener * b, const std::string & i) {
    //strCallbackMap[b] = i;
    // TODO
}


void Input::RemoveListener(InputListener * b) {
    deletedListeners.push_back(b);
}












bool IsShiftMod() {
    return Input::GetState(UserInput::Key_lshift) ||
           Input::GetState(UserInput::Key_rshift);
}

void getUnicode(float prevState, const InputDevice::Event & event) {
    

    // Go through a - z
    static int previousUnicode = 0;
    lastUnicode = event.utf8;


    if (!lastUnicode) return;

    // New press is detected
    static int counter = 0;
    if (lastUnicode != previousUnicode) counter = 0;


    for(uint32_t i = 0; i < unicodeListeners.size(); ++i) {
        unicodeListeners[i]->OnNewUnicode(lastUnicode);
    }
    
    if (!Engine::IsPaused()) {
        for(uint32_t i = 0; i < unicodeListenersPausable.size(); ++i) {
            unicodeListenersPausable[i]->OnNewUnicode(lastUnicode);
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
