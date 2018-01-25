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



#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Util/Vector.h>
#include <cmath>

using Dynacoe::Math;
using Dynacoe::Vector;


static float PI_FLOAT = 3.14159265358979323846f;

void Math::Clamp(double&i, double min, double max) {
    if (i > max) i = max;
    if (i < min) i = min;
}

void Math::Clamp(float&i, float min, float max) {
    if (i > max) i = max;
    if (i < min) i = min;
}   

void Math::Clamp(int&i, int min, int max) {
    if (i > max) i = max;
    if (i < min) i = min;
}

void Math::SnapToNearest(double & i, double subdiv) {
    i = round((i) / subdiv) * subdiv;
}
void Math::SnapToNearest(float & i, float subdiv) {
    i = round((i) / subdiv) * subdiv;
}

void Math::SnapToNearest(int & i, int subdiv) {
    i = round((i) / (double)subdiv) * subdiv;
}

float Math::DegreesToRads(float d) {
    return d * PI_FLOAT / 180.f;
}

float Math::RadsToDegrees(float d) {
    return d * 180.f / PI_FLOAT;
}

float Math::Step(float & f, float dest, float stepSize)  {


    if (f < dest) {
        if (f + stepSize > dest)
            f = dest;
        else
            f += stepSize;
    } else if (f > dest) {
        if (f - stepSize < dest)
            f = dest;
        else
            f -= stepSize;
    }
    return f;
}



float Math::Pi() { return PI_FLOAT; }


bool Math::IsVectorWithinBounds(const Dynacoe::Vector & pos,
                            const Dynacoe::Vector & bound,
                            float boundW, float boundH) {
    return (pos.x >= bound.x && pos.x <= bound.x + boundW &&
            pos.y >= bound.y && pos.y <= bound.y + boundH);

}

bool Math::IsVectorWithinRange(const Dynacoe::Vector & p1,
                              const Dynacoe::Vector & p2,
                              float range) {
    return p1.Distance(p2) < range;
}
