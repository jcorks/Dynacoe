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

#if ( defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32 )

#include <Dynacoe/Backends/Renderer/StaticState.h>


#include <Dynacoe/Backends/Renderer/ShaderGL_Multi.h>
#include <Dynacoe/Backends/Display/Display.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/StaticProgram.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB_Multi.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/GLVersionQuery.h>
#include <Dynacoe/Util/Chain.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>

#ifdef DC_SUBSYSTEM_X11        
    #include <X11/X.h>
    #include <X11/Xlib.h>
    typedef Display X11Display;
#endif



using namespace Dynacoe;
using std::endl;
using std::string;
using std::cout;
using std::vector;

const int   GLREQUEST_INIT_SIZE  = 1024;
const float GLREQUEST_FACTOR_INC = 1.2;
const int   NUM_FLOATS_PER_TEX   = 4;
const int DBUFFER_DEFAULT_INIT_LENGTH         = 32;

static void createContext();

const float IdentityMatrix[] =  {
    0.f, 0.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    1.f, 0.f, 0.f, 0.f
};

GLint DEBUGME_GetFramebufferAttachment() {
    GLint fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
    return fb;    
}

GLint DEBUGME_GetFramebufferColorAttachment() {
    GLint fb;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &fb);
    return fb;    
}
/// SHaders ////

const char * vertShader_BasicShader = 
"out vec2 UV;\n"
"out vec3 normals;\n"


"void main(void) {\n"
"   mat4 testMT;\n"
//"   testMT[0] = vec4(1, 0, 0, 0);\n"
//"   testMT[1] = vec4(0, 1, 0, 0);\n"
//"   testMT[2] = vec4(0, 0, 1, 0);\n"
//"   testMT[3] = vec4(0, 0, -30, 1);\n"
"   gl_Position = Dynacoe_ProjectionTransform * (Dynacoe_ViewTransform  * (Dynacoe_ModelTransform * vec4(Dynacoe_Position, 1.f)));\n"
//"   gl_Position = Dynacoe_ProjectionTransform * (testMT  * (Dynacoe_ModelTransform * vec4(Dynacoe_Position, 1.f)));\n"
"   UV = Dynacoe_UV;\n"
"   normals = Dynacoe_Normal;\n"
"}\n";


const char * vertShader_BasicShader21 = 
"varying vec2 UV;\n"
"varying vec3 normals;\n"



"void main(void) {\n"
"   gl_Position = Dynacoe_ProjectionTransform * (Dynacoe_ViewTransform  * (Dynacoe_ModelTransform * vec4(Dynacoe_Position, 1.f)));\n"
"   UV = Dynacoe_UV;\n"
"   normals = Dynacoe_Normal;\n"
"}\n";



const char * fragShader_BasicShader =
"in vec2 UV;\n"
"in vec3 normals;\n"


"out vec4 color;\n"

"void main(void) {\n"
"   if (Dynacoe_SlotHasTexture(0))\n"
"       color = Dynacoe_SampleColor(0, UV);\n"
"   else\n"
"       color = vec4(Dynacoe_MaterialDiffuse, 1.f);\n"
"   if (Dynacoe_CanSampleFramebuffer())\n"
"       color = .5*color+.5*Dynacoe_SampleFramebuffer(UV);\n"
"}\n";
    
const char * fragShader_BasicShader21 =
"varying vec2 UV;\n"
"varying vec3 normals;\n"


"void main(void) {\n"
"   vec4 color;\n"
"   if (Dynacoe_SlotHasTexture(0))\n"
"       color = Dynacoe_SampleColor(0, UV);\n"
"   else\n"
"       color = vec4(Dynacoe_MaterialDiffuse, 1.f);\n"
"   if (Dynacoe_CanSampleFramebuffer())\n"
"       color = .5*color+.5*Dynacoe_SampleFramebuffer(UV);\n"
"   gl_FragColor = color;\n"
"}\n";









const char * vertShader_LightShader = 


"out vec2 UV;\n"
"out vec3 normalInterp;\n"
"out vec3 pos;\n"


"void main(void) {\n"
/*"   mat4 projection = Dynacoe_ProjectionTransform;\n"
"   mat4 modelview =  Dynacoe_ViewTransform* Dynacoe_ModelTransform;\n"
"   vec3 vertPos;\n"
"   vec3 inputPosition = Dynacoe_Position;\n"
"   vec3 inputNormal = Dynacoe_Normal;\n"
"   mat4 normalMat = Dynacoe_ModelNormalTransform;\n"

"   gl_Position = projection * modelview * vec4(inputPosition, 1.0);\n"
"   vec4 vertPos4 = modelview * vec4(inputPosition, 1.0);\n"
"   vertPos = vec3(vertPos4) / vertPos4.w;\n"
"   normalInterp = vec3(normalMat * vec4(inputNormal, 0.0));\n"

"   pos = vertPos;\n"*/

