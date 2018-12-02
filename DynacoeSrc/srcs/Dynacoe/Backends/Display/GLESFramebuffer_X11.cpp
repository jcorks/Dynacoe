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

#ifdef DC_BACKENDS_GLESFRAMEBUFFER_X11

#include <Dynacoe/Backends/Display/Display.h>
#include <Dynacoe/Backends/Display/OpenGLFramebuffer_Multi.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget.h>
#include <Dynacoe/Modules/Console.h>
#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Image.h>
#include <Dynacoe/Interpreter.h>

#include <X11/Xatom.h>


#include <iostream>
#include <cassert>
#include <stack>


const int display_active_texture    =   5;
const int display_default_w_c       =   640;
const int display_default_h_c       =   480;



using namespace std;

static void wrangleGL();
static int xlibErrHandler(Display *, XErrorEvent *);




Dynacoe::OpenGLFBDisplay::~OpenGLFBDisplay() {
}

bool Dynacoe::OpenGLFBDisplay::IsCapable(Capability) {
    return true;
}

bool Dynacoe::OpenGLFBDisplay::Valid() {return valid;}

void Dynacoe::OpenGLFBDisplay::Resize(int w, int h) {
    XResizeWindow(dpy, win, w, h);
    XFlush(dpy);
    winW = w;
    winH = h;
}

void Dynacoe::OpenGLFBDisplay::Hide(bool b) {
    XUnmapWindow(dpy, win);
    XFlush(dpy);

}

void Dynacoe::OpenGLFBDisplay::LockClientResize(bool) {}
void Dynacoe::OpenGLFBDisplay::LockClientPosition(bool) {}


void Dynacoe::OpenGLFBDisplay::Fullscreen(bool d) {
    if (d) {
        Atom atoms[2] = { XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False), None };
        int status = XChangeProperty(
          dpy,
          win,
          XInternAtom(dpy, "_NET_WM_STATE", False),
          XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, 1
        );
        XFlush(dpy);
        //std::cout << status << std::endl;
    }
}


void Dynacoe::OpenGLFBDisplay::SetPosition(int x, int y) {
    XMoveWindow(dpy, win, x, y);
}

int Dynacoe::OpenGLFBDisplay::Width() {
    return winW;
}

int Dynacoe::OpenGLFBDisplay::Height() {
    return winH;
}

int Dynacoe::OpenGLFBDisplay::X() {
    return winX;
}

int Dynacoe::OpenGLFBDisplay::Y() {
    return winY;
}

void Dynacoe::OpenGLFBDisplay::SetName(const string & name) {
    XStoreName(dpy, win, name.c_str());
    XFlush(dpy);
}

bool Dynacoe::OpenGLFBDisplay::HasInputFocus() {
    Window ret;
    int unused;

    XGetInputFocus(dpy, &ret, &unused);
    return ret == win;
}



void Dynacoe::OpenGLFBDisplay::SetViewPolicy(ViewPolicy v) {
    policy = v;
}



std::vector<Dynacoe::Framebuffer::Type> Dynacoe::OpenGLFBDisplay::SupportedFramebuffers() {
    return std::vector<Dynacoe::Framebuffer::Type>({
        Dynacoe::Framebuffer::Type::GLFBPacket
    });
}

