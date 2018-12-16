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


#if (defined DC_BACKENDS_GLES_X11)

#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <Dynacoe/Backends/Renderer/RendererES/Light_ES.h>
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer_ES.h>

#include <cstring>
#include <set>

using namespace Dynacoe;

class LightData {
  public:
    LightData(Renderer::LightType t) {
        srcType = t;

        memset(data, 0, sizeof(float)*7);
        SetType();
        data[7] = 1.f;
    }



    float * GetData() {
        return data;
    }

    bool operator<(const LightData & other) const {
        if (data[0] < other.data[0]) return true;
        return false;
    }



    void Enable(bool src) {
        if (src) {
            SetType();
        } else {
            data[0] = 0;
        }
    }

  private:
    void SetType() {
        switch(srcType) {
          case Renderer::LightType::Point:
            data[0] = .05;
            break;
          case Renderer::LightType::Directional:
            data[0] = .15;
            break;
          case Renderer::LightType::Spot:
            data[0] = .25;
            break;
          default:
            data[0] = 0;
            break;
        }
    }

    Renderer::LightType srcType;


    float data[8];


};


class CompareLightData {
  public:
    bool operator()(LightData * const & a, LightData * const & b) const {
        return *a < *b;
    }
};

struct Dynacoe::Light_ESData {
    Light_ESData() {
        lightDataSrc  = new float[32*4];
        lightDataSrc2 = new float[32*4];
    }

    ~Light_ESData() {
        delete[] lightDataSrc;
        delete[] lightDataSrc2;
    }

    std::set<LightData*, CompareLightData> lightSet;
    Table<LightData*> lightTable;
    float * lightDataSrc;
    float * lightDataSrc2;
    bool lightsDirty;

};




Light_ES::Light_ES() {
    ES = new Light_ESData();
}

Light_ES::~Light_ES() {
    delete ES;
}   


LightID Light_ES::AddLight(Renderer::LightType type) {
    LightData * newLight = new LightData(type);
    ES->lightSet.insert(newLight);
    ES->lightsDirty = true;
    return ES->lightTable.Insert(newLight);
}



void Light_ES::UpdateLightAttributes(LightID id, float * data) {
    if (!ES->lightTable.Query(id)) return;
    LightData * lightData = ES->lightTable.Find(id);

    memcpy(lightData->GetData()+1, data, sizeof(float)*7);
    ES->lightsDirty = true;
}

void Light_ES::SyncLightBuffer(float * buffer) {
    if (!ES->lightsDirty) return;

    uint32_t i = 0;
    auto iter = ES->lightSet.begin();
    LightData * light;
    while(i < MaxEnabledLights() && iter != ES->lightSet.end()) {
        light = *iter;
        // pos
        ES->lightDataSrc[i*4+0] = light->GetData()[1];
        ES->lightDataSrc[i*4+1] = light->GetData()[2];
        ES->lightDataSrc[i*4+2] = light->GetData()[3];

        // intensity
        ES->lightDataSrc[i*4+3] = light->GetData()[7];



        // color
        ES->lightDataSrc2[i*4+0] = light->GetData()[4];
        ES->lightDataSrc2[i*4+1] = light->GetData()[5];
        ES->lightDataSrc2[i*4+2] = light->GetData()[6];

        // type
        ES->lightDataSrc2[i*4+3] = light->GetData()[0];


        i++; iter++;
    }
    ES->lightDataSrc2[i*4+3] = -1.f;


    memcpy(buffer,      ES->lightDataSrc,  32*4*sizeof(float));
    memcpy(buffer+32*4, ES->lightDataSrc2, 32*4*sizeof(float));
    ES->lightsDirty = false;
}

void Light_ES::EnableLight(LightID id, bool doIt) {
    if (!ES->lightTable.Query(id)) return;
    LightData * lightData = ES->lightTable.Find(id);

    lightData->Enable(doIt);
}

void Light_ES::RemoveLight(LightID id) {
    if (!ES->lightTable.Query(id)) return;
    LightData * lightData = ES->lightTable.Find(id);
    ES->lightTable.Remove(id);
    ES->lightSet.erase(lightData);
    delete lightData;
}


int Light_ES::MaxEnabledLights() {
    return 32;
}

int Light_ES::NumLights() {
    return ES->lightSet.size();
}

#endif