"   mat4 mv = (Dynacoe_ViewTransform  * Dynacoe_ModelTransform);"
"   vec4 unprojPos = (mv * (vec4(Dynacoe_Position, 1.f)));\n"
"   gl_Position = Dynacoe_ProjectionTransform * unprojPos;\n"
"   UV = Dynacoe_UV;\n"
"   normalInterp = (Dynacoe_ModelNormalTransform * vec4(Dynacoe_Normal, 0.f)).xyz;\n"
"   pos = unprojPos.xyz / unprojPos.w;\n"

"}\n";

const char * vertShader_LightShader21 = 


"varying vec2 UV;\n"
"varying vec3 normalInterp;\n"
"varying vec3 pos;\n"


"void main(void) {\n"
"   mat4 mv = (Dynacoe_ViewTransform  * Dynacoe_ModelTransform);"
"   vec4 unprojPos = (mv * (vec4(Dynacoe_Position, 1.f)));\n"
"   gl_Position = Dynacoe_ProjectionTransform * unprojPos;\n"
"   UV = Dynacoe_UV;\n"
"   normalInterp = (Dynacoe_ModelNormalMatrix * vec4((mv * (Dynacoe_Normal, 1.f)).xyz, 0.0)).xyz;\n"//vec3(Dynacoe_NormalMatrix * Dynacoe_ModelNormalMatrix * vec4(Dynacoe_Normal, 0.0));\n"
"   pos = (mv * (Dynacoe_Position, 1.f)).xyz;\n"
"}\n";


/*
const char * fragShader_LightShader = 
"in vec2 UV;\n"
"in vec3 normalInterp;\n"
"in vec3 pos;\n"


"out vec4 color;\n"

//"const vec3 lightPos = vec3(1.0,1.0,1.0);\n"
"const vec3 ambientColor = vec3(0.1, 0.0, 0.0);\n"
"const vec3 diffuseColor = vec3(0.1, 0.5, 0.0);\n"
"const vec3 specColor = vec3(.5, 1.0, .7);\n"
"const float shininess = 16.0;\n"
"const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space\n"
"const int mode = 1;"
"void main() {\n"
"    vec3 lightPos = (Dynacoe_ViewTransform * vec4(_BSI_Dynacoe_Light_pos(0), 1.f)).xyz;\n"
"    vec3 normal = normalize(normalInterp);\n"
"    vec3 lightDir = normalize(lightPos - pos);\n"
"    float lambertian = max(dot(lightDir,normal), 0.0);\n"
"    float specular = 0.0;\n"
"     lambertian = .001;\n"
"    if(lambertian > 0.0) {\n"

"        vec3 viewDir = normalize(-pos);\n"

"        // this is blinn phong\n"
"        vec3 halfDir = normalize(lightDir + viewDir);\n"
"        float specAngle = max(dot(halfDir, normal), 0.0);\n"
"        specular = pow(specAngle, shininess);\n"
         

"    }"
"        vec3 colorLinear = ambientColor +\n"
"                     lambertian * diffuseColor +\n"
"                     specular * specColor;\n"
"       color = vec4(colorLinear, 1.f);\n"
"}";
*/




const char * fragShader_LightShader = 
"in vec2 UV;\n"
"in vec3 normalInterp;\n"
"in vec3 pos;\n"


"out vec4 color;\n"

"void main(void) {\n"
"   float reflectivity;\n"
"   if (Dynacoe_SlotHasTexture(1)) {\n"
"       reflectivity = Dynacoe_SampleShininess(1, UV);\n"
"   } else { \n"
"       reflectivity = 1.f;\n"
"   }\n"
"   color.xyz = Dynacoe_MaterialAmbient.xyz + reflectivity *  Dynacoe_CalculateLightFragment(\n"
"         pos, \n"
"         normalInterp,\n"
"         Dynacoe_MaterialDiffuseAmount, Dynacoe_MaterialDiffuse, Dynacoe_MaterialSpecularAmount, Dynacoe_MaterialSpecular, Dynacoe_MaterialShininess);\n"
"   if (Dynacoe_SlotHasTexture(0)) {\n"
"       color = mix(color, Dynacoe_SampleColor(0, UV), .5);\n"
"   }\n"
"   if (Dynacoe_CanSampleFramebuffer()) {\n"
"       color = color * Dynacoe_SampleFramebuffer(UV);\n"
"   }\n"
"   color.a = 1.0;\n"
"}\n";