void Dynacoe::OpenGLFBDisplay::Update(Dynacoe::Framebuffer * framebuffer) {
    if (!framebuffer) return;
    if (framebuffer->GetHandleType() != Dynacoe::Framebuffer::Type::GLFBPacket) return;
    
    if (eglGetCurrentDisplay() != glDisplay) {
        eglMakeCurrent(glDisplay, glSurface, glSurface, glc);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    framebufferImage = (*(GLRenderTarget**)framebuffer->GetHandle())->GetTexture();
    if (!dumpQueue.empty()) {
        if (dumpWait.top() <= 0) {
        
            
            if(Dump(dumpQueue.top())) {
                Console::Info() << "Dumped frame to " << dumpQueue.top() << "\n";
            } else {
                Console::Info() << "Failed to dump frame to " << dumpQueue.top() << "\n";
            }
            dumpQueue.pop();
            dumpWait.pop();
        } else {
            if (cycleTime != time(NULL)) {
                int wait = dumpWait.top();
                dumpWait.pop();
                dumpWait.push(wait-1);
                cycleTime = time(NULL);
            } 
        }
    }
    // consume only configure events to get resizes.
    // Merely egnore all other event types and put them
    // back into the event queue.

    
    int numEvs = XEventsQueued(dpy, QueuedAlready);
    lastEvents.clear();
    XEvent evt;

    
    for(int i = 0; i < numEvs; ++i) {
        XNextEvent(dpy, &evt);
        if (evt.type == ConfigureNotify) {
            updateDims();
            //cout << endl << "Resize event detected." << endl;
            for(ResizeCallback * r : resizeCBs) {
                (*r)(winW, winH);
            }
        }
        lastEvents.push_back(evt);
    }






    // perhaps query input here if input lib is dependent on the win manager.
    int w = Width(),
        h = Height();
        
    if (eglGetCurrentDisplay() != glDisplay) {
        eglMakeCurrent(glDisplay, glSurface, glSurface, glc);
    }


    if (framebuffer) {
        (*(GLRenderTarget**)framebuffer->GetHandle())->Sync();
    }
    
    
    drawFrame(w, h);
    // render tex on this context's framebuffer.
}


void Dynacoe::OpenGLFBDisplay::AddResizeCallback(ResizeCallback * cb) {
    if (cb)
        resizeCBs.push_back(cb);
}

void Dynacoe::OpenGLFBDisplay::RemoveResizeCallback(ResizeCallback * cb) {
    for(int i = 0; i < resizeCBs.size(); ++i) {
        if (resizeCBs[i] == cb) {
            resizeCBs.erase(resizeCBs.begin() + i);
        }
    }
}


void Dynacoe::OpenGLFBDisplay::AddCloseCallback(CloseCallback * cb) {
    if (cb)
        closeCBs.push_back(cb);
}

void Dynacoe::OpenGLFBDisplay::RemoveCloseCallback(CloseCallback * cb) {
    for(int i = 0; i < closeCBs.size(); ++i) {
        if (closeCBs[i] == cb) {
            closeCBs.erase(closeCBs.begin() + i);
        }
    }
}


/* Implementation methods */

Dynacoe::OpenGLFBDisplay::OpenGLFBDisplay() {

    viewX = 0;
    viewY = 0;
    viewW = 640;
    viewH = 480;


    winX = winY = winH = winW = 0;
    policy = OpenGLFBDisplay::ViewPolicy::MatchSize;
    valid = realize();

}

bool Dynacoe::OpenGLFBDisplay::realize() {


    return
        spawnWindow("Dynacoe - OpenGL", 640, 480)
    ;

}




// Spawns a Win32 window and returns its handle
bool Dynacoe::OpenGLFBDisplay::spawnWindow(const char * name, int _w, int _h) {
    // setup window
    {

        dpy = XOpenDisplay(NULL);
        XSetErrorHandler(xlibErrHandler);

        root = DefaultRootWindow(dpy);
        int resultConfig;




        swa.event_mask = ExposureMask | StructureNotifyMask
		    | KeyPressMask | KeyReleaseMask
		    | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

        win = XCreateWindow(
            dpy,
            root,
            0,
            0,
            640, 480,
            0, CopyFromParent,
            InputOutput,
            CopyFromParent,
            CWEventMask, &swa
        );

        XMapWindow(dpy, win);
        XStoreName(dpy, win, "Dummy Window");

    }
   
    createContext();
    setupDisplayProgram();




    // wait for gl context to show before continuing.
    glFlush();
    return true;
}

// Creates a egl context within handle
bool Dynacoe::OpenGLFBDisplay::createContext() {




    /*if (eglGetCurrentContext()) {
        glDisplay = eglGetCurrentDisplay();
    } else {
        glDisplay = eglGetDisplay((EGLNativeDisplayType)dpy);

    }*/
    glDisplay = eglGetDisplay((EGLNativeDisplayType)dpy);


    assert(eglInitialize(glDisplay, NULL, NULL) == GL_TRUE);
    EGLint att[] = {

        EGL_RED_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,

        EGL_NONE
    };

    int resultConfig;

    if(!eglChooseConfig(glDisplay, att, &glConfig, 1, &resultConfig)) {
        cout << "OpenGLFBDisplay[OpenEGL-X11]: Could not acquire X11 visual..."  << endl;
        return false;
    }

    glSurface = eglCreateWindowSurface(glDisplay, glConfig, win, NULL);
    assert(glSurface != EGL_NO_SURFACE);

    if (eglSwapInterval) {
        eglSwapInterval(glDisplay, 0);
    } else {
        // going to be some issues
        std::cout << "No swap interval EXT found. Expect VSYNC\n";
    }


    short init = false;
    if (eglGetCurrentContext()) {
        glc = eglGetCurrentContext();
    } else {
        EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        glc = eglCreateContext(glDisplay, glConfig, EGL_NO_CONTEXT, contextAttribs);
        assert(glc);
        init = true;
    }
    eglMakeCurrent(glDisplay, glSurface, glSurface, glc);

    return true;


}


// Draws all the information to the screen.
// All draws from this frame were rendered to this texture.
void Dynacoe::OpenGLFBDisplay::drawFrame(int w, int h) {




    int dims[4];
    int isBlending;
    glGetIntegerv(GL_VIEWPORT, dims);
    glGetIntegerv(GL_BLEND, &isBlending);
    
    if (isBlending)
        glDisable(GL_BLEND);




    int active;
    GLint activeFB;

    if (policy == OpenGLFBDisplay::ViewPolicy::MatchSize) {
        glViewport(0, 0, winW, winH);
    } else {
        glViewport(0, winH - dims[3], dims[2], dims[3]);
    }

    glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &activeFB);
    glActiveTexture(GL_TEXTURE0 + display_active_texture);
    glBindTexture(GL_TEXTURE_2D, framebufferImage);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    glBindBuffer(GL_ARRAY_BUFFER, vbo);


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(11);
    glEnableVertexAttribArray(12);

    glUseProgram(programHandle);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(11);
    glDisableVertexAttribArray(12);



    glUseProgram(0);

    glActiveTexture(active);
    assert(glGetError() == GL_NO_ERROR);



    glBindFramebuffer(GL_FRAMEBUFFER, activeFB);


    eglSwapBuffers(glDisplay, glSurface);
    glViewport(dims[0], dims[1], dims[2], dims[3]);
    if (isBlending) {
        glEnable(GL_BLEND);
    }




}

