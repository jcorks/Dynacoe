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

#ifndef H_DC_DEBUGGER_INCLUDED
#define H_DC_DEBUGGER_INCLUDED

#include <Dynacoe/Modules/Module.h>
#include <Dynacoe/Entity.h>

namespace Dynacoe {

/// \brief Helps identify problems with the program.
///
/// The debugger has very few interactive functions, as it is meant to mostly
/// be controlled by the user while running the program. The main use is to
/// explore the Entity / World hierarchy and view detailed information on Entities.
class Debugger : public Module {
  public:

    /// \brief Adds a new DebugWindow set to the Entity given. If the EntityID
    /// is not valid, the window is open to the topmost level of the hierarchy.
    ///
    static void ViewID(Entity::ID){}


  private:

    Entity::ID debuggerBase;





  public:
    std::string GetName() { return "Debugger";}
    void Init(); void InitAfter(); void RunBefore(); void RunAfter(); void DrawBefore(); void DrawAfter();
    Backend * GetBackend();
};
}


#endif
