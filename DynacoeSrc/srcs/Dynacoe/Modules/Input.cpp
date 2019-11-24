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
#include <Dynacoe/Util/Time.h>

#include <algorithm>
#include <cstring>
#include <cmath>

using namespace Dynacoe;
using namespace std;



static bool IsShiftMod();

static void getUnicode(float prevState, const InputDevice::Event &);






static InputManager * manager;
static int lastUnicode;



static bool inputLocked;



static int mouseX = 0;
static int mouseY = 0;


static bool lockCallbackMaps = false;
static std::vector<InputListener*> deletedListeners;


class DeviceState;
struct MappedInputData {
    DeviceState * device;
    int input;

    bool operator==(const MappedInputData & other) const {
        return other.device == device && other.input == input;
    }
};

namespace std {
    template <>
    struct hash<MappedInputData> {
        std::size_t operator()(const MappedInputData & key) const {
            return hash<void*>()(key.device) ^ (hash<int>()(key.input) << 1);
        }
    };
}



static std::unordered_map<std::string, MappedInputData> stringMapInput;
static std::unordered_map<MappedInputData, std::string> inputMapString;
static std::unordered_map<std::string, std::vector<InputListener *>> stringListeners;


class DeviceState {
  public:

    struct InputState {
        float prev;
        float current;
        std::string mapped;
    };

    DeviceState() {
        device = nullptr;
        
    }

    ~DeviceState() {
        auto iter = inputs.begin();
        while(iter != inputs.end()) {
            delete iter->second; iter++;
        }
    }

    InputState * GetInput(int index) {
        InputState * input = nullptr;
        auto iter = inputs.find(index);
        if (iter == inputs.end()) {
            InputState * state = new InputState;
            state->prev = 0.f;
            state->current = 0.f;

            inputs[index] = state;
            input = state;
            return input;
        } else {
            return iter->second;
        }
    }    

    void Update() {
        InputDevice::Event ev;
        if (!device) return;
        while(device->GetEventCount()) {
            device->PopEvent(ev);
            int index = (int)ev.id;

            InputState * input = GetInput(index);
            input->prev = input->current;
            input->current = ev.state;

            //printf("%d -> %f\n", index, ev.state);

            

            if (listeners.size()) {
                auto inst = &listeners;
                for(size_t i = 0; i < inst->size(); ++i) {
                    (*inst)[i]->OnChange(index, input->current);
                    if (!input->prev && input->current) {
                        (*inst)[i]->OnPress(index);
                    } else if (input->prev && !input->current) {
                        (*inst)[i]->OnRelease(index);
                    }
                }
            }

            // string handlers
            if (!inputMapString.empty() && !stringListeners.empty()) {
                MappedInputData key;
                key.device = this;
                key.input = index;
                auto iter = inputMapString.find(key);
                if (iter != inputMapString.end()) {
                    auto inst = &stringListeners[iter->second];
                    for(size_t i = 0; i < inst->size(); ++i) {
                        (*inst)[i]->OnChange(index, input->current);
                        if (!input->prev && input->current) {
                            (*inst)[i]->OnPress(index);
                        } else if (input->prev && !input->current) {
                            (*inst)[i]->OnRelease(index);
                        }
                    }
                }
            }

            if (device->GetType() == InputDevice::Class::Keyboard) {                
                getUnicode(input->current, ev);
            }
        }



        // fire callbacks for held buttons
        if (listeners.size()) {

            for(auto it = inputs.begin(); it != inputs.end(); ++it) {
                InputState * input = it->second;
                if (fabs(input->current) > 0.0001) {
                    for(uint32_t i = 0; i < listeners.size(); ++i) {
                        listeners[i]->OnActive(it->first, input->current);
                    }            
                }
            }
        }

        // string listeners
        if (!inputMapString.empty() && !stringListeners.empty()) {
            for(auto it = inputMapString.begin(); it != inputMapString.end(); ++it) {
                // only listeners for this device
                if (it->first.device != this) continue;

                // get input state for this mapping
                InputState * input = inputs[it->first.input];

                if (fabs(input->current) > 0.0001) {
                    auto inst = &stringListeners[it->second];

                    for(size_t i = 0; i < inst->size(); ++i) {
                        (*inst)[i]->OnActive(it->first.input, input->current);
                    }            
                }
            }            
        }
    }
    InputDevice * device;


    void AddListener(InputListener * l) {
        listeners.push_back(l);
    }

  private:
    std::unordered_map<int, InputState*> inputs;
    std::vector<InputListener*> listeners;

};


static std::set<InputListener*> allListeners;



static DeviceState devices[(int)InputManager::DefaultDeviceSlots::NumDefaultDevices];


static int MouseXDeviceToWorld2D(int);
static int MouseYDeviceToWorld2D(int);
static ViewID focusID;

void Input::SetDeadzone(int pad, int input, float amt) {
    if (pad < 3) return;
    if (pad >= (int)InputManager::DefaultDeviceSlots::NumDefaultDevices) return;
    devices[pad].device->SetDeadzone(input, amt);
}

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

    DeviceState::InputState * inputY = devices[(int)InputManager::DefaultDeviceSlots::Mouse].GetInput((int)UserInput::Pointer_Y);
    DeviceState::InputState * inputX = devices[(int)InputManager::DefaultDeviceSlots::Mouse].GetInput((int)UserInput::Pointer_X);

    

    mouseY = inputY->current;
    mouseX = inputX->current;


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






float Input::GetState(int k) {

    DeviceState::InputState * input = nullptr;
    if (k < UserInput::Pointer_0) {
        input = devices[(int)InputManager::DefaultDeviceSlots::Keyboard].GetInput((int)k);
    } else {
        input = devices[(int)InputManager::DefaultDeviceSlots::Mouse].GetInput((int)k);
    } 

    return input->current;
}

