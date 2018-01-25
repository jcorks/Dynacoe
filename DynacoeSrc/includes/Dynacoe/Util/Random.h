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

#ifndef H_DC_RANDOM_UTIL_INCLUDED
#define H_DC_RANDOM_UTIL_INCLUDED


#include <cstdint>
#include <Dynacoe/Util/Vector.h>

namespace Dynacoe {
/// \brief Convenient RNG
///
class Random {
  public:
    /// \brief Seeds the random number generator
    ///
    static void Seed();

    /// \brief Seeds the random number generator with a specified number
    ///
    static void Seed(uint64_t);
    
    /// \brief Returns a random value between 0 and 1.
    ///
    static float Value();

    /// \brief Returns a random value between min and max.
    ///
    static float Spread(float min, float max);

    /// \brief Returns a random integer between 0 and max.
    ///
    static int Integer(int max);

    /// Returns a random point within the unit sphere.
    ///
    static Dynacoe::Vector Vector();


};
}
#endif
