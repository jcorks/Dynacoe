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
#include <Dynacoe/Backends/Display/Pixmap_X11.h>
#include <X11/Xatom.h>

#include <iostream>
#include <cassert>
#include <stack>

#include <Dynacoe/Components/Random.h>


const int display_default_w_c       =   640;    
const int display_default_h_c       =   480;



using namespace std;

static int xlibErrHandler(Display *, XErrorEvent *);




/* Display methods */





Dynacoe::Display::Display() {
    priv = new DisplayImpl;
}

Dynacoe::Display::~Display() {
    delete priv;
}


bool Dynacoe::Display::isValid() {return priv->valid;}

void Dynacoe::Display::resize(int w, int h) {
    XResizeWindow(priv->dpy, priv->win, w, h);
    XFlush(priv->dpy);
    priv->winW = w;
    priv->winH = h;
}

void Dynacoe::Display::hide(bool b) {
    XUnmapWindow(priv->dpy, priv->win);
    XFlush(priv->dpy);

}


void Dynacoe::Display::fullscreen(bool d) {
    if (d) {
        Atom atoms[2] = { XInternAtom(priv->dpy, "_NET_WM_STATE_FULLSCREEN", False), None };
        int status = XChangeProperty(
          priv->dpy, 
          priv->win, 
          XInternAtom(priv->dpy, "_NET_WM_STATE", False),
          XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, 1
        );
        XFlush(priv->dpy);
        std::cout << status << std::endl;
    }
}


void Dynacoe::Display::setPos(int x, int y) {
    XMoveWindow(priv->dpy, priv->win, x, y);
}

int Dynacoe::Display::getW() {
    return priv->winW;
}

int Dynacoe::Display::getH() {
    return priv->winH;
}

int Dynacoe::Display::getX() {
    return priv->winX;
}

int Dynacoe::Display::getY() {
    return priv->winY;
}

void Dynacoe::Display::setName(const string & name) {
    XStoreName(priv->dpy, priv->win, name.c_str());
    XFlush(priv->dpy);
}


int Dynacoe::Display::getViewX() {return priv->viewX;}
int Dynacoe::Display::getViewY() {return priv->viewY;}
int Dynacoe::Display::getViewW() {return priv->viewW;}
int Dynacoe::Display::getViewH() {return priv->viewH;}

void Dynacoe::Display::setView(int x, int y, int w, int h) {
    priv->viewX = x;
    priv->viewY = y;    
    priv->viewW = w;
    priv->viewH = h;
}


void Dynacoe::Display::updateDisplay(void * data) {
    priv->frame = (uint8_t*)data;
    
    static Random rng;
    static float patter = 0;
    patter+=.1;
    
    int num = DefaultDepth(priv->dpy, DefaultScreen(priv->dpy));

    // update data in pixmap
    /*
    for(int y = 0; y < 30; ++y) { 
        for(int x = 0; x < 30; ++x) {
            //XSetForeground(priv->dpy, priv->gcontext, Color(rng.spread(0, 256), rng.spread(0, 256), rng.spread(0, 256), 255).Int());
            //XDrawPoint(priv->dpy, priv->pixmap, priv->gcontext, x, y); 
            //XPutPixel(&priv->framebuffer, x, y, Color(rng.spread(0, 256), rng.spread(0, 256), rng.spread(0, 256), 255).Int());
            priv->framebuffer->data[(x + priv->framebuffer->width*y)*4] =   sin(patter)*255;
            priv->framebuffer->data[(x + priv->framebuffer->width*y)*4+1] = sin(patter)*255;
            priv->framebuffer->data[(x + priv->framebuffer->width*y)*4+2] = sin(patter)*255;
            priv->framebuffer->data[(x + priv->framebuffer->width*y)*4+3] = sin(patter)*255;
        }
    }
    */
    framebuffer->data = data;

    // consume only configure events to get resizes.
    // Merely egnore all other event types and put them
    // back into the event queue.

    int numEvs = XEventsQueued(priv->dpy, QueuedAlready);
    std::stack<XEvent> evtList;
    XEvent evt;

    for(int i = 0; i < numEvs; ++i) {
        XNextEvent(priv->dpy, &evt);
        if (evt.type == ConfigureNotify) {
            priv->updateDims();
            for(ResizeCallback * r : priv->resizeCBs) {
                (*r)(priv->winW, priv->winH);
            }
            
        } else {
            evtList.push(evt);
        }
    }
    while(!evtList.empty()) {
        XPutBackEvent(priv->dpy, &evtList.top());
        evtList.pop();
    }
    


   

    // perhaps query input here if input lib is dependent on the win manager.  
    int w = getW(),
        h = getH();
    priv->drawFrame(w, h);
    // render tex on this context's framebuffer.
}


