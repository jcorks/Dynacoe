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

#ifdef DC_BACKENDS_WININPUT_WIN32

#include <Dynacoe/Backends/InputManager/WinInput_Win32.h>
#include <Dynacoe/Backends/InputManager/InputDevice.h>
#include <Dynacoe/Backends/Display/Display.h>
#include <stack>
#include <cstdlib>
#include <cfloat>
#include <unordered_map>

/* WinAPI backend for input handling. */

#include <windows.h>

static int TranslateWINAPIKeyToDCKey(int k) {
    switch(k) {
        case 0x30: return Dynacoe::UserInput::Key_0;
        case 0x31: return Dynacoe::UserInput::Key_1;
        case 0x32: return Dynacoe::UserInput::Key_2;
        case 0x33: return Dynacoe::UserInput::Key_3;
        case 0x34: return Dynacoe::UserInput::Key_4;
        case 0x35: return Dynacoe::UserInput::Key_5;
        case 0x36: return Dynacoe::UserInput::Key_6;
        case 0x37: return Dynacoe::UserInput::Key_7;
        case 0x38: return Dynacoe::UserInput::Key_8;
        case 0x39: return Dynacoe::UserInput::Key_9;


        case 0x41: return Dynacoe::UserInput::Key_a;
        case 0x42: return Dynacoe::UserInput::Key_b;
        case 0x43: return Dynacoe::UserInput::Key_c;
        case 0x44: return Dynacoe::UserInput::Key_d;
        case 0x45: return Dynacoe::UserInput::Key_e;
        case 0x46: return Dynacoe::UserInput::Key_f;
        case 0x47: return Dynacoe::UserInput::Key_g;
        case 0x48: return Dynacoe::UserInput::Key_h;
        case 0x49: return Dynacoe::UserInput::Key_i;

        case 0x4A: return Dynacoe::UserInput::Key_j;
        case 0x4B: return Dynacoe::UserInput::Key_k;
        case 0x4C: return Dynacoe::UserInput::Key_l;
        case 0x4D: return Dynacoe::UserInput::Key_m;
        case 0x4E: return Dynacoe::UserInput::Key_n;
        case 0x4F: return Dynacoe::UserInput::Key_o;
        case 0x50: return Dynacoe::UserInput::Key_p;
        case 0x51: return Dynacoe::UserInput::Key_q;
        case 0x52: return Dynacoe::UserInput::Key_r;
        case 0x53: return Dynacoe::UserInput::Key_s;
        case 0x54: return Dynacoe::UserInput::Key_t;
        case 0x55: return Dynacoe::UserInput::Key_u;

        case 0x56: return Dynacoe::UserInput::Key_v;
        case 0x57: return Dynacoe::UserInput::Key_w;
        case 0x58: return Dynacoe::UserInput::Key_x;
        case 0x59: return Dynacoe::UserInput::Key_y;
        case 0x5A: return Dynacoe::UserInput::Key_z;

        case VK_LSHIFT: return Dynacoe::UserInput::Key_lshift;
        case VK_RSHIFT: return Dynacoe::UserInput::Key_rshift;

        case VK_LCONTROL: return Dynacoe::UserInput::Key_lctrl;
        case VK_RCONTROL: return Dynacoe::UserInput::Key_rctrl;

        case VK_LMENU: return Dynacoe::UserInput::Key_lalt;
        case VK_RMENU: return Dynacoe::UserInput::Key_ralt;

        case VK_TAB: return Dynacoe::UserInput::Key_tab;

        case VK_F1: return Dynacoe::UserInput::Key_F1;
        case VK_F2: return Dynacoe::UserInput::Key_F2;
        case VK_F3: return Dynacoe::UserInput::Key_F3;
        case VK_F4: return Dynacoe::UserInput::Key_F4;
        case VK_F5: return Dynacoe::UserInput::Key_F5;
        case VK_F6: return Dynacoe::UserInput::Key_F6;
        case VK_F7: return Dynacoe::UserInput::Key_F7;
        case VK_F8: return Dynacoe::UserInput::Key_F8;
        case VK_F9: return Dynacoe::UserInput::Key_F9;
        case VK_F10: return Dynacoe::UserInput::Key_F10;
        case VK_F11: return Dynacoe::UserInput::Key_F11;
        case VK_F12: return Dynacoe::UserInput::Key_F12;


        case VK_UP: return Dynacoe::UserInput::Key_up;
        case VK_DOWN: return Dynacoe::UserInput::Key_down;
        case VK_LEFT: return Dynacoe::UserInput::Key_left;
        case VK_RIGHT: return Dynacoe::UserInput::Key_right;

        case VK_OEM_MINUS: return Dynacoe::UserInput::Key_minus;
        case VK_OEM_PLUS: return Dynacoe::UserInput::Key_equal;
        case VK_BACK: return Dynacoe::UserInput::Key_backspace;
        case VK_OEM_3: return Dynacoe::UserInput::Key_grave;

        case VK_ESCAPE: return Dynacoe::UserInput::Key_esc;
        case VK_HOME: return Dynacoe::UserInput::Key_home;
        case VK_PRIOR: return Dynacoe::UserInput::Key_pageUp;
        case VK_NEXT: return Dynacoe::UserInput::Key_pageDown;
        case VK_END: return Dynacoe::UserInput::Key_end;
        case VK_OEM_102: return Dynacoe::UserInput::Key_backslash;

        case VK_OEM_4: return Dynacoe::UserInput::Key_lbracket;
        case VK_OEM_6: return Dynacoe::UserInput::Key_rbracket;
        case VK_OEM_1: return Dynacoe::UserInput::Key_semicolon;
        case VK_OEM_7: return Dynacoe::UserInput::Key_apostrophe;
        case VK_OEM_2: return Dynacoe::UserInput::Key_frontslash;

        case VK_RETURN: return Dynacoe::UserInput::Key_enter;
        case VK_DELETE: return Dynacoe::UserInput::Key_delete;

        case VK_NUMPAD0: return Dynacoe::UserInput::Key_numpad0;
        case VK_NUMPAD1: return Dynacoe::UserInput::Key_numpad1;
        case VK_NUMPAD2: return Dynacoe::UserInput::Key_numpad2;
        case VK_NUMPAD3: return Dynacoe::UserInput::Key_numpad3;
        case VK_NUMPAD4: return Dynacoe::UserInput::Key_numpad4;
        case VK_NUMPAD5: return Dynacoe::UserInput::Key_numpad5;
        case VK_NUMPAD6: return Dynacoe::UserInput::Key_numpad6;
        case VK_NUMPAD7: return Dynacoe::UserInput::Key_numpad7;
        case VK_NUMPAD8: return Dynacoe::UserInput::Key_numpad8;
        case VK_NUMPAD9: return Dynacoe::UserInput::Key_numpad9;

        case VK_SNAPSHOT: return Dynacoe::UserInput::Key_prtscr;
        case VK_LWIN: return Dynacoe::UserInput::Key_lsuper;
        case VK_RWIN: return Dynacoe::UserInput::Key_rsuper;

        case VK_SPACE: return Dynacoe::UserInput::Key_space;
        case VK_INSERT: return Dynacoe::UserInput::Key_insert;
        case VK_OEM_COMMA: return Dynacoe::UserInput::Key_comma;
        case VK_OEM_PERIOD: return Dynacoe::UserInput::Key_period;


        default: 
            return Dynacoe::UserInput::Count + k;



    }

    return Dynacoe::UserInput::Count + k;
}