float Input::GetState(PadID b, int k) {

    DeviceState::InputState * input = devices[(int)InputManager::DefaultDeviceSlots::Pad1+b].GetInput((int)k);
    return input->current;
}


float Input::GetState(const std::string & s) {
    auto iter = stringMapInput.find(s);
    if (iter == stringMapInput.end()) return false;

    DeviceState::InputState * input = iter->second.device->GetInput(iter->second.input);
    if (!input->prev) return false;
    return (input->current &&
            input->prev);


}





int Input::MouseX() {
    return mouseX;
}

int Input::MouseY() {
    return mouseY;
}

int Input::MouseXDelta() {
    DeviceState::InputState * input = devices[(int)InputManager::DefaultDeviceSlots::Mouse].GetInput((int)UserInput::Pointer_X);

    
    return MouseXDeviceToWorld2D(input->current) -
           MouseXDeviceToWorld2D(input->prev);


}

int Input::MouseYDelta() {
    DeviceState::InputState * input = devices[(int)InputManager::DefaultDeviceSlots::Mouse].GetInput((int)UserInput::Pointer_Y);

    
    return MouseYDeviceToWorld2D(input->current) -
           MouseYDeviceToWorld2D(input->prev);
}

int Input::MouseWheel() {
    DeviceState::InputState * input = devices[(int)InputManager::DefaultDeviceSlots::Mouse].GetInput((int)UserInput::Pointer_Wheel);
    return input->current;
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


void Input::MapInput(const std::string & id, int k) {
    DeviceState * input;
    if (k < UserInput::Pointer_0) {
        input = &devices[(int)InputManager::DefaultDeviceSlots::Keyboard];
    } else {
        input = &devices[(int)InputManager::DefaultDeviceSlots::Mouse];
    } 

    MappedInputData data;
    data.device = input;
    data.input = k;

    stringMapInput[id] = data;
    inputMapString[data] = id;
}


void Input::MapInput(const std::string & id, PadID idpad, int key) {
    DeviceState * input = &devices[(int)InputManager::DefaultDeviceSlots::Pad1+idpad];

    MappedInputData data;
    data.device = input;
    data.input = key;

    stringMapInput[id] = data;
    inputMapString[data] = id;

}














void Input::UnmapInput(const std::string & id) {
    stringMapInput.erase(id);
}



void Input::AddKeyboardListener(InputListener * b) {
    DeviceState * input = &devices[(int)InputManager::DefaultDeviceSlots::Keyboard];
    input->AddListener(b);
    allListeners.insert(b);
}

void Input::AddMouseListener(InputListener * b) {
    DeviceState* input = &devices[(int)InputManager::DefaultDeviceSlots::Mouse];
    input->AddListener(b);
    allListeners.insert(b);
}


void Input::AddPadListener(InputListener * b, PadID id) {
    DeviceState * input = &devices[(int)InputManager::DefaultDeviceSlots::Pad1+id];
    input->AddListener(b);
    allListeners.insert(b);    
}


void Input::AddMappedListener(InputListener * b, const std::string & i) {
    //strCallbackMap[b] = i;
    // TODO


    stringListeners[i].push_back(b);


}


void Input::RemoveListener(InputListener * b) {
    deletedListeners.push_back(b);

    if (!stringListeners.empty()) {
        for(auto it = stringListeners.begin(); it != stringListeners.end(); ++it) {
            std::vector<InputListener*> & inst = it->second;
            for(uint32_t i = 0; i < inst.size(); ++i) {
                if (inst[i] == b) {
                    inst.erase(inst.begin()+i);
                    return;
                }
            }
        }
    }
}












bool IsShiftMod() {
    return Input::GetState(UserInput::Key_lshift) ||
           Input::GetState(UserInput::Key_rshift);
}

void getUnicode(float prevState, const InputDevice::Event & event) {
    
    int unicode = event.utf8;

    // Filter newline
    switch(unicode) {
      case 12:
      case 13:
        unicode = 10;
        break;
      default:;
    }    


    static int previousUnicode = 0;
    static double time = 0;
    static double startTime = 0;

    lastUnicode = unicode;
    if (!lastUnicode) return;



    if (lastUnicode != previousUnicode) startTime = Dynacoe::Time::MsSinceStartup();

    if (event.state) {
        for(uint32_t i = 0; i < unicodeListeners.size(); ++i) {
            unicodeListeners[i]->OnNewUnicode(lastUnicode);
        }
        
        if (!Engine::IsPaused()) {
            for(uint32_t i = 0; i < unicodeListenersPausable.size(); ++i) {
                unicodeListenersPausable[i]->OnNewUnicode(lastUnicode);
            }            
        }
    } else if (event.state) {
    
        //printf("aaa %d vs %d\n", lastUnicode, previousUnicode);

        // Key has been held, implying a multiple key request (for us / latin keyboards)
        if (lastUnicode == previousUnicode && lastUnicode) {
            if ((Dynacoe::Time::MsSinceStartup() - startTime > 500) && 
                (Dynacoe::Time::MsSinceStartup() - time > 100)) {
                    
                for(uint32_t i = 0; i < unicodeListeners.size(); ++i) {
                    unicodeListeners[i]->OnRepeatUnicode(lastUnicode);
                }
                
                if (!Engine::IsPaused()) {
                    for(uint32_t i = 0; i < unicodeListenersPausable.size(); ++i) {
                        unicodeListenersPausable[i]->OnRepeatUnicode(lastUnicode);
                    }
                }
                time = Time::MsSinceStartup();
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
