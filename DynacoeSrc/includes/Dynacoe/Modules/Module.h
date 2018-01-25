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

#ifndef H_DC_MODULE_INCLUDED
#define H_DC_MODULE_INCLUDED

#include <string>

namespace Dynacoe {
class Backend;

class Module {
  public:
    // Happens when the Engine first starts up.
    // If returns false, the engine halts and exits
    virtual void Init() = 0;
    
    // Happens after all the Module Init()s occur
    virtual void InitAfter() = 0;

    // Runs prior to running entities, this runs
    virtual void RunBefore() = 0;
    
    // Runs after all entities have fun
    virtual void RunAfter() = 0;

    // Draws before all entities have (super non-useful)
    virtual void DrawBefore() = 0;

    // Draws after all entities have (super useful)
    virtual void DrawAfter() = 0;

    // Returns a string that describes the module in one or two words.
    virtual std::string GetName() = 0;
    
    // Returns the backend by which lower-level operations are processed.
    virtual Backend * GetBackend() = 0;
    

};
}

#endif
