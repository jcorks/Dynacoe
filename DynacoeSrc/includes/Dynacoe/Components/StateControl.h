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

#ifndef H_DC_STATE_CONTROL_COMPONENT_INCLUDED
#define H_DC_STATE_CONTROL_COMPONENT_INCLUDED

#include <Dynacoe/Component.h>
#include <string>
#include <map>

namespace Dynacoe {

/// \brief Adds a state machine mechanism to the entity.
///
/// Each StateControl instance is effectively a dynamic state machine.
/// After desgnating what states exist within the machine using
/// CreateState(), the machine can be run using Execute(). Using each state's
/// StepFunc, the states will transition until an invalid state is returned. Referring
/// to an invalid state halts the state machine.
///
/// It is also entirely valid to modify the state machine mid-execution.
class StateControl : public Component {
  public:
    StateControl();


    /// \brief Designates execution instructions for a state.
    ///
    /// During each state, different logic is disgnated to run to control
    /// what happens during this state and when the state should be altered.
    /// The StepFunc of the state loop is is meant to hold the bulk of the logic. 
    /// The DrawFunc can be used to produce visual effects associated with the state.
    /// The state may be changed any time using Execute.
    ///
    struct StateLoop {

        /// \brief Creates a new StateLoop
        ///  
        StateLoop(Component::EventHandler step=nullptr, 
                  Component::EventHandler draw=nullptr, 
                  Component::EventHandler init=nullptr,
                  void * data_=nullptr) : Step(step), Draw(draw), Init(init), data(data_){}

        Component::EventHandler Step;
        Component::EventHandler Draw;
        Component::EventHandler Init;
        void * data;

        bool operator==(const StateLoop & other) {
            return (Step == other.Step &&
                    Draw == other.Draw &&
                    Init == other.Init && 
                    data == other.data);
        }

    };
    
    /// \brief Sets up a new valid state.
    ///
    /// CreateState sets this StateLoop as the only one associated 
    /// with the state ofthe given name.
    void CreateState(const std::string & tag, StateLoop);


    /// \brief Removes the state.
    ///
    /// No action is taken if the tag does not refer to a valid state.
    void RemoveState(const std::string & tag);

    /// \brief Begins the state machine execution loop from the given state.
    ///
    /// If begin state does not refer to a valid state, no action is taken.
    /// If done in the middle of a state loop function, any queued 
    /// function calls are cancelled.
    void Execute(const std::string & beginState);

    /// \brief stops all current execution.
    ///
    /// If done during a state loop, all remaining state loop functions 
    /// that would have executed are cancelled.
    void Halt();

    /// \brief Returns whether or not the machine is in a halted state.
    ///
    bool IsHalted();

    /// \brief Returns the current state tag. If no execution state is active, the empty string is returned.
    ///
    std::string GetCurrentState();



    void OnStep();
    void OnDraw();
    std::string GetInfo();
  private:
    std::map<std::string, StateLoop> states;
    std::string current;
    StateLoop currentData;
    bool hasCurrent;
    bool midTerminate;
    bool queuedInit;
};
}

#endif
