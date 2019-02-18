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

#include <Dynacoe/Components/StateControl.h>

using namespace Dynacoe;

StateControl::StateControl(): Component(){
    SetTag("StateControl");
    hasCurrent = false;
    midTerminate = false;
    queuedInit = false;
    current = "";
}

void StateControl::CreateState(const std::string & tag, StateLoop loop) {
    states[tag] = loop;
    if (current == tag) {
        currentData = loop;
    }
}

void StateControl::RemoveState(const std::string & tag) {
    auto iter = states.find(tag);
    if (iter != states.end()) {
        states.erase(iter);
        if (current == tag) {
            midTerminate = true;
            hasCurrent = false;
            
        }
    }
}



void StateControl::Execute(const std::string & tag) {
    auto iter = states.find(tag);
    if (iter == states.end()) return;
    current = tag;
    currentData = iter->second;
    midTerminate = false;
    hasCurrent = true;
    queuedInit = true;
}

void StateControl::Halt() {
    current = "";
    hasCurrent = false;
    midTerminate = true;
}

bool StateControl::IsHalted() {
    return !current.size() || !GetHost() || !hasCurrent;
}

std::string StateControl::GetCurrentState() {
    return current;
}



void StateControl::OnStep() {
    if (IsHalted()) return;
    StateLoop * state = &currentData;


    // if this is the first Step set of this state,
    // we need to step any init functions of the state loop.
    if (queuedInit) {
        queuedInit = false;      
        if (state->Init) {
            state->Init(state->data, this, GetHostID(), Entity::ID(), {});
        }
        if (midTerminate) return;
    }




    // The stepFunc, while conventional, is not necessary. Merely skip  
    // if it DNE.
    if (state->Step) {
        state->Step(state->data, this, GetHostID(), Entity::ID(), {});        
    }
        

}

void StateControl::OnDraw() {
    if (IsHalted() || !currentData.Draw) return;

    currentData.Draw(currentData.data, this, GetHostID(), Entity::ID(), {});
}

std::string StateControl::GetInfo() {
    return (Chain() << "State: " << current << " (" << (int)states.size() << " total)\n");
}
