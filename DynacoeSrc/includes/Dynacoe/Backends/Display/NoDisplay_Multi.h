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


#ifndef H_DC_BACKENDS_NODISPLAY_INCLUDED
#define H_DC_BACKENDS_NODISPLAY_INCLUDED

#include <Dynacoe/Backends/Display/Display.h>

namespace Dynacoe {
class NoDisplay : public Display {
  public:
    NoDisplay();
    ~NoDisplay();

    std::string Name();
    std::string Version();
    bool Valid();


    void Resize(int, int);
    void SetPosition(int, int);
    void Fullscreen(bool);
    void Hide(bool);
    bool HasInputFocus();
    void LockClientResize(bool);
    void LockClientPosition(bool);
    void SetViewPolicy(ViewPolicy);
    
    int Width();
    int Height();
    int X();
    int Y();

    void SetName(const std::string &);
    void AddResizeCallback(ResizeCallback *);
    void RemoveResizeCallback(ResizeCallback *);
    void AddCloseCallback(CloseCallback *);
    void RemoveCloseCallback(CloseCallback *);

    bool IsCapable(Dynacoe::Display::Capability);
    void Update(Framebuffer *){}
    std::vector<Dynacoe::Framebuffer::Type> SupportedFramebuffers(){
        return {
            Dynacoe::Framebuffer::Type::RGBA_PixelArray,
            Dynacoe::Framebuffer::Type::GLFBPacket
        };
    }

    DisplayHandleType GetSystemHandleType();
    void * GetSystemHandle();
    DisplayEventType GetSystemEventType();
    void * GetLastSystemEvent();

};
}


#endif
