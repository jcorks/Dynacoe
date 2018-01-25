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

#ifndef H_DC_AUDIOSAMPLE_INCLUDED
#define H_DC_AUDIOSAMPLE_INCLUDED

#include <cstdint>

namespace Dynacoe {

/// \brief A unit of sound data.
///
/// Consists of 2 pieces of data: one for the 
/// left channel and one for the right channel.

struct AudioSample {
    AudioSample(int16_t l, int16_t r) {
        leftSample = l;
        rightSample = r;
    }

    AudioSample() {
        leftSample = 0;
        rightSample = 0;
    }

    /// \brief Returns the right sample as a value from 0 to 1.
    ///
    float NormalizedR() {
        return leftSample / (float) INT16_MAX;
    }

    /// \brief Returns the left sample as a value from 0 to 1.
    ///
    float NormalizedL() {
        return leftSample / (float) INT16_MAX;
    }
    
    /// \brief The left sample.
    ///
    int16_t leftSample;

    /// \brief The right sample.
    ///
    int16_t rightSample;

};
}


#endif
