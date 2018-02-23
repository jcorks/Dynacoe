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

#include <Dynacoe/Backends/InputManager/NoInput_Multi.h> 

using namespace std;
using namespace Dynacoe;



bool NoInputManager::IsSupported(InputManager::InputType){return false;}
bool NoInputManager::HandleEvents(){return false;}
InputDevice * NoInputManager::QueryDevice(int i) {return nullptr;}
InputDevice * NoInputManager::QueryDevice(DefaultDeviceSlots i) {return nullptr;}
int NoInputManager::QueryAuxiliaryDevices(int *){return 0;}
int NoInputManager::MaxDevices(){return 0;}
void NoInputManager::SetFocus(Display*){}
Display * NoInputManager::GetFocus(){return nullptr;}


bool NoInputManager::Valid(){return true; }
std::string NoInputManager::Name() { return "Dummy Input"; }
std::string NoInputManager::Version() { return "v1.0"; }



