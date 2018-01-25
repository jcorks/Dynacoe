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

#if ( defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32 || defined DC_BACKENDS_LEGACYGL_WIN32 || defined DC_BACKENDS_LEGACYGL_X11)

#include <Dynacoe/Backends/Framebuffer/OpenGLFB_Multi.h>
#include <Dynacoe/Backends/Framebuffer/OpenGLFB/GLRenderTarget.h>


#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Image.h>


using namespace Dynacoe;

OpenGLFB::OpenGLFB() : Framebuffer(
        Framebuffer::Type::GLFBPacket,
        640,
        480,
        &rt
    ){
    rt = CreateGLRenderTarget();
    OnResize(&rt, 640, 480);
}

OpenGLFB::~OpenGLFB() {
    delete rt;
}


bool OpenGLFB::OnResize(void * source, int newW, int newH) {
    rt->Resize(newW, newH);
    return true;
}


bool OpenGLFB::GetRawData(uint8_t * data) {
    rt->Sync();
    rt->GetRawData(data);
    return true;
}

std::string OpenGLFB::Name() {return "OpenGL Framebuffer";}
std::string OpenGLFB::Version() {return "v1.0 (OpenGL 3.0 or framebuffer_obj_EXT)";}
bool        OpenGLFB::Valid() {return true;}

std::string OpenGLFB::RunCommand(const std::string & cmd, uint8_t *) {
    if (cmd == "fill-debug") {
        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

    }
    if (cmd == "dump-texture") {
        GLint curTex;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &curTex);

        //Directly retrieve texture bytes (RGBA);
        glBindTexture(GL_TEXTURE_2D, rt->GetTexture());
        uint32_t width = Width();
        uint32_t height = Height();
        uint32_t numBytes = width*height*4;
        uint8_t * data = new uint8_t[numBytes];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // dump it to an image
        AssetID id = Assets::New(Assets::Type::Image, "dump-texture");
        if (id == AssetID()) {
            return "Temporary image creation failed";
        }

        for(int i = 0; i < width; ++i) {
            data[i*4] = 255;
            data[i*4+1] = 0;
            data[i*4+2] = 255;
            data[i*4+3] = 255;

        }

        {
            Image & img = Assets::Get<Image>(id);
            img.frames.push_back(Image::Frame(width, height, std::vector<uint8_t>(data, data+numBytes)));
            if (!Assets::Write(id, "png", "dump-texture.png")) {
                return "Failed to write texture.";
            }
        }
        Assets::Remove(id);

        delete[] data;
        Chain output;
        output << "Dumped framebuffer (" << width << "x" << height
               << " tex object " << rt->GetTexture();
        return output;
    }
}


void OpenGLFB::OnFilterChange(bool b) {
    rt->SetFiltering(b);
}

#endif
