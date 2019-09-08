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
#include <cstring>

const int display_active_texture    =   5;
const int display_default_w_c       =   640;
const int display_default_h_c       =   480;



using namespace std;

static int xlibErrHandler(Display *, XErrorEvent *);

static X11Display * mainDisplay = NULL;


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
        switch(evt.type) {
          case ConfigureNotify: {
            updateDims();
            //cout << endl << "Resize event detected." << endl;
            for(ResizeCallback * r : resizeCBs) {
                (*r)(winW, winH);
            }
            break;
          }
          // handles giving X11 the owned clipboard contents
          case SelectionRequest: {

            // notify the server that we changed the selection
            XSelectionRequestEvent event = evt.xselectionrequest;

            // interned string for UTF8 string
            Atom text_x11 = XInternAtom(dpy, "UTF8_STRING", True);

            // only handle normal string requests
            //if (event.target != text_x11) break;

            // Alters the input property with the contents of the clipboard
            // X11 essentially gives you a "bowl" from a foreign client 
            // to "fill" with clipboard content. ("bowl" is the custom-named 
            // property from the foreign client)
            XChangeProperty(
                dpy,                // display
                event.requestor,    // foreign client window
                event.property,     // foreign client "bowl"
                text_x11,           // type 
                8,                  // data format. We only export raw byte data.
                PropModeReplace,    // replacing contents of the property

                (const unsigned char *)&clipboardLocalSend[0],
                clipboardLocalSend.size()
            );


            // Next we send out a selectionnotify event to 
            // let the server know we changed the contents of selection.
            XSelectionEvent send;
            send.type = SelectionNotify;
            send.selection = event.selection;
            send.property = event.property;
            send.target = event.target;
            send.requestor = event.requestor;
            send.time = CurrentTime;


            XSendEvent(
                dpy,
                event.requestor,
                True,
                NoEventMask,
                (XEvent *)&send
            );
            break;
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
        if (mainDisplay) {
            dpy = mainDisplay;
        } else {
            dpy = XOpenDisplay(NULL);
            mainDisplay = dpy;
        }
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

    eglSwapInterval(glDisplay, 0);


    if (eglGetCurrentContext()) {
        glc = eglGetCurrentContext();
    } else {
        EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        glc = eglCreateContext(glDisplay, glConfig, EGL_NO_CONTEXT, contextAttribs);
        assert(glc);
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
    glVertexAttribPointer(VBO_positionLoc, 4, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
    glEnableVertexAttribArray(VBO_positionLoc);

    glUseProgram(programHandle);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    glDisableVertexAttribArray(VBO_positionLoc);
    


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
        glLinkProgram(programHandle);

        VBO_positionLoc = glGetAttribLocation(programHandle, "inputDataPos");


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


std::vector<uint8_t> Dynacoe::OpenGLFBDisplay::GetCurrentClipboard() {
    // There seems to be very limited info on X11 clipboard interaction, seemingly largely because the standard
    // for doing so has changed over time. 
    // As i understand it, to get the clipbaord, the following needs to occur:
    //
    //  - Transfer clipboard contents from the server to the client
    //  - Store the contents as a local property on the client side window
    //  - Read the property.
    //
    // When retrieving data from the server, it actually interacts with the other client 
    // that owns the clipboard content. See "SetCurrentClipboard" for more info.

    // we need a custom property atom to serve as a target location for 
    // storing the clipboard contents
    Atom property      = XInternAtom(dpy, "DYNACOE_COPY_BUFFER", False);
    Atom clipboard_x11 = XInternAtom(dpy, "CLIPBOARD", True);
    Atom text_x11      = XInternAtom(dpy, "UTF8_STRING", True);
    Atom text_x11_fallback = XA_STRING;

    // transfer to client
    XConvertSelection(
        dpy,           // display 
        clipboard_x11, // The selection atom, which shoul dbe clipboard.
        text_x11,      // the target type, text
        property,      // property? seems to be the "target" name for the property.
        win,
        CurrentTime
    );   

    // Apply commands. It blocks until all are processed, so at this point, 
    // the property should be either written to or not.
    XSync(dpy, False);  


    // since we dont want to return until we have valid clipboard contents, we are going to 
    // block and manually stalk the X11 event queue 
    XEvent evt;
    std::vector<XEvent> putback;
    while(true) {
        XNextEvent(dpy, &evt);

        if (evt.type == SelectionNotify) {
            break;  
        } else {
            // anhy irrelevant events, we want to put back in the event queue 
            // after we're done waiting for the clipboard request.
            putback.push_back(evt);
        }
    }
    

    // puts all other events back in the queue that we 
    // consumed while waiting.
    for(uint32_t i = 0; i < putback.size(); ++i) {
        XPutBackEvent(
            dpy,
            &putback[i]  
        );
    }


    Atom real_property_type;
    unsigned long itemCount = 0; // in the mode we're using, itemCount should "always be" the byte count.
    unsigned long remainingBytes = 0;
    int actualFormat;
    unsigned char * data = 0;

    XGetWindowProperty(
        dpy,           // display
        win,           // window
        property,      // named property
        0, 1024*1024,  // Bytes (offset -> max length)
        False,         // Remove property from window? (no)

        AnyPropertyType,
        &real_property_type,
        &actualFormat,


        &itemCount,
        &remainingBytes,
        &data
    );


    // If data wasn't retrievable from the selection, return empty
    if (!(itemCount && data)) {
        return {};
    }


    // convert data to pure bytes
    std::vector<uint8_t> output;
    output.resize(itemCount);
    memcpy(&output[0], data, itemCount);


    // cleanup and remove the property (is applied next X interaction)
    XDeleteProperty(
        dpy,
        win,
        property
    );
    return output;
};


void Dynacoe::OpenGLFBDisplay::SetCurrentClipboard(const std::vector<uint8_t> & data) {
    // process:
    // - Request X11 selection ownership!
    // - Have the clipboard data ready to send if X11 requests itemCount
    
    // The important note is that the clipboard data is retrieved directly from 
    // the program who "sets the clipboard contents". The use of quotes is to 
    // highlight that the clipboard content is never copied to an intermediate 
    // storage buffer for a long period of time.
    // Instead, clipboard content requests are issued to the clipbaord owner as time goes on.
    //
    // As a result, if the last owner of the clipboard is terminated, the clipboard 
    // contents are lost.   

    Atom clipboard_x11 = XInternAtom(dpy, "CLIPBOARD", False);
    Atom text_x11      = XInternAtom(dpy, "UTF8_STRING", False);

    // Make this program the owner. of the clipboadrd.
    // after this point, our X11 event queue should receive SelectionNotify events.
    // If we are already the owner, no action should occur
    XSetSelectionOwner(
        dpy, 
        clipboard_x11,
        win,
        CurrentTime
    );

    // Make sure the request went through.
    XSync(dpy, False);

    // When clipboard requests are made, this is the data that will be sent.
    // See SelectionRequest handling in the X11 event queue
    clipboardLocalSend = data;



}



#endif
