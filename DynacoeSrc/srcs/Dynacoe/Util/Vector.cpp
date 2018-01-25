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



#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Util/Chain.h>
#include <cmath>


#ifndef PI_DOUBLE
#define PI_DOUBLE 3.14159265358979323846
#endif


static float PhiVec(const Dynacoe::Vector &);
static float PsiVec(const Dynacoe::Vector &);
static float ThetaVec(const Dynacoe::Vector &);


Dynacoe::Vector::Vector() {
	x = 0;
	y = 0;
	z = 0;
}

Dynacoe::Vector::Vector(float in_x, float in_y, float in_z) {
	x = in_x;
	y = in_y;
	z = in_z;
}






Dynacoe::Vector::Vector(const std::vector<float> & pts) {
    x = y = z = 0.f;

    switch(pts.size()) {
      default:
      case 3: z = pts[2];
      case 2: y = pts[1];
      case 1: x = pts[0];
      case 0: return;
    }
}

Dynacoe::Vector::Vector(const std::string & str) {
    Dynacoe::Chain c;
    c.SetDelimiters(" \n\t\r,.<>(){}[]");
    c = str;

    float source[3];
    source[0] = source[1] = source[2] = 0.f;
    int iter = 0;
    while(c.LinksLeft() && iter < 3) {
        source[iter++] = atof(c.GetLink().c_str());
        c.NextLink();
    }
    x = source[0];
    y = source[1];
    z = source[2];
}


bool Dynacoe::Vector::operator==(const Dynacoe::Vector & p) const {
    return (x == p.x &&
            y == p.y &&
            z == p.z);
}

bool Dynacoe::Vector::operator!=(const Dynacoe::Vector & p) const {
    return !(p == *this);
}

float Dynacoe::Vector::Length() const {
	return sqrt(x*x +y*y + z*z);
}



Dynacoe::Vector & Dynacoe::Vector::operator()(float in_x, float in_y) {
	x = in_x;
	y = in_y;
	return (*this);
}

Dynacoe::Vector & Dynacoe::Vector::operator()(float in_x, float in_y, float in_z) {
	x = in_x;
	y = in_y;
	z = in_z;
	return (*this);
}


Dynacoe::Vector Dynacoe::Vector::operator-() const {
    return Vector(-x, -y, -z);
}


Dynacoe::Vector & Dynacoe::Vector::operator/=(const Dynacoe::Vector & in) {
	x /= in.x;
	y /= in.y;
	z /= in.z;
	return (*this);
}
Dynacoe::Vector & Dynacoe::Vector::operator*=(const Dynacoe::Vector & in) {
	x *= in.x;
	y *= in.y;
	z *= in.z;
	return (*this);
}

Dynacoe::Vector & Dynacoe::Vector::operator+=(const Dynacoe::Vector & in) {
	x += in.x;
	y += in.y;
	z += in.z;
	return (*this);
}
Dynacoe::Vector & Dynacoe::Vector::operator-=(const Dynacoe::Vector & in) {
	x -= in.x;
	y -= in.y;
	z -= in.z;
	return (*this);
}




Dynacoe::Vector & Dynacoe::Vector::operator/=(float in) {
	x /= in;
	y /= in;
	z /= in;
	return (*this);
}
Dynacoe::Vector & Dynacoe::Vector::operator*=(float in) {
	x *= in;
	y *= in;
	z *= in;
	return (*this);
}

Dynacoe::Vector & Dynacoe::Vector::operator+=(float in) {
	x += in;
	y += in;
	z += in;
	return (*this);
}
Dynacoe::Vector & Dynacoe::Vector::operator-=(float in) {
	x -= in;
	y -= in;
	z -= in;
	return (*this);
}


