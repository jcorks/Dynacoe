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


#include <Dynacoe/Material.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Util/RefBank.h>
#include <cstring>

using namespace Dynacoe;




const float defaultState[] = {
    .1, .1, .1, 1.f,   //< ambient color
    .5, .5, .5, 1.f,   //< diffuse
    .73, .8, .85, 1.f, //< specular

    // user data
    0, 0, 0, 0,   0, 0, 0, 0,
    0, 0, 0, 0,   0, 0, 0, 0,

    0, 0, 0, 0,   0, 0, 0, 0,
    0, 0, 0, 0,   0, 0, 0, 0
};


bool Material::State::operator==(const State & other) {
    const uint8_t * dataA = (uint8_t*)this;
    const uint8_t * dataB = (uint8_t*)&other;
    for(uint16_t i = 0; i < sizeof(State); ++i) {
        if (*dataA != *dataB) return false;
        dataA++; dataB++;
    }
    return true;
}

Material::State::State() {
    ambient = {.1f, .1f, .1f, 1.f};
    diffuse = {.5f, .5f, .5f, 1.f};
    specular = {.73f, .8f, .85f, 1.f};


    specularAmount = 1.f;
    diffuseAmount = 1.f;
    shininess = 1.f;

    memset(userData, 0, sizeof(float)*32);

}




Material::Material(const Material & mat) {
    (*this) = mat;
}

Material & Material::operator=(const Material & mat) {
    state = mat.state;

    type = mat.type;
    texturesRaw = mat.texturesRaw;
    texturesSrc = mat.texturesSrc;
    textureFrameCount = mat.textureFrameCount;
    framebufferID = mat.framebufferID;
    return *this;
}

Material::Material() {
    textureFrameCount = 0;
    type = Graphics::GetRenderer()->ProgramGetBuiltIn(Renderer::BuiltInShaderMode::BasicShader);

}

Material::~Material() {
    if (buffer.Valid())
        Graphics::GetRenderer()->RemoveBuffer(buffer);
}




void Material::SetProgram(CoreProgram s) {
    type = GetProgramID(s);
}

void Material::SetProgram(ProgramID id) {
    type = id;
}


//TODO: SLOW SLOW SLOW PLS MAKE FASTER
void Material::AddTexture(TextureSlot slot, AssetID im) {
    if (im == AssetID()) return;
    switch(slot) {
        case TextureSlot::Color:  AddTexture(0, im); break;
        case TextureSlot::Normal: AddTexture(1, im); break;
        case TextureSlot::Shiny:  AddTexture(2, im); break;
    }
}

void Material::SetFramebufferSource(Camera & c) {
    framebufferID = c.GetID();
}


void Material::AddTexture(uint32_t i, AssetID tex) {
    texturesSrc.push_back({i, tex});
    texturesRaw.push_back({i, Assets::Get<Image>(tex).frames[0].GetHandle()});
}



void Material::NextTextureFrame() {
    // TODO: update ids properly
}
/// statics


ProgramID Material::GetProgramID(CoreProgram s) {
    switch(s) {
      case Material::CoreProgram::Basic:
        return Graphics::GetRenderer()->ProgramGetBuiltIn(Renderer::BuiltInShaderMode::BasicShader);
        break;

      case Material::CoreProgram::Lighting:
        return Graphics::GetRenderer()->ProgramGetBuiltIn(Renderer::BuiltInShaderMode::LightMaterial);
        break;




    }
    return ProgramID();
}




void Material::PopulateState(StaticState * st) {

    if (!buffer.Valid()) {
        buffer = Graphics::GetRenderer()->AddBuffer((float*)defaultState, 44);
    }
    if (!(previousState == state)) {
        float dataState[44];

        // ambient
        dataState[0] = state.ambient.r;
        dataState[1] = state.ambient.g;
        dataState[2] = state.ambient.b;
        dataState[3] = state.shininess;

        dataState[4] = state.diffuse.r;
        dataState[5] = state.diffuse.g;
        dataState[6] = state.diffuse.b;
        dataState[7] = state.diffuseAmount;

        dataState[8] = state.specular.r;
        dataState[9] = state.specular.g;
        dataState[10] = state.specular.b;
        dataState[11] = state.specularAmount;


        memcpy(dataState+12, state.userData, sizeof(float)*32);

        Graphics::GetRenderer()->UpdateBuffer(buffer, dataState, 0, 44);
        previousState = state;
    }

    st->materialData = buffer;
    st->program = type;
    st->textures = &texturesRaw;

    if (framebufferID.Valid()) {
        Camera * c = framebufferID.IdentifyAs<Camera>();
        st->samplebuffer = c->GetFramebuffer();
    }
}

std::string Material::Info() {
    float data[44];
    Graphics::GetRenderer()->ReadBuffer(buffer, data, 0, 44);
    Chain info = Chain()
     << "Ambient  Color: " << data[0] << " " << data[1] << " " << data[2] << "\n"
     << "Diffuse  Color: " << data[4] << " " << data[5] << " " << data[6]  << " amt: " << data[7] << "\n"
     << "Specular Color: " << data[8] << " " << data[9] << " " << data[10] << " amt: " << data[11] << "\n"
     << "Shininess: " << data[3] << "\n";
    if (type == Graphics::GetRenderer() ->ProgramGetBuiltIn(Renderer::BuiltInShaderMode::BasicShader)) {
        info << "Program: Basic (Flat)";
    } else if (type == Graphics::GetRenderer() ->ProgramGetBuiltIn(Renderer::BuiltInShaderMode::LightMaterial)) {
        info << "Program: Lighting";
    } else {
        info << "Program: Custom";
    }
    return info;
}
