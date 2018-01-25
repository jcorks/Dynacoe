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

#ifndef H_DC_UTIL_MATH_INCLUDED
#define H_DC_UTIL_MATH_INCLUDED

#include <Dynacoe/Util/Vector.h>

/* Static class that does basic value manipulation and computation
   geared towards graphical applications 

   Johnathan Corkery 2015
    
   */



namespace Dynacoe {

/// \brief Utility providing some additional math operations.
///
class Math {
  public:
    /// \name Clamp functions
    ///
    /// Clamping bounds the first argument, i, to be within min and max.
    /// If i is less than min, it is set to min. If it is greater than max,
    /// i is set to max.
    ///\{ 
    static void Clamp(double &i, double min, double max);
    static void Clamp(int&, int min, int max);
    static void Clamp(float &i, float min, float max);
    ///\}

    /// \name Snapping functions
    ///
    /// Snapping binds a value, i, to the nearest multiple of a value,
    /// subdiv.
    ///\{
    static void SnapToNearest(int & i, int subdiv);
    static void SnapToNearest(float & i, float subdiv);
    static void SnapToNearest(double & i, double subdiv);
    ///\}

    /// \brief Makes a value approach another.
    ///
    /// @param f The value to modify.
    /// @param dest The value to approach.
    /// @param stepSize The rate at which to approach dest.
    static float Step(float & f, float dest, float stepSize);

    /// \brief Returns sqrt((pow(a, 2) + pow(b, 2)). 
    ///
    static float GetHypotenuse(float a,float  b);
    
    /// \name Radian <-> degree conversion.
    ///
    /// \{
    static float RadsToDegrees(float rads);
    static float DegreesToRads(float degrees);
    /// \}

    /// \brief Returns a standard pi value.
    ///
    static float Pi();

    /// \brief Returns whether or not a point is within a rectangle bounding box.
    /// The edge of the bounding box is considered within bounds.
    ///
    /// @param src The point to test.
    /// @param boundPos The topleft corner of the box.
    /// @param boundW The width of the bounding box.
    /// @param boundH The height of the bounding box.
    static bool IsVectorWithinBounds(const Dynacoe::Vector & src,
                                       const Dynacoe::Vector & boundPos,
                                       float boundW, float boundH);


    /// \brief Checks to see if a point is within a certain distance.
    ///
    static bool IsVectorWithinRange(const Dynacoe::Vector & p1, 
                                   const Dynacoe::Vector & p2,
                                   float range);
};
}

#endif