void Dynacoe::Display::addResizeCallback(ResizeCallback * cb) {
    if (cb)
        priv->resizeCBs.push_back(cb);
}

void Dynacoe::Display::removeResizeCallback(ResizeCallback * cb) {
    for(int i = 0; i < priv->resizeCBs.size(); ++i) {
        if (priv->resizeCBs[i] == cb) {
            priv->resizeCBs.erase(priv->resizeCBs.begin() + i);
        }
    }
}



/* Implementation methods */

Dynacoe::DisplayImpl::DisplayImpl() {

    viewX = 0;
    viewY = 0;
    viewW = 640;
    viewH = 480;

    winX = winY = winH = winW = 0;
    valid = realize();

    
}

bool Dynacoe::DisplayImpl::realize() {
    flags = 
        Dynacoe::Display::Capability::CAN_RESIZE | 
        Dynacoe::Display::Capability::CAN_MOVE | 
        Dynacoe::Display::Capability::CAN_FULLSCREEN | 
        Dynacoe::Display::Capability::CAN_HIDE;

    return 
        spawnWindow("Dynacoe - OpenGL", 640, 480)
    ;

}


// Spawns a Win32 window and returns its handle
bool Dynacoe::DisplayImpl::spawnWindow(const char * name, int _w, int _h) {

    dpy = XOpenDisplay(NULL);
    XSetErrorHandler(xlibErrHandler);
     
    if (!dpy) {
        cout << "Display[Pixmap-X11]: Could not connect to X server..." << endl;
        return false;
    }
            
    root = DefaultRootWindow(dpy);


    swa.colormap = cmap;
    swa.event_mask = ExposureMask | StructureNotifyMask
		| KeyPressMask | KeyReleaseMask
		| PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
     
    win = XCreateSimpleWindow(
        dpy, 
        root, 
        0, 
        0, 
        _w, _h, 
        0,
        0,
        0
    );

    XMapWindow(dpy, win);
    XStoreName(dpy, win, name);
   

    

    pixmap = XCreatePixmap(dpy, win, _w, _h, DefaultDepth(dpy, DefaultScreen(dpy)));

    unsigned long valuemask = 0;
    XGCValues values;
    unsigned int line_width = 1;
    int line_style = LineSolid;
    int cap_style = CapButt;
    int join_style = JoinBevel;
    int screen_num = DefaultScreen(dpy);

    gcontext = XCreateGC(dpy, win, valuemask, &values);
    if (gcontext < 0) {
        cout << "[Display::Pixmap]: Failed to create X11 graphics context!\n" << endl;
    }



    XSetForeground(dpy, gcontext, BlackPixel(dpy, screen_num));
    XSetBackground(dpy, gcontext, WhitePixel(dpy, screen_num));
    XSetLineAttributes(dpy, gcontext, line_width, line_style, cap_style, join_style);

    XSetFillStyle(dpy, gcontext, FillSolid);


    framebuffer = XCreateImage( 
        dpy,
        XDefaultVisual( dpy, DefaultScreen(dpy)),
        XDefaultDepth ( dpy, DefaultScreen(dpy)),
        ZPixmap,
        0,
        new char[_w*_h*4],
        _w, 
        _h,
        32,
        _w*4
    );

    XFlush(dpy);
    return true;
}



// Draws all the information to the screen.
// All draws from this frame were rendered to this texture.
void Dynacoe::DisplayImpl::drawFrame(int w, int h) {

    
    //XCopyArea(dpy, pixmap, win, gcontext, 0, 0, w, h, 0, 0);
    XPutImage(dpy, win, gcontext, framebuffer, 0, 0, 0, 0, w, h);    


    XFlush(dpy);
}

Dynacoe::Display::VisualDataType Dynacoe::Display::requiredDisplayDataClass() {
    return Dynacoe::Display::VisualDataType::RGBA_PixelArray;
}

void Dynacoe::DisplayImpl::updateDims() {
    unsigned int depth, border;
    Window getRoot; 
    //XGetGeometry(dpy, win, &getRoot, 
    //    &winX, &winY, &winW, &winH,
    //    &border, &depth
    //);

    XGetWindowAttributes(dpy, win, &gwa);
    winH = gwa.height;
    winW = gwa.width;

    XFlush(dpy);


    XFreePixmap(dpy, pixmap);
    pixmap = XCreatePixmap(dpy, win, winW, winH, DefaultDepth(dpy, DefaultScreen(dpy)));


}




int xlibErrHandler(Display * d, XErrorEvent * e) {
    char bufferErr[4096];
    XGetErrorText(d, e->error_code, bufferErr, 4096);
    

    cout << endl << "Dynacoe::X11 Error: " << bufferErr << endl;
    return 0;
}

#endif

