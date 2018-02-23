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

#ifndef H_DC_BACKEND_BASE_INCLUDED
#define H_DC_BACKEND_BASE_INCLUDED

#include <string>
#include <vector>
#include <Dynacoe/Interpreter.h>

namespace Dynacoe {

/// \brief Backend 
///
/// Backends are what links your multimedia project to the host system 
/// through standardized interfaces, abstracting out system-specific features.
/// They are not documented through the HTML docs, you will need to explore the headers yourself
/// if you wish to write your own. 
///
/// There exist Backends for major groups of functionality:
/// Rendering, Input, Audio, Display (windowing), Framebuffer storage/access and perhaps more in the future.
///
/// Directory costruction should always match:
/// [backend name]/[bankend name].h
/// [backend name]/[system-variant].h
/// [backend name]/[system-variant]/*
class Backend {
  public:
    
    Backend();
    
    virtual ~Backend();

    // Identifies the backend
    virtual std::string Name() = 0;

    // Optional string showing the version of the backend.
    virtual std::string Version() = 0;

    // Returns whether or not the backend is in a valid state.
    //
    virtual bool Valid() = 0;


    // Runs an implementation-dependent command of some kind.
    // THe most helpful implementations will have output regardless of 
    // whether the command is recofgnized.
    std::string RunCommand(const std::string & command);

    static Backend * CreateDefaultRenderer();
    static Backend * CreateDefaultAudioManager();
    static Backend * CreateDefaultInputManager();
    static Backend * CreateDefaultDisplay();
    static Backend * CreateDefaultFramebuffer();
  private:
    Interpreter * commander;
  protected:
    Interpreter * GetInterpreter() {return commander;};
};
}

#endif
