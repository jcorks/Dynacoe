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

#if(defined DC_BACKENDS_OPENGLFRAMEBUFFER_X11 || defined DC_BACKENDS_OPENGLFRAMEBUFFER_WIN32)
#ifndef H_DC_DISPLAY_GLFRAME
#define H_DC_DISPLAY_GLFRAME

/*
    Display_GL3

    A display signified by a Window with an OpenGL context

 */


#ifdef DC_SUBSYSTEM_WIN32
    #ifndef GLEW_STATIC
    #define GLEW_STATIC
    #endif
    #include <glew.h>
    #include <windows.h>
    #include <gl/gl.h>

#endif
#ifdef DC_SUBSYSTEM_X11
    #include <X11/X.h>
    #include <X11/Xlib.h>
    #ifndef GLEW_STATIC
    #define GLEW_STATIC
    #endif
    #include <GL/glew.h>
    #include <GL/glx.h>
    typedef Display X11Display; 
#endif

#include <vector>
#include <stack>
#include <Dynacoe/Backends/Display/Display.h>


namespace Dynacoe {


class OpenGLFBDisplay : public Dynacoe::Display {
  public:
    OpenGLFBDisplay();
    ~OpenGLFBDisplay();


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
    void Update();
    void AttachSource(Dynacoe::Framebuffer *);
    std::vector<Dynacoe::Framebuffer::Type> SupportedFramebuffers();
    Dynacoe::Framebuffer * GetSource();
    

    void * GetSystemHandle();
    DisplayHandleType GetSystemHandleType();
    void * GetLastSystemEvent();
    DisplayEventType GetSystemEventType();
    
    bool Dump(const std::string &);
    void QueueDump(const std::string &, int delay);
    
  private:
    

    std::vector<Display::ResizeCallback *> resizeCBs;
    std::vector<Display::CloseCallback *> closeCBs;
    
    bool valid;
    int screenTex; // openGL texture to be handled
    uint32_t flags;
    int viewX;
    int viewY;
    int viewW;
    int viewH;

    char * framebufferPixels;
    GLuint framebufferImage;
    Dynacoe::Framebuffer * framebuffer;
    GLint  framebufferW, framebufferH;
    
    bool realize();    
    bool spawnWindow(const char *, int, int);
    
    bool queryCapabilities();
    void setupDisplayProgram();
    void drawFrame(int, int);
    Display::ViewPolicy policy;

    GLfloat transformResult[16];
    unsigned int  winH, winW;
    
    #ifdef DC_SUBSYSTEM_WIN32
        bool    createContext(HWND h);
        HWND    handle;
        HDC     deviceHandle; 
        HGLRC   GLcontext;
        std::vector<MSG> lastMsgs;

        bool needSizeUpdate;
        static LRESULT CALLBACK WindowCallback(HWND, UINT, WPARAM, LPARAM);
        void UpdateSize();
    #endif

    #ifdef DC_SUBSYSTEM_X11
        bool createContext();
        X11Display              *dpy;
        XVisualInfo             *vi;
        Colormap                cmap;
        XSetWindowAttributes    swa;
        Window                  win;
        Window                  root;
        GLXContext              glc;
        XWindowAttributes       gwa;
        XEvent                  xev;
        std::vector<XEvent>      lastEvents;

        int winX, winY;
        
        void updateDims();
    #endif

    GLint programHandle;
    GLuint vao;
    GLuint vbo;

    enum ProgramBindPoint {
        VertexPosition = 0,
        VertexColor = 1,
        VertexUVs = 2,
    };
    std::stack<std::string> dumpQueue;
    std::stack<int> dumpWait;
    time_t cycleTime;

};

struct DisplayVertex {
    GLfloat
    x, y, z, w,
    texX, texY;
};
}


#endif
#endif

