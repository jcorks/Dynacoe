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

#ifdef DC_BACKENDS_OPENGLFRAMEBUFFER_X11

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
#include <unistd.h>

const int display_active_texture    =   5;
const int display_default_w_c       =   640;
const int display_default_h_c       =   480;



using namespace std;

static void wrangleGL();
static int xlibErrHandler(Display *, XErrorEvent *);




/* OpenGLFBDisplay methods */
class Command_GLFB_dump : public Dynacoe::Interpreter::Command {
  public:
    Dynacoe::OpenGLFBDisplay * fb;
    Command_GLFB_dump(Dynacoe::OpenGLFBDisplay * in) {
        fb = in;
    }
    
    std::string operator()(const std::vector<std::string> & args) {
        if (args.size() < 2) {
            return "Usage: dump [path to png]";
        }
        
        std::string path = args[1];
        fb->QueueDump(path, 0);
        return "Ok.";
    }
    
    std::string Help() const {
        return "";
    }
    
};


class Command_GLFB_dump_delay : public Dynacoe::Interpreter::Command {
  public:
    Dynacoe::OpenGLFBDisplay * fb;
    Command_GLFB_dump_delay(Dynacoe::OpenGLFBDisplay * in) {
        fb = in;
    }
    
    std::string operator()(const std::vector<std::string> & args) {
        if (args.size() < 3) {
            return "Usage: dump-delay [seconds to wait] [path to png]";
        }
        
        std::string path = args[1];
        fb->QueueDump(path, atoi(args[2].c_str()));
        return "Ok.";
    }
    