class WinAPIDevice {
  public:
    WinAPIDevice(Dynacoe::InputDevice::Class c) {
        device = new Dynacoe::InputDevice(c);
        jid = -1;
        connected = false;
    }

    void HandleMessage(int inputKey, float inputValue, int unicode) {
        Dynacoe::InputDevice::Event ev;
        ev.id = inputKey;
        ev.state = inputValue;
        ev.utf8 = unicode;

        printf("%d %f %d\n", inputKey, inputValue, unicode);
        fflush(stdout);

        device->PushEvent(ev);
    }


    int GetJoyID() {
        return jid;
    }

    void SetJoyID(int id) {
        jid = id;
    }

    bool IsJoyIDConnected() {
        return connected;
    }

    void SetJoyIDConnected(bool b) {
        connected = b;
    }

    Dynacoe::InputDevice * GetDevice() {
        return device;
    }

  private:
    Dynacoe::InputDevice* device;
    int jid;
    bool connected;
};


static std::vector<WinAPIDevice*> devices;
Dynacoe::WinInputManager::WinInputManager() {
    devices.push_back(new WinAPIDevice(Dynacoe::InputDevice::Class::Keyboard));
    devices.push_back(new WinAPIDevice(Dynacoe::InputDevice::Class::Pointer));
    devices.push_back(new WinAPIDevice(Dynacoe::InputDevice::Class::Pointer));

    devices.push_back(new WinAPIDevice(Dynacoe::InputDevice::Class::Gamepad));
    devices.push_back(new WinAPIDevice(Dynacoe::InputDevice::Class::Gamepad));
    devices.push_back(new WinAPIDevice(Dynacoe::InputDevice::Class::Gamepad));
    devices.push_back(new WinAPIDevice(Dynacoe::InputDevice::Class::Gamepad));




}



bool Dynacoe::WinInputManager::Valid(){return true; }
std::string Dynacoe::WinInputManager::Name() { return "WinAPI Input"; }
std::string Dynacoe::WinInputManager::Version() { return "v1.0"; }


bool Dynacoe::WinInputManager::IsSupported(InputType){return true;}


