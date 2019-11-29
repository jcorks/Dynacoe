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
#include <windowsx.h>

#include <XInput.h>
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

static std::unordered_map<int, int> xinput_to_dc;

class WinAPIDevice {
  public:
    WinAPIDevice(Dynacoe::InputDevice::Class c) : padState({}) {
        device = new Dynacoe::InputDevice(c);
        jid = -1;
        connected = false;
        padState = {};
        prevState = {};
        padState.dwPacketNumber = -1;
        prevState.dwPacketNumber = -1;

        if (c == Dynacoe::InputDevice::Class::Gamepad) {
            device->SetDeadzone(Dynacoe::UserInput::Pad_axisY2, .3);
            device->SetDeadzone(Dynacoe::UserInput::Pad_axisX2, .3);
            device->SetDeadzone(Dynacoe::UserInput::Pad_axisY3, .3);
            device->SetDeadzone(Dynacoe::UserInput::Pad_axisX3, .3);

        }

    }

    void HandleMessage(int inputKey, float inputValue, int unicode=0) {
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
        prevState.dwPacketNumber = -1; // repoll input when reconnected
    }


    #define DXDELTA(__X__) ((prevState.Gamepad.wButtons & __X__) != (padState.Gamepad.wButtons & __X__))
    void UpdateButtonPad(int pos, int neg) {
        if (DXDELTA(pos) ||
            DXDELTA(neg)) {
            int dc = xinput_to_dc[pos];

            if (padState.Gamepad.wButtons & pos) {
                HandleMessage(dc, 1.f);
            } else if (padState.Gamepad.wButtons & neg) {
                HandleMessage(dc, -1.f);
            } else {
                HandleMessage(dc, 0.f);
            }
        }
    }

    void UpdateAxis(int bt, float val, float min, float max) {
        HandleMessage(bt, 2*((val - min) / (max - min))-1);
    }
    #define DXAXISDELTA(__X__, __ID__, __MIN__, __MAX__) if(prevState.Gamepad.__X__ != padState.Gamepad.__X__) UpdateAxis(__ID__, padState.Gamepad.__X__, __MIN__, __MAX__);

    void UpdateButton(int bt) {
        if (DXDELTA(bt)) HandleMessage(xinput_to_dc[bt], padState.Gamepad.wButtons & bt ? 1.f : 0.f);
    }

    void UpdatePadValues() {
        XInputGetState(GetJoyID(), &padState);
        if (padState.dwPacketNumber != prevState.dwPacketNumber) {
            // add events for each change
            UpdateButtonPad(XINPUT_GAMEPAD_DPAD_UP,    XINPUT_GAMEPAD_DPAD_DOWN);
            UpdateButtonPad(XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_LEFT);

            UpdateButton(XINPUT_GAMEPAD_START);
            UpdateButton(XINPUT_GAMEPAD_BACK);

            UpdateButton(XINPUT_GAMEPAD_LEFT_THUMB);
            UpdateButton(XINPUT_GAMEPAD_RIGHT_THUMB);

            UpdateButton(XINPUT_GAMEPAD_LEFT_SHOULDER);
            UpdateButton(XINPUT_GAMEPAD_RIGHT_SHOULDER);


            UpdateButton(XINPUT_GAMEPAD_A);
            UpdateButton(XINPUT_GAMEPAD_B);
            UpdateButton(XINPUT_GAMEPAD_X);
            UpdateButton(XINPUT_GAMEPAD_Y);

            DXAXISDELTA(bLeftTrigger,  Dynacoe::UserInput::Pad_l2, 0, 255);
            DXAXISDELTA(bRightTrigger, Dynacoe::UserInput::Pad_r2, 0, 255);

            DXAXISDELTA(sThumbLX, Dynacoe::UserInput::Pad_axisX2, -0xffff/2, 0xffff/2);
            DXAXISDELTA(sThumbLY, Dynacoe::UserInput::Pad_axisY2, -0xffff/2, 0xffff/2);

            DXAXISDELTA(sThumbRX, Dynacoe::UserInput::Pad_axisR2, -0xffff/2, 0xffff/2);
            DXAXISDELTA(sThumbRY, Dynacoe::UserInput::Pad_axisR2, -0xffff/2, 0xffff/2);

            prevState = padState;

            if (device->GetEventCount()) {
                printf("gamepad %d has %d new events\n", GetJoyID(), device->GetEventCount());
                fflush(stdout);
            }
        }        
        

    }

    Dynacoe::InputDevice * GetDevice() {
        return device;
    }

    // if connected, then the pad state is reliable
    XINPUT_STATE padState;
  private:
    XINPUT_STATE prevState;
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


    XInputEnable(TRUE);