Dynacoe::Vector Dynacoe::Vector::operator/(const Dynacoe::Vector & in) const {
	Vector out = (*this);
	out /= in;
	return out;
}
Dynacoe::Vector Dynacoe::Vector::operator*(const Dynacoe::Vector & in) const {
	Vector out = (*this);
	out *= in;
	return out;
}
Dynacoe::Vector Dynacoe::Vector::operator+(const Dynacoe::Vector & in) const {
	Vector out = (*this);
	out += in;
	return out;
}
Dynacoe::Vector Dynacoe::Vector::operator-(const Dynacoe::Vector & in) const {
	Vector out = (*this);
	out -= in;
	return out;
}






Dynacoe::Vector Dynacoe::Vector::operator/(float in) const {
	Vector out = (*this);
	out /= in;
	return out;
}
Dynacoe::Vector Dynacoe::Vector::operator*(float in) const {
	Vector out = (*this);
	out *= in;
	return out;
}
Dynacoe::Vector Dynacoe::Vector::operator+(float in) const {
	Vector out = (*this);
	out += in;
	return out;
}
Dynacoe::Vector Dynacoe::Vector::operator-(float in) const {
	Vector out = (*this);
	out -= in;
	return out;
}












Dynacoe::Vector Dynacoe::Vector::Normalize() const {
	Vector out = (*this);
	return out.SetToNormalize();
}


float Dynacoe::Vector::Dot(const Dynacoe::Vector & in) const {
	return ((x * in.x) + (y * in.y) + (z * in.z));
}

float Dynacoe::Vector::CrossFlat(const Dynacoe::Vector & in) const {
    return ((x * in.y) - (y * in.x));
}

Dynacoe::Vector Dynacoe::Vector::Cross(const Dynacoe::Vector & in)  const{
	Vector out;
	out.x = y *  in.z - z * in.y;
	out.y = z *  in.x - x * in.z;
	out.z = x *  in.y - y * in.x;
	return out;

}







float Dynacoe::Vector::RotationZ() const {
    return PhiVec(*this);
}

float Dynacoe::Vector::RotationZDiff(const Dynacoe::Vector & to) const {
    // If any inputs are undefined in this plane, return 0
    if ((x == 0.f && y == 0.f) || (to.x == 0.f && to.y == 0.f)) return 0;
	float out = PhiVec(to) - PhiVec(*this);
	if (out < 0) out += 360.f;
	return out;
}

float Dynacoe::Vector::RotationZDiffRelative(const Dynacoe::Vector & to) const {
	Vector out = (to - (*this));
	return out.RotationZ();
}





float Dynacoe::Vector::RotationY() const {
    return ThetaVec(*this);
}

float Dynacoe::Vector::RotationYDiff(const Dynacoe::Vector & to) const {
    // If any inputs are undefined in this plane, return 0
    if ((x == 0.f && z == 0.f) || (to.x == 0.f && to.z == 0.f)) return 0;
	float out = ThetaVec(to) - ThetaVec(*this);
	if (out < 0) out += 360.f;
	return out;
}

float Dynacoe::Vector::RotationYDiffRelative(const Dynacoe::Vector & to) const {
	Vector out = (to - (*this));
	return out.RotationY();
}



float Dynacoe::Vector::RotationX() const {
    return PsiVec(*this);
}

float Dynacoe::Vector::RotationXDiff(const Dynacoe::Vector & to) const {
    // If any inputs are undefined in this plane, return 0
    if ((y == 0.f && z == 0.f) || (to.y == 0.f && to.z == 0.f)) return 0;
	float out = PsiVec(to) - PsiVec(*this);
	if (out < 0) out += 360.f;
	return out;
}

float Dynacoe::Vector::RotationXDiffRelative(const Dynacoe::Vector & to) const {
	Vector out = (to - (*this));
	return out.RotationX();
}





Dynacoe::Vector Dynacoe::Vector::RotateZ(float f) {
    return  Vector(
        x * cos(f * (PI_DOUBLE / 180.0)) - y * sin(f * (PI_DOUBLE / 180.0)),
        y * cos(f * (PI_DOUBLE / 180.0)) + x * sin(f * (PI_DOUBLE / 180.0)),
        z
    );
}