static BYTE lastKeyboardState[256];
static std::unordered_map<Dynacoe::Display *, bool> hasRegisteredJoypads;
bool Dynacoe::WinInputManager::HandleEvents() {




    // Need the window's display to deal with input
    
    if (!display) {return false;}
        

    MSG msg;
    std::vector<MSG> * ev = (std::vector<MSG>*)display->GetLastSystemEvent();

    if (ev->size()) {
        int num = joyGetNumDevs();

        bool claimedDevs[num] = {};

        // get existing joypad updates
        for(int i = 3; i < devices.size(); ++i) {
            if (devices[i]->GetJoyID() == -1) continue;

            JOYINFOEX jp = {};
            jp.dwSize = sizeof(JOYINFOEX);
            jp.dwFlags = JOY_RETURNALL;

            int result;

            // if its connected:
            //   - check if its still connected
            //   - mark it as claimed

            if ((result = joyGetPosEx(devices[i]->GetJoyID(), &jp)) != JOYERR_NOERROR) {
                if (devices[i]->IsJoyIDConnected()) {
                    printf("Slot %d disconnected\n", i);
                    fflush(stdout);
                    devices[i]->SetJoyIDConnected(false);
                }
            } else if (!devices[i]->IsJoyIDConnected()) {
                // reconnected!!
                devices[i]->SetJoyIDConnected(true);
                printf("Slot %d reconnected\n");
            }

            claimedDevs[i] = true;

        }

        // look for 
        for(int i = 0; i < 16; ++i) {
            if (claimedDevs[i]) continue;

            JOYINFOEX jp = {};
            jp.dwSize = sizeof(JOYINFOEX);
            jp.dwFlags = JOY_RETURNALL;
            int result;
            if ((result = joyGetPosEx(i, &jp)) == JOYERR_NOERROR) {
                for(int n = 3; n < devices.size(); ++n) {
                    if (devices[n]->GetJoyID() == -1) {
                        if(joySetCapture(((*ev)[0]).hwnd, i, 0, FALSE)) {
                            printf("Could not capture ID %d\n", i);
                            fflush(stdout);
                            devices[n]->SetJoyID(i);
                        } else {
                            printf("captured ID %d -> gamepad %d\n", i, n-3);
                        }

                    }
                }
            } else {
                /*
                printf("No info or availability for slot %d\n", i, result);
                switch(result) {
                  case MMSYSERR_NODRIVER:    printf("(MMSYSERR_NODRIVER)\n"); break;
                  case MMSYSERR_INVALPARAM:  printf("(MMSYSERR_INVALPARAM)\n"); break;
                  case MMSYSERR_BADDEVICEID: printf("(MMSYSERR_BADDEVICEID)\n"); break;

                  case JOYERR_UNPLUGGED: printf("(UNPLUGGED)\n"); break;
                  case JOYERR_PARMS: printf("(PARMS)\n"); break;


                }
                */
            }

        }

            fflush(stdout);
            


    }

    for(uint32_t i = 0; i < ev->size(); ++i) {
        MSG * msg = &(*ev)[i];

        switch(msg->message) {

          // keyboard!
          case WM_KEYUP:
          case WM_KEYDOWN: {

            GetKeyboardState(lastKeyboardState);
            
            WORD ch = 0;
            if (!ToAscii(
                msg->wParam,
                (msg->lParam >> 16) & 0x00ff,
                lastKeyboardState,
                &ch,
                0
            )) {
                ch = 0;
            }


            devices[0]->HandleMessage(
                TranslateWINAPIKeyToDCKey(msg->wParam), 
                msg->message == WM_KEYUP ? 1.f : 0.f, 
                ch
            );
            break;
          }
          
          // pads!
          case MM_JOY1MOVE: 
          case MM_JOY1BUTTONDOWN: 
          case MM_JOY1BUTTONUP:
          {

            printf("JPID 1\n");
            fflush(stdout);
            break;
          }

          case MM_JOY2MOVE: 
          case MM_JOY2BUTTONDOWN: 
          case MM_JOY2BUTTONUP:
          {

            printf("JPID 2\n");
            fflush(stdout);
            break;
          }


          default: {
              printf("%d \n", msg->message);
              fflush(stdout);
          }

        }
    }


    return true;
}


Dynacoe::InputDevice * Dynacoe::WinInputManager::QueryDevice(int ID) {
    if (ID < 0 || ID >= devices.size()) {
        return nullptr;
    }
    return devices[ID]->GetDevice();
}

Dynacoe::InputDevice * Dynacoe::WinInputManager::QueryDevice(DefaultDeviceSlots slot) {
    return QueryDevice(static_cast<int>(slot));
}



int Dynacoe::WinInputManager::QueryAuxiliaryDevices(int * IDs) {
    return 0;
}


int Dynacoe::WinInputManager::MaxDevices() {
    return (int)DefaultDeviceSlots::NumDefaultDevices;
}



void Dynacoe::WinInputManager::SetFocus(Dynacoe::Display * d) {
    display = d;


}

Dynacoe::Display * Dynacoe::WinInputManager::GetFocus() {
    return display;
}





#endif

