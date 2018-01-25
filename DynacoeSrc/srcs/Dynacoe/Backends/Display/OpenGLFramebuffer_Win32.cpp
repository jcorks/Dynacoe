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

#ifdef DC_BACKENDS_OPENGLFRAMEBUFFER_WIN32

#include <Dynacoe/Backends/Display/Display.h>
#include <Dynacoe/Backends/Display/OpenGLFramebuffer_Multi.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB_Multi.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Modules/Console.h>


#include <iostream>
#include <cassert>


const int display_active_texture    =   5;
const int display_default_w_c       =   640;    
const int display_default_h_c       =   480;

const GLenum display_tex_active     =   GL_TEXTURE3;
const GLenum user_tex_active        =   GL_TEXTURE2;


using namespace std;
using namespace Dynacoe;

LRESULT CALLBACK WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void wrangleGL();
static GLuint   mainDisplayProgram;
static GLuint   mainDisplayTexCoordID;
static GLuint   mainDisplayVertID;
static GLuint   mainDisplayColorID;
static GLuint   mainDisplayBuffer;




static GLfloat transformVertices[16];
static GLfloat colorVertices[16];
static GLfloat texVertices[8];
static DisplayVertex vData[4];


DisplayVertex vDataStatic[] = {
    {0.f, 0.f, 0.f, 1.f,    0.f, 1.f},
    {0.f, 0.f, 0.f, 1.f,    0.f, 0.f},
    {0.f, 0.f, 0.f, 1.f,    1.f, 1.f},
    {0.f, 0.f, 0.f, 1.f,    1.f, 0.f}
};



/* OpenGLFBDisplay methods */





Dynacoe::OpenGLFBDisplay::~OpenGLFBDisplay() {
}

bool Dynacoe::OpenGLFBDisplay::IsCapable(Capability) {
    return true;
}


bool OpenGLFBDisplay::Valid() {return valid;}

