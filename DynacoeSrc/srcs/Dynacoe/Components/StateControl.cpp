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

StateControl::StateControl(): Component("StateControl"){
    current = "";
}

void StateControl::CreateState(const std::string & tag, StateLoop loop) {
    std::vector<StateLoop> & localStates = states[tag];
    localStates.clear();
    localStates.push_back(loop);
}

void StateControl::ConnectState(const std::string & tag, StateLoop loop) {
    std::vector<StateLoop> & localStates = states[tag];
    localStates.push_back(loop);
}

void StateControl::RemoveState(const std::string & tag) {
    auto iter = states.find(tag);
    if (iter != states.end()) 
        states.erase(iter);
}

void StateControl::RemoveLoop(const std::string & tag, StateLoop loop) {
    auto iter = states.find(tag);
    if (iter != states.end()) {
        auto loops = iter->second;
        for(size_t i = 0; i < loops.size(); ++i) {
            if (loops[i] == loop) {
                loops.erase(loops.begin() + i);
                return;
            }
        }
    }
}



void StateControl::Execute(const std::string & tag) {
    auto iter = states.find(tag);
    if (iter == states.end()) return;
    current = tag;
    midTerminate = true;
    queuedInit = true;
}

void StateControl::Halt() {
    current = "";
    midTerminate = true;
}

bool StateControl::IsHalted() {
    return !current.size() || !GetHost() || current == "";
}

std::string StateControl::GetCurrentState() {
    return current;
}



void StateControl::OnStep() {
    if (IsHalted()) return;
    midTerminate = false;
    StateLoop * state;
    auto iter = states.find(current);

    
    // Looks like we ran on a state that was removed. Halt the machine.
    if (iter == states.end()) {
        current = "";
        return;
    }
    auto loops = iter->second;


    // if this is the first Step set of this state,
    // we need to step any init functions of the state loop.
    if (queuedInit) {
        for(int64_t i = loops.size()-1; i >= 0; --i) {
            state = &loops[i];
            if (state->Init) state->Init(state->data, this, GetHostID(), Entity::ID(), {});
            if (midTerminate) return;
        }
        queuedInit = false;      
    }



    // Go through all step funcs associated with the state 
    // Keep stepnign until one requests a state change    
    for(int64_t i = loops.size()-1; i >= 0; --i) {
        state = &loops[i];        

        // The stepFunc, while conventional, is not necessary. Merely skip  
        // if it DNE.
        if (!state->Step) {
            continue;
        }
        
        // execute all from the end of the state loop
        state->Step(state->data, this, GetHostID(), Entity::ID(), {});
        if (midTerminate) return;        
    }

}

void StateControl::OnDraw() {
    if (IsHalted()) return;
    midTerminate = true;

    auto iter = states.find(current);

    auto loops = iter->second;
    StateLoop * state;
    for(int64_t i = loops.size()-1; i >= 0; --i) {
        state = &loops[i];
        
        if (state->Draw) {
            state->Draw(state->data, this, GetHostID(), Entity::ID(), {});
            if (midTerminate) return;
        }        
        
    }
}

std::string StateControl::GetInfo() {
    return (Chain() << "State: " << current << " (" << (int)states.size() << " total)\n");
}
