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

#ifdef DC_BACKENDS_X11INPUT_X11

#include <Dynacoe/Backends/InputManager/X11Input_X11.h>
#include <Dynacoe/Backends/InputManager/InputDevice.h>
#include <Dynacoe/Backends/Display/Display.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <cassert>
#include <map>
#include <fcntl.h>

#include <Dynacoe/Util/Time.h>
#include <Dynacoe/Util/Filesys.h>



static void handleEvent(std::vector<Dynacoe::InputDevice *> & devices, XEvent event);
static void initKeysymMappings();


static std::map<int, Dynacoe::Keyboard> symMapping;
static std::map<int, Dynacoe::MouseButtons> mbMapping;



class InputPad {
  public:
    InputPad(std::string & nameIn) : state(nullptr) {
        if (!pathMan) {
            pathMan = new Dynacoe::Filesys;
            assert(pathMan->ChangeDir("/dev/input/by-path"));
        }
        name = nameIn;
        std::string path = pathMan->GetCWD()+pathMan->GetDirectorySeparator()+name;
        fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (libevdev_new_from_fd(fd, &device) < 0) {
            return;
        }

        printf("Detected %s (%d %d %d)\n", 
            libevdev_get_name(device), 
            libevdev_get_id_bustype(device),
            libevdev_get_id_vendor(device),
            libevdev_get_id_product(device)
        );


        // poll available buttons and axes to get count for the state pointer.
        for(int i = BTN_MISC; i < KEY_OK; ++i) {
            if (libevdev_has_event_code(device, EV_KEY, i)) {
                printf("BUTTON - %s\n", libevdev_event_code_get_name(EV_KEY, i));
                buttons.push_back(i);
                buttonMap[i] = buttons.size()-1;
            } else {
                buttonMap[i] = -1;
            }
        }

        for(int i = ABS_X; i <= ABS_MISC; ++i) {
            if (libevdev_has_event_code(device, EV_ABS, i)) {
                printf("AXIS - %s\n", libevdev_event_code_get_name(EV_ABS, i));
                axes.push_back(i);
                axisMap[i] = axes.size()-1;
            } else {
                axisMap[i] = -1;
            }
        }

        timeLast = Dynacoe::Time::MsSinceStartup();
        state = new Dynacoe::InputDevice(buttons.size(), axes.size());
    }

    static const std::vector<std::string> & GetAllDevices() {
        if (!pathMan) {
            pathMan = new Dynacoe::Filesys;
            assert(pathMan->ChangeDir("/dev/input/by-path"));
        }

        static std::vector<std::string> out;
        static double lastTime = 0;

        if (Dynacoe::Time::MsSinceStartup() - lastTime > 500) {
            auto dir = pathMan->QueryDirectory();
            out.clear();
            while(!dir.AtEnd()) {
                std::string name = dir.GetNextName();
                if (name.find("event-joystick") != std::string::npos) {
                    out.push_back(name);
                }
            }
            lastTime = Dynacoe::Time::MsSinceStartup();
            
        }
        return out;


    }

    Dynacoe::InputDevice * GetInputDevice() {
        return state;
    }

    const std::string & GetName() {
        return name;
    }

    void Update() {
        // flush event queue update state with state
        struct input_event ev;
        if (libevdev_next_event(device, LIBEVDEV_READ_FLAG_NORMAL, &ev) < 0) {
            return;
        }

        switch(ev.type) {
          case EV_KEY:
            state->buttons[buttonMap[ev.code]] = ev.value;
            break;
          case EV_ABS:
            state->axes[axisMap[ev.code]] = ev.value;
            break;
            
        }
    }

    bool IsValid() {
        return device;
    }

  private:
    static Dynacoe::Filesys * pathMan;
    double timeLast;;

    std::vector<int> buttons;
    std::vector<int> axes;
    int buttonMap[KEY_OK];
    int axisMap[ABS_MISC+1];

    std::string name;
    int fd;
    struct libevdev * device;
    Dynacoe::InputDevice * state;    

};

Dynacoe::Filesys * InputPad::pathMan = nullptr;;

std::string Dynacoe::X11InputManager::Name() { return "X11Input Manager";}
std::string Dynacoe::X11InputManager::Version() { return "0.1";}
bool Dynacoe::X11InputManager::Valid() { return true; }


