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

#if (DC_BACKENDS_GLES_X11)

#ifndef H_DC_BACKENDS_LIGHTDATA
#define H_DC_BACKENDS_LIGHTDATA

#include <GLES2/gl2.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>

namespace Dynacoe {
class Light_ESData;
class Light_ES {
  public:
    Light_ES();
    ~Light_ES();

    LightID AddLight(Renderer::LightType);

    void UpdateLightAttributes(LightID, float *);

    void EnableLight(LightID, bool doIt);

    void RemoveLight(LightID);

    int MaxEnabledLights();

    int NumLights();


    void SyncLightBuffer(float * formattedDataBuffer);

  private:
    Light_ESData * ES;
};
}


#endif
#endif
