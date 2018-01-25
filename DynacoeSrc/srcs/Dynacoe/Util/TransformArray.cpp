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


#include <Dynacoe/Util/TransformArray.h>
#include <Dynacoe/Components/Mutator.h>
#include <Dynacoe/Util/Math.h>
#include <cmath>
#include <cstring>


/* Data:
 0, 1, 2        -> translation x, y, z
 3, 4, 5, 6     -> Quaternion (a, b, c, and d respectively, (aka, w x y z) where 
                               a + b*i + c*j + d*k represents the quaternion and
                               i*j*k == i^2 == j^2 == k^2 == -1 
 7, 8, 9        -> scale x, y, z */

using namespace Dynacoe;

TransformArray::TransformArray() {
    Reset();
}

void TransformArray::Reset() {
    data[0] = 0.f;
    data[1] = 0.f;
    data[2] = 0.f;

    data[3] = 1.f;
    data[4] = 0.f;
    data[5] = 0.f;
    data[6] = 0.f;

    data[7] = 1.f;
    data[8] = 1.f;
    data[9] = 1.f;
}

TransformMatrix TransformArray::Compose() {
    static TransformMatrix translation;
    static TransformMatrix scale;
    static TransformMatrix rotation;

    translation.SetToIdentity();
    scale.SetToIdentity();
    rotation.SetToIdentity();


    translation.Translate(data[0], data[1], data[2]);
    scale.Scale(data[7], data[8], data[9]);



    float * rotationData = rotation.GetData();



    // TODO: FIGURE OUT WHY INDICES 9 AND 6 ARE REVERSED
    // EVERY IMPLEMENTATION HAS THOSE CALCULATIONS THE SAME WAY,
    // BUT WHEN USED, THIER VALUES WERE SWITCHED. WHY
    
    rotationData[0] = 1 - 2*(data[5]*data[5]) - 2*(data[6]*data[6]);
    rotationData[1] = 2*data[4]*data[5] - 2*data[3]*data[6];
    rotationData[2] = 2*data[4]*data[6] + 2*data[3]*data[5];

    rotationData[4] = 2*data[4]*data[5] + 2*data[3]*data[6];  
    rotationData[5] = 1 - 2*(data[4]*data[4]) - 2*(data[6]*data[6]);
    rotationData[9] = 2*data[5]*data[6] + 2*data[3]*data[4];

    rotationData[8] = 2*data[4]*data[6] - 2*data[3]*data[5];
    rotationData[6] = 2*data[5]*data[6] - 2*data[3]*data[4];
    rotationData[10] =  1 - 2*(data[4]*data[4]) - 2*(data[5]*data[5]);
    



    return (translation * rotation) * scale;
    
}

TransformArray TransformArray::Lerp(const TransformArray & other, float amt) {
    TransformArray out;    
    for(int i = 0; i < 10; ++i) {
        out.data[i] = Mutator::StepTowards(data[i], other.data[i], amt);
    }
    return out;
}

void TransformArray::RotateByAxis(const Dynacoe::Vector & a, float amountDegrees) {
    // New quat = t * quat

    Dynacoe::Vector axis = a.Normalize();
    float t[4];
    float c[4];
    float rads = Math::DegreesToRads(amountDegrees);


    memcpy(c, &data[3], sizeof(float)*4);

    t[0] = cos(rads/2);
    t[1] = axis.x * sin(rads/2);
    t[2] = axis.y * sin(rads/2);
    t[3] = axis.z * sin(rads/2);



        
    // then multiply after defining

    data[4] = t[1]*c[0] + t[2]*c[3] - t[3]*c[2] + t[0]*c[1];
    data[5] =-t[1]*c[3] + t[2]*c[0] + t[3]*c[1] + t[0]*c[2];
    data[6] = t[1]*c[2] - t[2]*c[1] + t[3]*c[0] + t[0]*c[3];
    data[3] =-t[1]*c[1] - t[2]*c[2] - t[3]*c[3] + t[0]*c[0];

}


void TransformArray::Translate(float x, float y, float z) {
    data[0] += x;
    data[1] += y;
    data[2] += z;
}

void TransformArray::Scale(float x, float y, float z) {
    data[7] += x;
    data[8] += y;
    data[9] += z;
}



Dynacoe::Vector TransformArray::GetRotation() {
    Dynacoe::Vector out;
    out.x = atan2(2*(data[3]*data[4] + data[5]*data[6]), 1 - 2*(data[4]*data[4]  + data[5]*data[5]));
    out.y = asin(2*(data[3]*data[5] - data[4]*data[6]));
    out.z = atan2(2*(data[3]*data[6] + data[4]*data[5]), 1 - 2*(data[5]*data[5]  + data[6]*data[6]));
    
    out.x = Math::RadsToDegrees(out.x);
    out.y = Math::RadsToDegrees(out.y);
    out.z = Math::RadsToDegrees(out.z);
    

    return out;
}


Dynacoe::Chain TransformArray::Print() {
    Dynacoe::Chain out;
    Dynacoe::Vector rot;
    rot = GetRotation();
    out << "Transform Array: \n"
        << "Position      = {" << data[0] << ", " << data[1] << ", " << data[2] << "}\n"
        << "Raw QUaternion= {" << data[3] << ", " << data[4] << ", " << data[5] << ", " << data[6] << "}\n"
        << "Euler Approx. = {" << rot.x << ", " << rot.y << ", " << rot.z << "}\n"
        << "Scale         = {" << data[7] << ", " << data[8] << ", " << data[9] << "}\n";
    return out;
}