void Dynacoe::OpenGLFBDisplay::updateDims() {
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
}


void Dynacoe::OpenGLFBDisplay::setupDisplayProgram() {

        static const char * passthroughVert_ES =
            "uniform sampler2D tex;\n"
            "attribute highp vec4 inputDataPos;\n"

            "varying mediump vec2 outUVs;\n"

            "void main() {\n"
            "   gl_Position = inputDataPos.xyzw;\n"
            "   outUVs.x    = (inputDataPos.x+1.0)/2.0;\n"
            "   outUVs.y    = (inputDataPos.y+1.0)/2.0;\n"
            "}";


        static const char * passthroughFrag_ES =
            "uniform sampler2D tex;\n"
            "varying mediump vec2 outUVs;\n"


            "void main() {\n"
            "   lowp vec4 finalColor;\n"
            "   finalColor = texture2D(tex, outUVs);\n"
            "   finalColor.a = 1.0;\n"
            "   gl_FragColor = finalColor;\n"
            "}";

        static float programDefaultVBOdata[] = {
            -1, -1, 0, 1,
             1, -1, 0, 1,
             1,  1, 0, 1,

             1,  1, 0, 1,
            -1,  1, 0, 1,
            -1, -1, 0, 1,
        };

        programHandle = glCreateProgram();

        GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);
        GLuint vertID = glCreateShader(GL_VERTEX_SHADER);


        std::string vSrc;
        std::string fSrc;
        std::string header;

        header = "";
        vSrc = header + passthroughVert_ES;
        fSrc = header + passthroughFrag_ES;


        const char * vPtr = vSrc.c_str();
        const char * fPtr = fSrc.c_str();

        glShaderSource(fragID, 1, &fPtr, NULL);
        glShaderSource(vertID, 1, &vPtr, NULL);



        glAttachShader(programHandle, fragID);
        glAttachShader(programHandle, vertID);

        glCompileShader(fragID);
        glCompileShader(vertID);

        assert(glGetError() == GL_NO_ERROR);

        int success =0;
        glGetShaderiv(fragID, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            cout << "[Dynacoe::OpenGLFBDisplay]: Fragment shader failed to compile:" << endl;
            int logLength;
            glGetShaderiv(fragID, GL_INFO_LOG_LENGTH, &logLength);
            char log[logLength];

            glGetShaderInfoLog(fragID, logLength, &logLength, log);
            cout << log << endl;
            assert(0);
        }


        glGetShaderiv(vertID, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            cout << "[Dynacoe::OpenGLFBDisplay]: Vertex shader failed to compile:" << endl;
            int logLength;
            glGetShaderiv(vertID, GL_INFO_LOG_LENGTH, &logLength);
            char log[logLength];
            glGetShaderInfoLog(vertID, logLength, &logLength, log);
            cout << log << endl;
            assert(0);
        }

        // Binding is always applied fo rth next link
        glBindAttribLocation(programHandle, 11, "inputDataPos");
        glLinkProgram(programHandle);

        glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            cout << "[Dynaoce::OpenGL]: Linking for program failed" << endl;
            assert(0);
        }




        assert(glGetError() == GL_NO_ERROR);


        // now set up vbo
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);


        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, programDefaultVBOdata, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);



        // set tex param
        int uniformLoc = glGetUniformLocation(programHandle, "tex");
        glUseProgram(programHandle);
        glUniform1i(uniformLoc, display_active_texture);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        assert(glGetError() == GL_NO_ERROR);


}


