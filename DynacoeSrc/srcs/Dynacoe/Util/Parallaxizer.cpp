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


#include <Dynacoe/Util/Parallaxizer.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Util/Math.h>

using namespace Dynacoe;

static float maxParallax = 1.f;


void Parallaxizer::SetRange(float max) {
    maxParallax = max;
}



Vector Parallaxizer::Shift(const Vector & p) {
    Vector out;

    float wMax = Graphics::GetRenderCamera().Width();
    float hMax = Graphics::GetRenderCamera().Height();
    Vector vanishingVector = Graphics::GetCamera2D().node.GetPosition() + Vector(wMax/2, hMax/2);

    float influence = 1 - (p.z / maxParallax);
    Math::Clamp(influence, 0.f, 2.f);


    out.x = vanishingVector.x + (p.x - vanishingVector.x)*influence;
    out.y = vanishingVector.y + (p.y - vanishingVector.y)*influence;
    return out;
}