Dynacoe::X11InputManager::X11InputManager() {
    static bool initializedMappings;
    if (!initializedMappings) {
        initKeysymMappings();
        initializedMappings = true;
    }
        

    // initialize default devices.
    devices.resize((int)DefaultDeviceSlots::NumDefaultDevices);
    devices[(int)DefaultDeviceSlots::Keyboard] = new Dynacoe::InputDevice((int)Dynacoe::Keyboard::NumButtons, 0);
    devices[(int)DefaultDeviceSlots::Mouse] = new Dynacoe::InputDevice((int)Dynacoe::MouseButtons::NumButtons, (int)Dynacoe::MouseAxes::NumAxes);

    // these devices are just not supported yet
    devices[(int)DefaultDeviceSlots::Touchpad] = new Dynacoe::InputDevice(0, 0);
    devices[(int)DefaultDeviceSlots::Pad1] = new Dynacoe::InputDevice(0, 0);
    devices[(int)DefaultDeviceSlots::Pad2] = new Dynacoe::InputDevice(0, 0);
    devices[(int)DefaultDeviceSlots::Pad3] = new Dynacoe::InputDevice(0, 0);
    devices[(int)DefaultDeviceSlots::Pad4] = new Dynacoe::InputDevice(0, 0);
}

bool Dynacoe::X11InputManager::IsSupported(InputType type) {
    if (type == Touchpad) {
        return false;
    }
    return true;
}


static InputPad * physicalPads[4] = {};
std::vector<std::string> lastPadState;