void OpenGLFBDisplay::Resize(int w, int h) {

    

    
    RECT clientRect = {0, 0, w, h}; 
    bool b = AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
    b &= SetWindowPos(
        handle, 0,
        0, 0,
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top, 
        SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    

    /*
    RECT rect;
    GetClientRect(handle, &rect);
    winW = rect.right;
    winH = rect.bottom;
    */

    // reform the viewport where
    // the dims requested do not match the actual
    // vieing geometry of the window.
    //glViewport(0, 0, winW, winH);
}


void OpenGLFBDisplay::SetPosition(int x, int y) {

    SetWindowPos(
        handle, handle,
        x, y,
        0, 0,
        SWP_NOSIZE);
}

void OpenGLFBDisplay::SetName(const string & s) {
    SetWindowText(handle, s.c_str());
}


void OpenGLFBDisplay::Hide(bool d) {
    ShowWindow(handle, d?SW_HIDE:SW_SHOW);
}

bool OpenGLFBDisplay::HasInputFocus() {
    return handle == GetForegroundWindow();
}


void Dynacoe::OpenGLFBDisplay::LockClientResize(bool) {}
void Dynacoe::OpenGLFBDisplay::LockClientPosition(bool) {}
void OpenGLFBDisplay::Fullscreen(bool d) {}

int OpenGLFBDisplay::Width() {
    return winW;
}

int OpenGLFBDisplay::Height() {
    return winH;
}

int OpenGLFBDisplay::X() {
    return -1;
}

int OpenGLFBDisplay::Y() {
    return -1;
}

void Dynacoe::OpenGLFBDisplay::SetViewPolicy(ViewPolicy v) {
    policy = v;
}


void Dynacoe::OpenGLFBDisplay::AttachSource(Dynacoe::Framebuffer * f) {
    if (!f) {
        framebuffer = f;
        return;
    }
    if (deviceHandle != wglGetCurrentDC()) {
        wglMakeCurrent(
            deviceHandle, 
            wglGetCurrentContext()
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (f->GetHandleType() == Dynacoe::Framebuffer::Type::GLFBPacket)
        framebuffer = f;
}

void Dynacoe::OpenGLFBDisplay::Update() {
    if (!framebuffer) 
        return;

    framebufferImage = (*(Dynacoe::GLRenderTarget**)framebuffer->GetHandle())->GetTexture();
    MSG msg = {};
    
    UpdateSize();
    lastMsgs.clear();
    while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        lastMsgs.push_back(msg);
    }
  
    int w = Width(),
        h = Height();
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

OpenGLFBDisplay::OpenGLFBDisplay() {

    viewX = 0;
    viewY = 0;
    viewW = 640;
    viewH = 480;

    framebufferPixels = new char[viewW*viewH*4];
    needSizeUpdate = true;


    winW = winH = 0;
    policy = OpenGLFBDisplay::ViewPolicy::MatchSize;
    valid = realize();
    framebuffer = nullptr;
}

bool OpenGLFBDisplay::realize() {


    return 
        spawnWindow("Dynacoe - OpenGL", 640, 480)
    ;

}


// Spawns a Win32 window and returns its handle
bool OpenGLFBDisplay::spawnWindow(const char * name, int _w, int _h) {
    static int val = 0;
    
    wchar_t className[2048];
    swprintf_s(className, L"DynacoeProject%i", val++);


    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowCallback;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = (LPCTSTR)className;

    assert(RegisterClass(&wc));


    RECT clientRect = {0, 0, _w, _h}; 
    bool b = AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);


    handle = CreateWindowEx(WS_EX_APPWINDOW,
        (LPCTSTR)className,
        (LPCTSTR)name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top,
        0,
        0,
        GetModuleHandle(NULL),
        this
    );

    SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR) this);

    

    while(!wglGetCurrentContext());


    if (!handle) {
        cout << "Error occured: " << GetLastError() << endl;
    } else
    //cout << "Spawned window @" << handle << endl;

    if (handle) {
        ShowWindow(handle, SW_SHOW);
        
        wrangleGL();
        setupDisplayProgram();
    }


    // wait for gl context to show before continuing.
    glFlush();
    return handle;
}

// Creates a wgl context within handle
bool OpenGLFBDisplay::createContext(HWND hwnd) {
    //while(!static_deviceContext);

    deviceHandle = GetDC(hwnd);
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    
		PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
		32,                        //Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                        //Number of bits for the depthbuffer
		8,                        //Number of bits for the stencilbuffer
		0,                        //Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	
	int pixelFormat;

	pixelFormat = ChoosePixelFormat(deviceHandle, &pfd); 
	SetPixelFormat(deviceHandle, pixelFormat, &pfd);

    HGLRC curContext = wglGetCurrentContext();
    if (!curContext) {
	    curContext = wglCreateContext(deviceHandle);

        GLenum err = glewInit();
        if (err != GLEW_OK)  {
            cout << "[OpenGLDisplay]: Couldn't wrangle GL names!" << endl;
            exit(9);
        }

        // we set it to the first time to make sure that there is always a valid context
    }

    wglMakeCurrent(
        deviceHandle, 
        curContext
    );

    
    







    return true;
}