Dynacoe::Vector Dynacoe::Vector::RotateY(float f) {
    return  Vector(
        x * cos(f * (PI_DOUBLE / 180.0)) + z * sin(f * (PI_DOUBLE / 180.0)),
        y,
        z * cos(f * (PI_DOUBLE / 180.0)) - x * sin(f * (PI_DOUBLE / 180.0))
    );
}

Dynacoe::Vector Dynacoe::Vector::RotateX(float f) {
return  Vector(
        x,
        y * cos(f * (PI_DOUBLE / 180.0)) - z * sin(f * (PI_DOUBLE / 180.0)),
        z * cos(f * (PI_DOUBLE / 180.0)) + y * sin(f * (PI_DOUBLE / 180.0))
    );
}




Dynacoe::Vector Dynacoe::Vector::RotateZFrom(const Vector & p, float f) {
    Vector out = *this - p;
    out = out.RotateZ(f);
    return out + p;
}

Dynacoe::Vector Dynacoe::Vector::RotateYFrom(const Vector & p, float f) {
    Vector out = *this - p;
    out = out.RotateY(f);
    return out + p;
}

Dynacoe::Vector Dynacoe::Vector::RotateXFrom(const Vector & p, float f) {
    Vector out = *this - p;
    out = out.RotateX(f);
    return out + p;
}




Dynacoe::Vector Dynacoe::Vector::Floor() const {
	Vector out(*this);
	return out.SetToFloor();
}

Dynacoe::Vector & Dynacoe::Vector::SetToNormalize() {
	float l = Length();
	if (!l) {
        (*this) = Vector();
        return (*this);
	}
	x = x / l;
	y = y / l;
	z = z / l;
	return (*this);
}


Dynacoe::Vector & Dynacoe::Vector::SetToFloor() {
	x = (int)(x);
	y = (int)(y);
	z = (int)(z);
	return (*this);
}

float Dynacoe::Vector::Distance(const Dynacoe::Vector & in) const {
	return sqrt(pow(in.x - x, 2) + pow(in.y - y, 2) + pow(in.z - z, 2));
}






//// private:

/*
    cos(a) = (A . B) / (A.length * B.Length);
 */

// Z
float PhiVec(const Dynacoe::Vector & in) {
	float dProduct = (in.x); // reduced dot product based around 0-degree vector {1, 0, 0}
    float lengthPlane_this = 1.f;
    float lengthPlane_in   = sqrt(in.x*in.x + in.y* in.y);
    if (lengthPlane_in == 0.f)
        return 0.f;

    float cAngle = acos(dProduct / (lengthPlane_in * lengthPlane_this));
	if (in.y < 0) cAngle = -cAngle + (PI_DOUBLE * 2.0);
	return (cAngle * (180.0 / PI_DOUBLE));
}

// X
float PsiVec(const Dynacoe::Vector & in) {
	float dProduct = (in.y); // reduced dot product based around 0-degree vector {0, 1, 0}
    float lengthPlane_this = 1.f;
    float lengthPlane_in   = sqrt(in.z*in.z + in.y* in.y);
    if (lengthPlane_in == 0.f)
        return 0.f;

    float cAngle = acos(dProduct / (lengthPlane_in * lengthPlane_this));
	if (in.z < 0) cAngle = -cAngle + (PI_DOUBLE * 2.0);
	return (cAngle * (180.0 / PI_DOUBLE));
}

// Y
float ThetaVec(const Dynacoe::Vector & in) {
	float dProduct = (in.z); // reduced dot product based around 0-degree vector {0, 0, 1}
    float lengthPlane_this = 1.f;
    float lengthPlane_in   = sqrt(in.x*in.x + in.z* in.z);
    if (lengthPlane_in == 0.f)
        return 0.f;

	float cAngle = acos(dProduct / (lengthPlane_this * lengthPlane_in));
	if (in.x < 0) cAngle = -cAngle + (PI_DOUBLE * 2.0);
	return (cAngle * (180.0 / PI_DOUBLE));
}
