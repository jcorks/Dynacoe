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

#ifdef DC_BACKENDS_PIXMAP_X11

#ifndef H_DC_PIXMAP_X11_BACKENDS
#define H_DC_PIXMAP_X11_BACKENDS


#include <X11/X.h>
#include <X11/Xlib.h>
typedef Display X11Display; 

#include <Dynacoe/Shader.h>
#include <vector>
#include <Dynacoe/Backends/Display/Display.h>





namespace Dynacoe {
struct DisplayImpl {
    

    DisplayImpl();


    std::vector<Display::ResizeCallback *> resizeCBs;
    std::vector<Display::CloseCallback *> closeCBs;
    
    bool valid;
    uint32_t flags;
    int viewX;
    int viewY;
    int viewW;
    int viewH;
    
    bool realize();    
    bool spawnWindow(const char *, int, int);
    bool createContext();
    bool queryCapabilities();
    void drawFrame(int, int);

    GLfloat transformResult[16];
    unsigned int  winH, winW;
        



    X11Display              *dpy;
    Colormap                cmap;
    XSetWindowAttributes    swa;
    Window                  win;
    Window                  root;
    XWindowAttributes       gwa;
    XEvent                  xev;
    uint8_t                *frame;    


    Pixmap pixmap;
    XImage * framebuffer;
    GC gcontext;

    int winX, winY;
    
    void updateDims();



};
}


#endif
#endif