const char * fragShader_LightShader21 = 
"varying vec2 UV;\n"
"varying vec3 normalInterp;\n" 
"varying vec3 pos;\n"


"void main(void) {\n"
"   vec4 color;\n"
"   float reflectivity;\n"
"   if (Dynacoe_SlotHasTexture(1)) {\n"
"       reflectivity = Dynacoe_SampleShininess(1, UV);\n"
"   } else { \n"
"       reflectivity = 1.f;\n"
"   }\n"
"   color.xyz = reflectivity *  Dynacoe_CalculateLightFragment(\n"
"         (Dynacoe_ModelTransform * vec4(pos, 1)).xyz, \n"
"         normalize(mat3(Dynacoe_ModelNormalTransform) * normalInterp),\n"
"         normalize(Dynacoe_ViewTransform * (Dynacoe_ModelTransform * vec4(pos, 1.f))).xyz,\n"
"         Dynacoe_MaterialDiffuseAmount, Dynacoe_MaterialDiffuse, Dynacoe_MaterialSpecularAmount, Dynacoe_MaterialSpecular);\n"
"   if (Dynacoe_SlotHasTexture(0)) {\n"
"       color = color*.5 + Dynacoe_SampleColor(0, UV)*.5;\n"
"   }\n"
"   if (Dynacoe_CanSampleFramebuffer()) {\n"
"       color = color * Dynacoe_SampleFramebuffer(UV);\n"
"   }\n"
"   color.a = 1.0;\n"
"   gl_FragColor = color;\n"
"}\n";













ProgramID ShaderGLRenderer::ProgramAdd(const std::string & vertexSrc, const std::string & fragSrc, std::string & log) {
    static int i = 0;
    std::string s = (Dynacoe::Chain("ShaderProgram_") << i++);
    StaticProgram * p = CreateStaticProgram();
    if (!p->Set(
        vertexSrc.c_str(),
        fragSrc.c_str(),
        s,
        
        buffers.Find(mainViewUniform)      ,
        buffers.Find(mainProjectionUniform),
        buffers.Find(mainTextureUniform)   ,
        buffers.Find(mainTextureUniform2)  ,
        buffers.Find(mainLightUniform)     ,
        buffers.Find(mainLightUniform2)     
    )) {
        log = p->GetLog();
        delete p;
        return ProgramID();
    }
    
    log = p->GetLog();
    return shaderPrograms.Insert(p);
}



void ShaderGLRenderer::initGL() {

    valid = checkSupported();
    if (!valid) {
        gl3fatal("One or more required OpenGL features are not supported on this machine");
        return;
    }



    drawMode = GL_TRIANGLES;
    lightsDirty = true;
    lightDataSrc = nullptr;

    bool gl3 = false;
    if (GLVersionQuery(GL_Version3_0 | GL_UniformBufferObject) ||
        GLVersionQuery(GL_Version3_1))
        gl3 = true;
    

    

    RenderBuffer* viewUniform, *projUniform, *texUniform, *lightUniform, *texUniform2, *lightUniform2;
    

    RenderBuffer * newBuffer = CreateRenderBuffer();
    newBuffer->Define(nullptr, 32);
    newBuffer->SetType(GL_UNIFORM_BUFFER);
    mainViewUniform = buffers.Insert(newBuffer);
    viewUniform = newBuffer;

    newBuffer = CreateRenderBuffer();
    newBuffer->Define(IdentityMatrix, 16);
    newBuffer->SetType(GL_UNIFORM_BUFFER);
    mainProjectionUniform = buffers.Insert(newBuffer);
    projUniform = newBuffer;    


    newBuffer = CreateRenderBuffer();
    newBuffer->Define(nullptr, 1024*4);
    newBuffer->SetType(GL_UNIFORM_BUFFER);
    mainTextureUniform = buffers.Insert(newBuffer);
    texUniform = newBuffer;  

    newBuffer = CreateRenderBuffer(!gl3);
    newBuffer->Define(nullptr, 1024*4);
    newBuffer->SetType(GL_UNIFORM_BUFFER);
    mainTextureUniform2 = buffers.Insert(newBuffer);
    texUniform2 = newBuffer; 



    
    
    newBuffer = CreateRenderBuffer();
    newBuffer->Define(nullptr, 1024*4);
    newBuffer->SetType(GL_UNIFORM_BUFFER);
    mainLightUniform = buffers.Insert(newBuffer);
    lightUniform = newBuffer;  

    newBuffer = CreateRenderBuffer(!gl3);
    newBuffer->Define(nullptr, 1024*4);
    newBuffer->SetType(GL_UNIFORM_BUFFER);
    mainLightUniform2 = buffers.Insert(newBuffer);
    lightUniform2 = newBuffer;  

    float endBuffer = -1;
    newBuffer->UpdateData(&endBuffer, 0, 1);

    StaticProgram * program;
    program = CreateStaticProgram();
    if (!program->Set(
        gl3 ? vertShader_BasicShader : vertShader_BasicShader21,
        gl3 ? fragShader_BasicShader : fragShader_BasicShader21,
        "Basic Shader",
        viewUniform,
        projUniform,
        texUniform,
        texUniform2,
        lightUniform,
        lightUniform2
    )) {
        cout << program->GetLog() << endl;
    }
    basicProgramID = shaderPrograms.Insert(program);
    program = CreateStaticProgram();
    if (!program->Set(
        gl3 ? vertShader_LightShader : vertShader_LightShader21,
        gl3 ? fragShader_LightShader : fragShader_LightShader21,
        "Light Shader",
        viewUniform,
        projUniform,
        texUniform,
        texUniform2,
        lightUniform,
        lightUniform2
    )) {
        cout << program->GetLog() << endl;
        
    }    
    lightProgramID = shaderPrograms.Insert(program);


    glDisable(GL_CULL_FACE);

    assert(glGetError() == GL_NO_ERROR);
}
struct VertexData{
    GLfloat
    x, y, z, w,
    r, g, b, a;
};