Dynacoe::Display::DisplayHandleType Dynacoe::OpenGLFBDisplay::GetSystemHandleType() {
    return DisplayHandleType::X11Display;
}

void * Dynacoe::OpenGLFBDisplay::GetSystemHandle() {
    return dpy;
}


Dynacoe::Display::DisplayEventType Dynacoe::OpenGLFBDisplay::GetSystemEventType() {
    return DisplayEventType::X11Event;
}

void * Dynacoe::OpenGLFBDisplay::GetLastSystemEvent() {
    return &lastEvents;
}


int xlibErrHandler(Display * d, XErrorEvent * e) {
    char bufferErr[4096];
    XGetErrorText(d, e->error_code, bufferErr, 4096);


    cout << endl << "Dynacoe::X11 Error: " << bufferErr << endl;
    //assert(0);
    return 0;
}

std::string Dynacoe::OpenGLFBDisplay::Name() {return "OpenGLFBDisplay (For X11)";}
std::string Dynacoe::OpenGLFBDisplay::Version() {return "v1.0";}

bool Dynacoe::OpenGLFBDisplay::Dump(const std::string & path) {
    assert(!"Not implemented yet");
    return true;
}


void Dynacoe::OpenGLFBDisplay::QueueDump(const std::string & str, int wait) {
    dumpQueue.push(str);
    dumpWait.push(wait);
}


#endif