    std::string Help() const {
        return "";
    }
    
};




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
    
    if (glXGetCurrentDrawable() != win) {
        glXMakeCurrent(dpy, win, glXGetCurrentContext());
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
          case ConfigureNotify:
            updateDims();
            //cout << endl << "Resize event detected." << endl;
            for(ResizeCallback * r : resizeCBs) {
                (*r)(winW, winH);
            }
            break;


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
        
        
    if (glXGetCurrentDrawable() != win) {
        glXMakeCurrent(dpy, win, glXGetCurrentContext());
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
    GetInterpreter()->AddCommand("dump",       new Command_GLFB_dump(this));
    GetInterpreter()->AddCommand("dump-delay", new Command_GLFB_dump_delay(this));

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
    createContext();
    setupDisplayProgram();




    // wait for gl context to show before continuing.
    glFlush();
    return true;
}

// Creates a glx context within handle
bool Dynacoe::OpenGLFBDisplay::createContext() {

    if (glXGetCurrentContext()) {
        dpy = glXGetCurrentDisplay();
    } else {
        dpy = XOpenDisplay(NULL);
    }

    GLint att[] = {

        /*
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT | GLX_WINDOW_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_CONFIG_CAVEAT, GLX_NONE,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER, True,
        */


        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,

        None };

    XSetErrorHandler(xlibErrHandler);

    if (!dpy) {
        cout << "OpenGLFBDisplay[OpenGL-X11]: Could not connect to X server..." << endl;
        return false;
    }

    root = DefaultRootWindow(dpy);
    vi = glXChooseVisual(dpy, 0, att);

    if(!vi) {
        cout << "OpenGLFBDisplay[OpenGL-X11]: Could not acquire X11 visual..."  << endl;
        return false;
    }


    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | StructureNotifyMask
		| KeyPressMask | KeyReleaseMask
		| PointerMotionMask | ButtonPressMask | ButtonReleaseMask
        | SelectionClear | SelectionRequest | SelectionNotify;

    win = XCreateWindow(
        dpy,
        root,
        0,
        0,
        640, 480,
        0, vi->depth,
        InputOutput,
        vi->visual,
        CWColormap | CWEventMask, &swa
    );

    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Dummy Window");
    PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");  // Set the glxSwapInterval to 0, ie. disable vsync!
    if (!glXSwapIntervalEXT) {
        glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalMESA");
    }

    if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(dpy, win, 0);
    } else {
        // going to be some issues
        std::cout << "No swap interval EXT found. Expect VSYNC\n";
    }

    // TODO: Figure out why context retrieval cant be used to set any existing context without an error?
    /*
    glc = glXGetCurrentContext();
    if (glc) {
        cout << "[OpenGLFBDisplay-OpenGL]: Context already active, skipping context creation." << endl;
    } else {
        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        glXMakeCurrent(dpy, win, glc);
    }
    */

    short init = false;
    if (glXGetCurrentContext()) {
        glc = glXGetCurrentContext();
    } else {
        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        init = true;
    }
    glXMakeCurrent(dpy, win, glc);

    if (init)
        return (glewInit() == GLEW_OK);
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


    #if( defined DC_BACKENDS_LEGACYGL_X11)
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);


        glBindTexture(GL_TEXTURE_2D, framebufferImage);



        glFlush();



        int texW, texH;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &texW);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texH);




        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();




        float normX, normY, normW, normH;
        if (policy == OpenGLFBDisplay::ViewPolicy::MatchSize) {
            glViewport(0, 0, winW, winH);
            normX = -1; normY = -1;
            normW = 1; normH = 1;
        } else {
            glViewport(0, winH - dims[3], dims[2], dims[3]);
            normX = -1; normY = -1;
            normW = 1; normH = 1;

        }



        /*
            int newDims[4];
            glGetIntegerv(GL_VIEWPORT, newDims);
        std::cout << normX << "-" << normY << "-" << normW << "-" << normH <<
                  " FB: " << dims[2] << "x" << dims[3] <<
                  " (actual FB: " << texW << "x" << texH << ")" <<
                  " VP: " << winW << "x" << winH <<
                  " (actual VP: " << newDims[2] << "x" << newDims[3] << ")" <<std::endl;
        */


        glColor4f(1, 1, 1, 1);
        glBegin(GL_TRIANGLES);
            glTexCoord2f(0, 0); glVertex2f(normX,normY);
            glTexCoord2f(1, 0); glVertex2f(normW,normY);
            glTexCoord2f(0, 1); glVertex2f(normX,normH);


            glTexCoord2f(0, 1); glVertex2f(normX,normH);
            glTexCoord2f(1, 1); glVertex2f(normW,normH);
            glTexCoord2f(1, 0); glVertex2f(normW,normY);

        glEnd();







        glBindTexture(GL_TEXTURE_2D, 0);
        assert(glGetError() == GL_NO_ERROR);
        //glGetError();


    #endif


    #ifdef DC_BACKENDS_SHADERGL_X11

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

    #endif


    glXSwapBuffers(dpy, win);
    glViewport(dims[0], dims[1], dims[2], dims[3]);
    if (isBlending) {
        glEnable(GL_BLEND);
    }




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
    #if( defined DC_BACKENDS_SHADERGL_X11)
        static const char * passthroughVert =
            "uniform sampler2D tex;\n"
            "in mat2x4 inputData;\n"
            "out vec2 outUVs;\n"

            "void main() {\n"
            "   gl_Position = inputData[0].xyzw;\n"
            "   outUVs      = inputData[1].xy;\n"
            "}";


        static const char * passthroughFrag =
            "uniform sampler2D tex;\n"
            "in vec2 outUVs;\n"
            "out vec4 finalColor;\n"

            "void main() {\n"
            "   finalColor = texture2D(tex, outUVs);\n"
            "   finalColor.a = 1.f;\n"
            "}";

        static const char * passthroughVert_21 =
            "uniform sampler2D tex;\n"
            "attribute mat2x4 inputData;\n"

            "varying vec2 outUVs;\n"

            "void main() {\n"
            "   gl_Position = inputData[0].xyzw;\n"
            "   outUVs      = inputData[1].xy;\n"
            "}";


        static const char * passthroughFrag_21 =
            "uniform sampler2D tex;\n"
            "varying vec2 outUVs;\n"


            "void main() {\n"
            "   vec4 finalColor;\n"
            "   finalColor = texture2D(tex, outUVs);\n"
            "   finalColor.a = 1.f;\n"
            "   gl_FragColor = finalColor;\n"
            "}";

        static float programDefaultVBOdata[] = {
            -1, -1, 0, 1,   0, 0,   0, 0,
             1, -1, 0, 1,   1, 0,   0, 0,
             1,  1, 0, 1,   1, 1,   0, 0,

             1,  1, 0, 1,   1, 1,   0, 0,
            -1,  1, 0, 1,   0, 1,   0, 0,
            -1, -1, 0, 1,   0, 0,   0, 0
        };

        programHandle = glCreateProgram();

        GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);
        GLuint vertID = glCreateShader(GL_VERTEX_SHADER);


        std::string vSrc;
        std::string fSrc;
        std::string header;
        if (glewIsSupported("GL_VERSION_3_1")) {
            header = "#version 140\n";
            vSrc = header + passthroughVert;
            fSrc = header + passthroughFrag;
        } else if (glewIsSupported("GL_VERSION_3_0 GL_ARB_uniform_buffer_object")) {
            header = "#version 130\n"
                     "#extension GL_ARB_uniform_buffer_object: require\n";
            vSrc = header + passthroughVert;
            fSrc = header + passthroughFrag;
        } else {
            header = "#version 120\n";
            vSrc = header + passthroughVert_21;
            fSrc = header + passthroughFrag_21;
        }


        const char * vPtr = vSrc.c_str();
        const char * fPtr = fSrc.c_str();

        glShaderSource(fragID, 1, &fPtr, NULL);
        glShaderSource(vertID, 1, &vPtr, NULL);



        glAttachShader(programHandle, fragID);
        glAttachShader(programHandle, vertID);

        glCompileShader(fragID);
        glCompileShader(vertID);


        int success;
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
        glBindAttribLocation(programHandle, 11, "inputData");
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


        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8*6, programDefaultVBOdata, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(11, 4, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)0);
        glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)(4*sizeof(float)));



        // set tex param
        int uniformLoc = glGetUniformLocation(programHandle, "tex");
        glUseProgram(programHandle);
        glUniform1i(uniformLoc, display_active_texture);
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        assert(glGetError() == GL_NO_ERROR);

    #endif

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