void ShaderGLRenderer::framebufferCheck() {
    if (!framebuffer) return;
    if (framebufferW != framebuffer->Width() ||
        framebufferH != framebuffer->Height()||
        framebufferHandle != (*(GLRenderTarget**)framebuffer->GetHandle())->GetTexture()) {

        //renderTarget->SetTexture(*((GLuint*)framebuffer->GetHandle()));

        framebufferW = framebuffer->Width();
        framebufferH = framebuffer->Height();
        framebufferHandle = (*(GLRenderTarget**)framebuffer->GetHandle())->GetTexture();
        (*(GLRenderTarget**)framebuffer->GetHandle())->DrawTo();

    }
}






void ShaderGLRenderer::resolveDisplayMode(
    ShaderGLRenderer::Polygon p, ShaderGLRenderer::DepthTest d, ShaderGLRenderer::AlphaRule a) {

    
    switch(p) {
        case ShaderGLRenderer::Polygon::Triangle: drawMode = GL_TRIANGLES; curPolygon = p; break;
        case ShaderGLRenderer::Polygon::Line: drawMode = GL_LINES;  curPolygon = p; break;
        default:;


    }

    /*
    switch(e) {
        case Renderer::EtchRule::NoEtching: curEtchRule = e; glDisable(GL_STENCIL_TEST); break;
        case Renderer::EtchRule::EtchDefine: 
            curEtchRule = e; 
            glEnable(GL_STENCIL_TEST); 
            glStencilFunc(GL_ALWAYS, 1, 0xff);
            glStencilMask(0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

        case Renderer::EtchRule::EtchUndefine: 
            curEtchRule = e; 
            glEnable(GL_STENCIL_TEST); 
            glStencilFunc(GL_ALWAYS, 0, 0xff);
            glStencilMask(0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

        case Renderer::EtchRule::EtchIn: 
            curEtchRule = e; 
            glEnable(GL_STENCIL_TEST); 
            glStencilFunc(GL_EQUAL, 1, 0xff);
            glStencilMask(0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            break;


    }
    */


    switch(d) {
        case ShaderGLRenderer::DepthTest::NoTest:  glDisable(GL_DEPTH_TEST); curDepthTest = d; break;
        case ShaderGLRenderer::DepthTest::Greater: glEnable(GL_DEPTH_TEST); glDepthFunc(GL_GREATER); curDepthTest = d; break;
        case ShaderGLRenderer::DepthTest::Less:    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); curDepthTest = d; break;
        case ShaderGLRenderer::DepthTest::GEQ:     glEnable(GL_DEPTH_TEST); glDepthFunc(GL_GEQUAL); curDepthTest = d; break;
        case ShaderGLRenderer::DepthTest::LEQ:     glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); curDepthTest = d; break;
        default:;
    }

    switch(a) {
        case ShaderGLRenderer::AlphaRule::Allow:
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);
            glAlphaFunc(GL_GREATER,0);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
            curAlphaRule = a;
            break;

        case ShaderGLRenderer::AlphaRule::Opaque:
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
            curAlphaRule = a;
            break;


        case ShaderGLRenderer::AlphaRule::Translucent:
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);
            glAlphaFunc(GL_GREATER,0);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
            curAlphaRule = a;
            break;

        case Renderer::AlphaRule::Invisible:
            glDisable(GL_ALPHA_TEST);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
            curAlphaRule = a;

        default:;

    }


}