bool Dynacoe::X11InputManager::HandleEvents() {
    auto p = InputPad::GetAllDevices();

    int changed = p.size() != lastPadState.size();
    if (!changed) {
        for(int i = 0; i < p.size() && i < lastPadState.size(); ++i) {
            if (p[i] != lastPadState[i]) {
                changed = true;
                break;       
            }
        }
    }

    if (changed) {
        // figure out what changed
        std::vector<std::string> newNames;
        std::vector<std::string> removedNames;
        for(int i = 0; i < p.size(); ++i) {
            int found = false;
            for(int n = 0; n < lastPadState.size(); ++n) {
                if (p[i] == lastPadState[n]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                newNames.push_back(p[i]);
            }
        }
        for(int i = 0; i < lastPadState.size(); ++i) {
            int found = false;
            for(int n = 0; n < p.size(); ++n) {
                if (p[n] == lastPadState[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                removedNames.push_back(lastPadState[i]);
            }
        }




        // remove missing names
        for(int n = 0; n < removedNames.size(); ++n) {
            printf("untracking removed device %s\n", removedNames[n].c_str());

            for(int i = 0; i < 4; ++i) {

                // next empty slot
                if (physicalPads[i] && physicalPads[i]->GetName() == removedNames[n]) {
                    //delete devices[(int)DefaultDeviceSlots::Pad1+i];
                    delete physicalPads[i];
                    devices[(int)DefaultDeviceSlots::Pad1+i] = nullptr;
                    physicalPads[i] = nullptr;
                    break;
                }
            }
        }
        

        // add new names
        for(int n = 0; n < newNames.size(); ++n) {
            printf("tracking added device %s\n", newNames[n].c_str());

            for(int i = 0; i < 4; ++i) {
                // next empty slot
                if (!physicalPads[i]) {
                    //delete devices[(int)DefaultDeviceSlots::Pad1+i];
                    physicalPads[i] = new InputPad(newNames[n]);
                    devices[(int)DefaultDeviceSlots::Pad1+i] = physicalPads[i]->GetInputDevice();
                    break;
                }
            }
        }
        

        lastPadState = p;
    }

    if (!display) return false;
    std::vector<XEvent> * ev = (std::vector<XEvent>*)display->GetLastSystemEvent();
    if (!ev) return false;
    devices[1]->axes[(int)Dynacoe::MouseAxes::Wheel] = 0;

    for(uint32_t i = 0; i < ev->size(); ++i) {
        handleEvent(devices, (*ev)[i]);            
    }
    return ev->size();;
}

Dynacoe::InputDevice * Dynacoe::X11InputManager::QueryDevice(int ID) {
    if (ID >= devices.size()) return nullptr;
    return devices[ID];    
}


Dynacoe::InputDevice * Dynacoe::X11InputManager::QueryDevice(DefaultDeviceSlots slot) {
    return QueryDevice((int)slot);
}


int Dynacoe::X11InputManager::QueryAuxiliaryDevices(int * slots) {
    return 0;
}

int Dynacoe::X11InputManager::MaxDevices() {
    return (int) DefaultDeviceSlots::NumDefaultDevices;
}

void Dynacoe::X11InputManager::SetFocus(Dynacoe::Display * d) {
    display = d;
}

Dynacoe::Display * Dynacoe::X11InputManager::GetFocus() {
    return display;
}








// key sym to Dynacoe::Keyboard:: mappings:
void initKeysymMappings() {
    symMapping[XK_0] = Dynacoe::Keyboard::Key_0;
    symMapping[XK_1] = Dynacoe::Keyboard::Key_1;
    symMapping[XK_2] = Dynacoe::Keyboard::Key_2;
    symMapping[XK_3] = Dynacoe::Keyboard::Key_3;
    symMapping[XK_4] = Dynacoe::Keyboard::Key_4;
    symMapping[XK_5] = Dynacoe::Keyboard::Key_5;
    symMapping[XK_6] = Dynacoe::Keyboard::Key_6;
    symMapping[XK_7] = Dynacoe::Keyboard::Key_7;
    symMapping[XK_8] = Dynacoe::Keyboard::Key_8;
    symMapping[XK_9] = Dynacoe::Keyboard::Key_9;

    symMapping[XK_parenright] = Dynacoe::Keyboard::Key_0;
    symMapping[XK_exclam] = Dynacoe::Keyboard::Key_1;
    symMapping[XK_at] = Dynacoe::Keyboard::Key_2;
    symMapping[XK_numbersign] = Dynacoe::Keyboard::Key_3;
    symMapping[XK_dollar] = Dynacoe::Keyboard::Key_4;
    symMapping[XK_percent] = Dynacoe::Keyboard::Key_5;
    symMapping[XK_asciicircum] = Dynacoe::Keyboard::Key_6;
    symMapping[XK_ampersand] = Dynacoe::Keyboard::Key_7;
    symMapping[XK_asterisk] = Dynacoe::Keyboard::Key_8;
    symMapping[XK_parenleft] = Dynacoe::Keyboard::Key_9;

    

    symMapping[XK_a] = Dynacoe::Keyboard::Key_a;
    symMapping[XK_b] = Dynacoe::Keyboard::Key_b;
    symMapping[XK_c] = Dynacoe::Keyboard::Key_c;
    symMapping[XK_d] = Dynacoe::Keyboard::Key_d;
    symMapping[XK_e] = Dynacoe::Keyboard::Key_e;
    symMapping[XK_f] = Dynacoe::Keyboard::Key_f;
    symMapping[XK_g] = Dynacoe::Keyboard::Key_g;
    symMapping[XK_h] = Dynacoe::Keyboard::Key_h;
    symMapping[XK_i] = Dynacoe::Keyboard::Key_i;
    symMapping[XK_j] = Dynacoe::Keyboard::Key_j;

    symMapping[XK_k] = Dynacoe::Keyboard::Key_k;
    symMapping[XK_l] = Dynacoe::Keyboard::Key_l;
    symMapping[XK_m] = Dynacoe::Keyboard::Key_m;
    symMapping[XK_n] = Dynacoe::Keyboard::Key_n;
    symMapping[XK_o] = Dynacoe::Keyboard::Key_o;
    symMapping[XK_p] = Dynacoe::Keyboard::Key_p;
    symMapping[XK_q] = Dynacoe::Keyboard::Key_q;
    symMapping[XK_r] = Dynacoe::Keyboard::Key_r;
    symMapping[XK_s] = Dynacoe::Keyboard::Key_s;
    symMapping[XK_t] = Dynacoe::Keyboard::Key_t;

    symMapping[XK_u] = Dynacoe::Keyboard::Key_u;
    symMapping[XK_v] = Dynacoe::Keyboard::Key_v;
    symMapping[XK_w] = Dynacoe::Keyboard::Key_w;
    symMapping[XK_x] = Dynacoe::Keyboard::Key_x;
    symMapping[XK_y] = Dynacoe::Keyboard::Key_y;
    symMapping[XK_z] = Dynacoe::Keyboard::Key_z;

    symMapping[XK_A] = Dynacoe::Keyboard::Key_a;
    symMapping[XK_B] = Dynacoe::Keyboard::Key_b;
    symMapping[XK_C] = Dynacoe::Keyboard::Key_c;
    symMapping[XK_D] = Dynacoe::Keyboard::Key_d;
    symMapping[XK_E] = Dynacoe::Keyboard::Key_e;
    symMapping[XK_F] = Dynacoe::Keyboard::Key_f;
    symMapping[XK_G] = Dynacoe::Keyboard::Key_g;
    symMapping[XK_H] = Dynacoe::Keyboard::Key_h;
    symMapping[XK_I] = Dynacoe::Keyboard::Key_i;
    symMapping[XK_J] = Dynacoe::Keyboard::Key_j;

    symMapping[XK_K] = Dynacoe::Keyboard::Key_k;
    symMapping[XK_L] = Dynacoe::Keyboard::Key_l;
    symMapping[XK_M] = Dynacoe::Keyboard::Key_m;
    symMapping[XK_N] = Dynacoe::Keyboard::Key_n;
    symMapping[XK_O] = Dynacoe::Keyboard::Key_o;
    symMapping[XK_P] = Dynacoe::Keyboard::Key_p;
    symMapping[XK_Q] = Dynacoe::Keyboard::Key_q;
    symMapping[XK_R] = Dynacoe::Keyboard::Key_r;
    symMapping[XK_S] = Dynacoe::Keyboard::Key_s;
    symMapping[XK_T] = Dynacoe::Keyboard::Key_t;

    symMapping[XK_U] = Dynacoe::Keyboard::Key_u;
    symMapping[XK_V] = Dynacoe::Keyboard::Key_v;
    symMapping[XK_W] = Dynacoe::Keyboard::Key_w;
    symMapping[XK_X] = Dynacoe::Keyboard::Key_x;
    symMapping[XK_Y] = Dynacoe::Keyboard::Key_y;
    symMapping[XK_Z] = Dynacoe::Keyboard::Key_z;




    //symMapping[XK_Tab<<1



    symMapping[XK_Shift_L] = Dynacoe::Keyboard::Key_lshift;
    symMapping[XK_Shift_R] = Dynacoe::Keyboard::Key_rshift;
    symMapping[XK_Control_L] = Dynacoe::Keyboard::Key_lctrl;
    symMapping[XK_Control_R] = Dynacoe::Keyboard::Key_rctrl;
    symMapping[XK_Alt_L] = Dynacoe::Keyboard::Key_lalt;
    symMapping[XK_Alt_R] = Dynacoe::Keyboard::Key_ralt;
    symMapping[XK_Tab] = Dynacoe::Keyboard::Key_tab;
    symMapping[XK_ISO_Left_Tab] = Dynacoe::Keyboard::Key_tab;


    symMapping[XK_F1] = Dynacoe::Keyboard::Key_F1;
    symMapping[XK_F2] = Dynacoe::Keyboard::Key_F2;
    symMapping[XK_F3] = Dynacoe::Keyboard::Key_F3;
    symMapping[XK_F4] = Dynacoe::Keyboard::Key_F4;
    symMapping[XK_F5] = Dynacoe::Keyboard::Key_F5;
    symMapping[XK_F6] = Dynacoe::Keyboard::Key_F6;
    symMapping[XK_F7] = Dynacoe::Keyboard::Key_F7;
    symMapping[XK_F8] = Dynacoe::Keyboard::Key_F8;
    symMapping[XK_F9] = Dynacoe::Keyboard::Key_F9;

    symMapping[XK_F10] = Dynacoe::Keyboard::Key_F10;
    symMapping[XK_F11] = Dynacoe::Keyboard::Key_F11;
    symMapping[XK_F12] = Dynacoe::Keyboard::Key_F12;
    symMapping[XK_Up] = Dynacoe::Keyboard::Key_up;
    symMapping[XK_Down] = Dynacoe::Keyboard::Key_down;
    symMapping[XK_Left] = Dynacoe::Keyboard::Key_left;
    symMapping[XK_Right] = Dynacoe::Keyboard::Key_right;
    symMapping[XK_minus] = Dynacoe::Keyboard::Key_minus;
    symMapping[XK_underscore] = Dynacoe::Keyboard::Key_minus;    
    symMapping[XK_equal] = Dynacoe::Keyboard::Key_equal;
    symMapping[XK_plus] = Dynacoe::Keyboard::Key_equal;

    symMapping[XK_BackSpace] = Dynacoe::Keyboard::Key_backspace;
    symMapping[XK_grave] = Dynacoe::Keyboard::Key_grave;
    symMapping[XK_asciitilde] = Dynacoe::Keyboard::Key_apostrophe;
    symMapping[XK_Escape] = Dynacoe::Keyboard::Key_esc;
    symMapping[XK_Home] = Dynacoe::Keyboard::Key_home;
    symMapping[XK_Page_Up] = Dynacoe::Keyboard::Key_pageUp;
    symMapping[XK_Page_Down] = Dynacoe::Keyboard::Key_pageDown;
    symMapping[XK_End] = Dynacoe::Keyboard::Key_end;
    symMapping[XK_backslash] = Dynacoe::Keyboard::Key_backslash;
    symMapping[XK_bar] = Dynacoe::Keyboard::Key_backslash;
    symMapping[XK_bracketleft] = Dynacoe::Keyboard::Key_lbracket;
    symMapping[XK_bracketright] = Dynacoe::Keyboard::Key_rbracket;
    symMapping[XK_braceleft] = Dynacoe::Keyboard::Key_lbracket;
    symMapping[XK_braceright] = Dynacoe::Keyboard::Key_rbracket;


    symMapping[XK_semicolon] = Dynacoe::Keyboard::Key_semicolon;
    symMapping[XK_colon] = Dynacoe::Keyboard::Key_semicolon;
    symMapping[XK_apostrophe] = Dynacoe::Keyboard::Key_apostrophe;
    symMapping[XK_quotedbl] = Dynacoe::Keyboard::Key_apostrophe;
    symMapping[XK_slash] = Dynacoe::Keyboard::Key_frontslash;
    symMapping[XK_question] = Dynacoe::Keyboard::Key_frontslash;
    symMapping[XK_Return] = Dynacoe::Keyboard::Key_enter;
    symMapping[XK_Delete] = Dynacoe::Keyboard::Key_delete;

    symMapping[XK_KP_0] = Dynacoe::Keyboard::Key_numpad0;
    symMapping[XK_KP_1] = Dynacoe::Keyboard::Key_numpad1;
    symMapping[XK_KP_2] = Dynacoe::Keyboard::Key_numpad2;
    symMapping[XK_KP_3] = Dynacoe::Keyboard::Key_numpad3;
    symMapping[XK_KP_4] = Dynacoe::Keyboard::Key_numpad4;
    symMapping[XK_KP_5] = Dynacoe::Keyboard::Key_numpad5;
    symMapping[XK_KP_6] = Dynacoe::Keyboard::Key_numpad6;
    symMapping[XK_KP_7] = Dynacoe::Keyboard::Key_numpad7;
    symMapping[XK_KP_8] = Dynacoe::Keyboard::Key_numpad8;
    symMapping[XK_KP_9] = Dynacoe::Keyboard::Key_numpad9;

    //TODO: missing printscreen!
    //symMapping[XK_0] = Dynacoe::Keyboard::Key_prtscr;
    symMapping[XK_Super_L] = Dynacoe::Keyboard::Key_lsuper;
    symMapping[XK_Super_R] = Dynacoe::Keyboard::Key_rsuper;
    symMapping[XK_space] = Dynacoe::Keyboard::Key_space;
    symMapping[XK_Insert] = Dynacoe::Keyboard::Key_insert;
    symMapping[XK_comma] = Dynacoe::Keyboard::Key_comma;
    symMapping[XK_less] = Dynacoe::Keyboard::Key_comma;
    symMapping[XK_period] = Dynacoe::Keyboard::Key_period;
    symMapping[XK_greater] = Dynacoe::Keyboard::Key_period;




    mbMapping[1] = Dynacoe::MouseButtons::Left;
    mbMapping[2] = Dynacoe::MouseButtons::Middle;
    mbMapping[3] = Dynacoe::MouseButtons::Right;
    

}


#include <iostream>
#include <cstdio>
void handleEvent(std::vector<Dynacoe::InputDevice *> & devices, XEvent event) {
    devices[1]->axes[(int)Dynacoe::MouseAxes::Wheel] = 0;

    switch(event.type) {
      case KeyPress:
      case KeyRelease: {
        KeySym key;
        XKeyEvent * ev = (XKeyEvent*)&event;
        XLookupString(ev, (char*)"", 0, &key, nullptr);

        // update keyboard        
        devices[0]->buttons[(int)symMapping[key]] = event.type == KeyPress;
      } break;



      case ButtonPress:
      case ButtonRelease: {
        XButtonEvent * ev = (XButtonEvent*)&event;
        switch(ev->button) {
            case 1:
            case 2:
            case 3:
                devices[1]->buttons[(int)mbMapping[ev->button]] = event.type == ButtonPress;
                break;
            
            case 4:
            case 5:
                devices[1]->axes[(int)Dynacoe::MouseAxes::Wheel] = (ev->button == 4 ? 1 : -1);
                break;
        }
      } break;

      case MotionNotify: {
        XMotionEvent * ev = (XMotionEvent*)&event;
        
        devices[1]->axes[(int)Dynacoe::MouseAxes::X] = ev->x;
        devices[1]->axes[(int)Dynacoe::MouseAxes::Y] = ev->y;
        
        
      } break;



      default:;

    }
}

#endif


