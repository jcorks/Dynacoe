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


#include <Dynacoe/Components/RenderLight.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Util/Math.h>

using namespace Dynacoe;

RenderLight::RenderLight() : Component("RenderLight"){}
RenderLight::~RenderLight() {
    Clear();
}

void RenderLight::Clear() {
    if (!(lightID == LightID())) {
        Graphics::GetRenderer()->RemoveLight(lightID);
        lightID = LightID();
    }
}

void RenderLight::FormLight(Light light) {
    Clear();

    lightID = Graphics::GetRenderer()->AddLight((Renderer::LightType)((int)light));
    state.intensity = 10.f;
    state.color = Color(1.f, 1.f, 1.f);
    type = light;
    Enable(true);
    Draw();
}


void RenderLight::Enable(bool doIt) {
    Graphics::GetRenderer()->EnableLight(lightID, doIt);
    enabled = doIt;
}

void RenderLight::OnDraw() {
    Vector base = state.position;
    
    if (type != Light::Directional) {
        base = GetHost()->GetGlobalTransform().Transform(base);
    } 
    
    //state.position = p->local.position;
    
    if (!(current == state) || base != currentPos) {
        float data[7];
        data[0] = base.x;
        data[1] = base.y;
        data[2] = base.z;

        data[3] = state.color.r;
        data[4] = state.color.g;
        data[5] = state.color.b;

        data[6] = state.intensity;
        Math::Clamp(data[6], 0, 1000000000.f);
        Graphics::GetRenderer()->UpdateLightAttributes(lightID, data);  
        current = state;
        currentPos = base;
    }
}

std::string RenderLight::GetInfo() {
    Chain typeStr;
    switch(type) {
      case Light::Point:       typeStr = "Vector"; break;
      case Light::Directional: typeStr = "Directional"; break;
      default: typeStr = "??";
    }
    
    return (Chain() <<
        "Type      : " << typeStr << "\n" <<
        "Enabled?  : " << (enabled ? "Yep" : "Nope") << "\n" <<
        "Position  : " << state.position << "\n"
        "Intensity : " << state.intensity
    );
    
}