// Draws all the information to the screen.
// All draws from this frame were rendered to this texture.
void OpenGLFBDisplay::drawFrame(int w, int h) {
    if (deviceHandle != wglGetCurrentDC()) {
	    wglMakeCurrent(
            deviceHandle, 
            wglGetCurrentContext()
        );
    }

    int dims[4];
    glGetIntegerv(GL_VIEWPORT, dims);

    if (framebuffer) {
        (*(GLRenderTarget**)framebuffer->GetHandle())->Sync();
    }

    

    #if( defined DC_BACKENDS_LEGACYGL_WIN32)
        
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
        glDrawBuffer(GL_BACK);
        assert(glGetError() == GL_NO_ERROR);
        
        
    #endif


    #ifdef DC_BACKENDS_SHADERGL_WIN32 
        
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
        
        // Apparently,. according to spec, either the 0th vertex attrib pointer slot OR 
        // the glVertexPointer for position (with the client state) MUST be enabled
        // for rendering to occur, so we set the 0th slot as a dummy shadow 
        // of the vertex array.
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*8, (void*)0);
        
        glEnableVertexAttribArray(0); // required for some systems
        glEnableVertexAttribArray(11);
        glEnableVertexAttribArray(12);

        glUseProgram(programHandle);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glFinish();

        glDisableVertexAttribArray(0); // required for some systems
        glDisableVertexAttribArray(11);
        glDisableVertexAttribArray(12);



        glUseProgram(0);   

        glActiveTexture(active);     
        assert(glGetError() == GL_NO_ERROR);


        glBindFramebuffer(GL_FRAMEBUFFER, activeFB);

    #endif


    SwapBuffers(deviceHandle);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glViewport(dims[0], dims[1], dims[2], dims[3]);




}


LRESULT CALLBACK OpenGLFBDisplay::WindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    

    
    switch (uMsg)
    {

    case WM_CREATE:
    {
    
        OpenGLFBDisplay * priv = (OpenGLFBDisplay*)((CREATESTRUCT*)lParam)->lpCreateParams;
        priv->createContext(hWnd);

        return 0;    
    }




    case WM_DESTROY:
    {
        OpenGLFBDisplay * priv = (OpenGLFBDisplay*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
        
        if (priv->closeCBs.size()) {for(OpenGLFBDisplay::CloseCallback * r : priv->closeCBs) {
            (*r)();
        }} else {
            
            Engine::Quit();
        }

        return 0;


    }
    
    case WM_SIZING:
    case WM_SIZE:
        ((OpenGLFBDisplay*) GetWindowLongPtr(hWnd, GWLP_USERDATA))->needSizeUpdate = true;
    
    }
    
    

    



    return DefWindowProc(hWnd, uMsg, wParam, lParam);

}


void OpenGLFBDisplay::UpdateSize() {
    if (!needSizeUpdate) return;
    needSizeUpdate = false;
    
    RECT rect;
    GetClientRect(handle, &rect);
    if (rect.bottom != winH || rect.left != winW) {
        winW = rect.right;
        winH = rect.bottom;


        delete[] framebufferPixels;
        framebufferPixels = new char[winH*winW*4];

        for(OpenGLFBDisplay::ResizeCallback * r : resizeCBs) {
            (*r)(winW, winH);
        }

        //cout << winW << "x" << winH << endl;
        // reform the viewport where
        // the dims requested do not match the actual
        // vieing geometry of the window.

    }
    
}


void wrangleGL() {

}
void OpenGLFBDisplay::setupDisplayProgram() {
    #if( defined DC_BACKENDS_SHADERGL_WIN32)
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


Dynacoe::Framebuffer * Dynacoe::OpenGLFBDisplay::GetSource() {
    return framebuffer;
}

std::vector<Dynacoe::Framebuffer::Type> Dynacoe::OpenGLFBDisplay::SupportedFramebuffers() {
    return std::vector<Dynacoe::Framebuffer::Type>({
        Dynacoe::Framebuffer::Type::GLFBPacket
    });
}

Display::DisplayHandleType OpenGLFBDisplay::GetSystemHandleType() {
    return DisplayHandleType::WINAPIHandle;
}

void * OpenGLFBDisplay::GetSystemHandle() {
    return handle;
}

Display::DisplayEventType OpenGLFBDisplay::GetSystemEventType() {
    return DisplayEventType::WINAPIMsg;
}

void * OpenGLFBDisplay::GetLastSystemEvent() {
    return &lastMsgs;
}


std::string OpenGLFBDisplay::Name() {return "OpenGLFBDisplay (For WINAPI)";}
std::string OpenGLFBDisplay::Version() {return "v1.0";}


std::string OpenGLFBDisplay::RunCommand(const std::string &, uint8_t * d) {
    return "";
}






#endif