void Dynacoe::OpenGLFBDisplay::QueueDump(const std::string & str, int wait) {
    dumpQueue.push(str);
    dumpWait.push(wait);
}



bool Dynacoe::OpenGLFBDisplay::Dump(const std::string & path) {
    GLint curTex;
    glActiveTexture(GL_TEXTURE0 + display_active_texture);

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &curTex);
    glBindTexture(GL_TEXTURE_2D, framebufferImage);
    //Directly retrieve texture bytes (RGBA);
    int width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    uint32_t numBytes = width*height*4;
    uint8_t * data = new uint8_t[numBytes];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    for(uint32_t i = 0; i < width*height; ++i) {
        *(data+i*4+3) = 255;
    }

    // dump it to an image
    AssetID id = Dynacoe::Assets::New(Dynacoe::Assets::Type::Image, "dump-texture");
    if (id == AssetID()) {
        delete[] data;
        return false;
    }



    {
        Image & img = Dynacoe::Assets::Get<Image>(id);
        img.frames.push_back(Image::Frame(width, height, std::vector<uint8_t>(data, data+numBytes)));
        if (!Dynacoe::Assets::Write(id, "png", path)) {
            Dynacoe::Assets::Remove(id);
            delete[] data;
            return false;
        }
        Dynacoe::Assets::Remove(id);
    }
    delete[] data;
    return true;
}

#endif
