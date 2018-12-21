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

#include <Dynacoe/Backends/Backend.h>

#include <Dynacoe/Backends/AudioManager/NoAudio_Multi.h>
#include <Dynacoe/Backends/AudioManager/RtAudio_Multi.h>


#include <Dynacoe/Backends/Display/NoDisplay_Multi.h>
#include <Dynacoe/Backends/Display/OpenGLFramebuffer_Multi.h>
#include <Dynacoe/Backends/Display/AndroidBridge_Multi.h>

#include <Dynacoe/Backends/InputManager/Gainput_Multi.h>
#include <Dynacoe/Backends/InputManager/AndroidBridge_Input.h>
#include <Dynacoe/Backends/InputManager/NoInput_Multi.h>
#include <Dynacoe/Backends/InputManager/X11Input_X11.h>

#include <Dynacoe/Backends/Renderer/ShaderGL_Multi.h>
#include <Dynacoe/Backends/Renderer/NoRender_Multi.h>
#include <Dynacoe/Backends/Renderer/RendererES.h>


#include <Dynacoe/Backends/Framebuffer/OpenGLFB_Multi.h>
#include <Dynacoe/Backends/Framebuffer/NOFB_Multi.h>

using namespace Dynacoe;


class Command_Backend_RerouteHelp : public Interpreter::Command {
  public:
    Backend * backend;
    Command_Backend_RerouteHelp(Backend * in) {
        backend = in;
    }
    std::string operator()(const std::vector<std::string> & argvec) {
        return backend->RunCommand("help");
    }
    
    std::string Help() const { return ""; }
};

Backend::Backend() {
    commander = new Interpreter;
    commander->AddCommand("", new Command_Backend_RerouteHelp(this));
}

Backend::~Backend() {
    delete commander;
}

Backend * Backend::CreateDefaultRenderer() {
    #if(defined DC_BACKENDS_SHADERGL_X11 | defined DC_BACKENDS_SHADERGL_WIN32)
    return new ShaderGLRenderer();
    #endif

    #if (defined DC_BACKENDS_GLES_X11)
    return new GLES2();
    #endif
    return new NoRenderer();
}



Backend * Backend::CreateDefaultAudioManager() {
    #if defined(DC_BACKENDS_RTAUDIO_WIN32) || defined(DC_BACKENDS_RTAUDIO_ALSA) || defined(DC_BACKENDS_RTAUDIO_OSS)
    return new RtAudioManager();
    #endif

    return new NoAudioManager();
}


Backend * Backend::CreateDefaultInputManager() {
    #ifdef ANDROID 
    return new AndroidBridge_Input();
    #endif
    #if (defined DC_BACKENDS_X11INPUT_X11)
    return new X11InputManager();
    #endif
    #if (defined DC_BACKENDS_GAINPUTX11 || defined DC_BACKENDS_GAINPUTWIN32)
    return new GainputManager();
    #endif

    return new NoInputManager();

}


Backend * Backend::CreateDefaultDisplay() {
    #ifdef ANDROID
        return new AndroidBridge_Display();
    #else
        #if(defined DC_BACKENDS_OPENGLFRAMEBUFFER_X11 || defined DC_BACKENDS_OPENGLFRAMEBUFFER_WIN32 || defined DC_BACKENDS_GLESFRAMEBUFFER_X11)
        return new OpenGLFBDisplay();
        #endif
        return new NoDisplay();
    #endif

}


Backend * Backend::CreateDefaultFramebuffer() {
    #if(defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32 || DC_BACKENDS_LEGACYGL_WIN32 || DC_BACKENDS_LEGACYGL_X11 || DC_BACKENDS_GLESFRAMEBUFFER_X11)
    return new OpenGLFB();
    #endif
    return new NoFB();
}


std::string Backend::RunCommand(const std::string & command) {
    return commander->RunCommand(command);
}