void ShaderGLRenderer::gl3warning(const char * str) {
    cout << "[Dynacoe::ShaderGL (Warning)]:" << str << endl;
}

void ShaderGLRenderer::gl3fatal(const char * str) {
    cout << "[Dynacoe::ShaderGL (Error)]:" << str << endl;
}






bool ShaderGLRenderer::checkSupported() {
    // assumes glewInit was already called by the display that setup the context
    /*
    if (!glewIsSupported("GL_VERSION_3_0")) {
        gl3fatal("Version 3.0 of OpenGL is required for this renderer backend");
        gl3fatal("It looks like OpenGL exists and works, though. Here is your OpenGL version string:");
        gl3fatal((const char *)glGetString(GL_VERSION));
        return false;
    } else if (!glewIsSupported("GL_ARB_uniform_buffer_object")) {
        gl3fatal("The OpenGL extension 'uniform buffer object' is required for this renderer backend");
        gl3fatal("Though, this means we detected that an environment of OpenGL 3.0 or higher was found.");
        gl3fatal("Usually this can be resolved by updating your graphics drivers");
        return false;
    }
    */
    return true;
}

// statics
#ifdef DC_SUBSYSTEM_WIN32
static LRESULT CALLBACK Win32_SpawnWindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

bool ShaderGLRenderer::createContext() {
    #ifdef DC_SUBSYSTEM_X11
        
        #include <X11/X.h>
        #include <X11/Xlib.h>


        static X11Display * dpy;
        static XVisualInfo * vi;
        static Colormap cmap;
        static XSetWindowAttributes swa;
        static Window win;
        static Window root;
        static GLXContext glc;
        static XWindowAttributes gwa;
        static XEvent xev;        

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

            None };
        dpy = XOpenDisplay(NULL);

         
        if (!dpy) {
            cout << "[OpenGL-ShaderGLRenderer]: Could not connect to X server..." << endl;
            return false;
        }
                
        root = DefaultRootWindow(dpy);
        vi = glXChooseVisual(dpy, 0, att);

        if(!vi) {
            cout << "[OpenGL-ShaderGLRenderer]: Could not acquire X11 visual..."  << endl;
            return false;
        }


        cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | StructureNotifyMask
		    | KeyPressMask | KeyReleaseMask
		    | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
         
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

        //XMapWindow(dpy, win);
        XStoreName(dpy, win, "ShaderGLRenderer");

        glc = glXGetCurrentContext();
        if (!glc) {
            glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
            glXMakeCurrent(dpy, win, glc);
        }
        
        

        //XUnmapWindow(dpy, win);
        XFlush(dpy);
        if (!GLVersionInit()) return false;
       



        return true;

    #endif

    #ifdef DC_SUBSYSTEM_WIN32
        if (!wglGetCurrentContext()) {
            WNDCLASS wc = {};
            wc.lpfnWndProc = Win32_SpawnWindowCallback;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = (LPCTSTR)L"ShaderCONTEXTWND";

            RegisterClass(&wc);

            HWND handle = CreateWindowEx(0,
                (LPCTSTR)L"ShaderCONTEXTWND",
                (LPCTSTR)L"ShaderCONTEXTWND",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                128,
                128,
                0,
                0,
                GetModuleHandle(NULL),
                this
            );
            ShowWindow(handle, SW_HIDE);

            while(!wglGetCurrentContext());

        }
        return true;
        //assert();
    #endif





}


#ifdef DC_SUBSYSTEM_WIN32
LRESULT CALLBACK Win32_SpawnWindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    
    
    switch (uMsg) {

      case WM_CREATE:
      {
            HDC     deviceHandle; 
            deviceHandle = GetDC(hWnd);
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
	        HGLRC context = wglCreateContext(deviceHandle);
            wglMakeCurrent(
                deviceHandle, 
                context
            );
            // we set it to the first time to make sure that there is always a valid context
            if (!GLVersionInit()) return false;
            /*
            GLint attribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
                WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                // Uncomment this for forward compatibility mode
                //WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                // Uncomment this for Compatibility profile
                //WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                // We are using Core profile here
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
            };

            context = wglCreateContextAttribsARB(deviceHandle, 0, attribs);
            wglMakeCurrent(
                deviceHandle, 
                context
            );
            */

            
        return 0;    
      }
    }
    return 1;
}




#endif

#endif