    xinput_to_dc[XINPUT_GAMEPAD_DPAD_UP]     = Dynacoe::UserInput::Pad_axisY;
    xinput_to_dc[XINPUT_GAMEPAD_DPAD_DOWN]   = Dynacoe::UserInput::Pad_axisY;
    xinput_to_dc[XINPUT_GAMEPAD_DPAD_LEFT]   = Dynacoe::UserInput::Pad_axisX;
    xinput_to_dc[XINPUT_GAMEPAD_DPAD_RIGHT]  = Dynacoe::UserInput::Pad_axisX;

    xinput_to_dc[XINPUT_GAMEPAD_START]  = Dynacoe::UserInput::Pad_start;
    xinput_to_dc[XINPUT_GAMEPAD_BACK]  = Dynacoe::UserInput::Pad_select;
    xinput_to_dc[XINPUT_GAMEPAD_A]  = Dynacoe::UserInput::Pad_a;
    xinput_to_dc[XINPUT_GAMEPAD_B]  = Dynacoe::UserInput::Pad_b;
    xinput_to_dc[XINPUT_GAMEPAD_X]  = Dynacoe::UserInput::Pad_x;
    xinput_to_dc[XINPUT_GAMEPAD_Y]  = Dynacoe::UserInput::Pad_y;

    xinput_to_dc[XINPUT_GAMEPAD_LEFT_SHOULDER] = Dynacoe::UserInput::Pad_l;
    xinput_to_dc[XINPUT_GAMEPAD_RIGHT_SHOULDER] = Dynacoe::UserInput::Pad_r;
    xinput_to_dc[XINPUT_GAMEPAD_LEFT_THUMB] = Dynacoe::UserInput::Pad_l2;
    xinput_to_dc[XINPUT_GAMEPAD_RIGHT_THUMB] = Dynacoe::UserInput::Pad_r2;



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

    if (true) {
        bool claimedDevs[4] = {};

        // get existing joypad updates
        for(int i = 3; i < devices.size(); ++i) {
            if (devices[i]->GetJoyID() == -1) continue;


            int result;

            // if its connected:
            //   - check if its still connected
            //   - mark it as claimed

            if (XInputGetState(devices[i]->GetJoyID(), &devices[i]->padState) != ERROR_SUCCESS) {
                if (devices[i]->IsJoyIDConnected()) {
                    printf("Slot %d disconnected\n", i);
                    fflush(stdout);
                    devices[i]->SetJoyIDConnected(false);
                }
            } else if (!devices[i]->IsJoyIDConnected()) {
                // reconnected!!
                devices[i]->SetJoyIDConnected(true);
                printf("Slot %d reconnected\n");
                fflush(stdout);
            }

            claimedDevs[devices[i]->GetJoyID()] = true;

        }

        // look for 
        for(int i = 0; i < 4; ++i) {
            if (claimedDevs[i]) continue;


            int result;
            XINPUT_STATE state;
            if ((result = XInputGetState(i, &state)) == ERROR_SUCCESS) {
                for(int n = 3; n < devices.size(); ++n) {
                    if (devices[n]->GetJoyID() == -1) {
                        devices[n]->SetJoyID(i);
                        devices[n]->SetJoyIDConnected(true);
                        devices[n]->padState = state;
                        printf("captured ID %d -> gamepad %d\n", i, n-3);
                        fflush(stdout);
                        break;
                    }
                }
            } 

        }

        fflush(stdout);
        


    }
    for(int i = 3; i < devices.size(); ++i) {
        if (devices[i]->IsJoyIDConnected()) {
            devices[i]->UpdatePadValues();
        }
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
                msg->message == WM_KEYDOWN ? 1.f : 0.f, 
                ch
            );
            break;
          }



          // mouse
          case WM_MOUSEMOVE:
            devices[1]->HandleMessage(Dynacoe::UserInput::Pointer_X, GET_X_LPARAM(msg->lParam));
            devices[1]->HandleMessage(Dynacoe::UserInput::Pointer_Y, GET_Y_LPARAM(msg->lParam));
            break;

          case WM_LBUTTONUP:
            devices[1]->HandleMessage(Dynacoe::UserInput::Pointer_0, 0.f);
            break;

          case WM_LBUTTONDOWN:
            devices[1]->HandleMessage(Dynacoe::UserInput::Pointer_0, 1.f);
            break;

          case WM_RBUTTONUP:
            devices[1]->HandleMessage(Dynacoe::UserInput::Pointer_1, 0.f);
            break;

          case WM_RBUTTONDOWN:
            devices[1]->HandleMessage(Dynacoe::UserInput::Pointer_1, 1.f);
            break;

          case WM_MBUTTONUP:
            devices[1]->HandleMessage(Dynacoe::UserInput::Pointer_2, 0.f);
            break;

          case WM_MBUTTONDOWN:
            devices[1]->HandleMessage(Dynacoe::UserInput::Pointer_2, 1.f);
            break;


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

