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


#include <Dynacoe/Util/Random.h>
#include <random>
#include <cstdlib>
#include <iostream>
#include <ctime>

//TODO: replace with mersenne twister
using namespace Dynacoe;

namespace {
    uint64_t seed;
}

void Random::Seed() {
    seed =  time(NULL)*time(NULL);
    srand(seed);   
}

void Random::Seed(uint64_t s) {
    seed = s;
    srand(seed);
}



float Random::Value() {
    return rand() / (float) RAND_MAX;
}

float Random::Spread(float min, float max) {

    if (min > max || min == max) {
        return min;
    }
    //TODO: This is biased. Need full random
    return min + (float) rand() / ((float) RAND_MAX /(max - min));
}


int Random::Integer(int max) {
    int n = (int)Spread(0, max+1);
    return n;
}


Dynacoe::Vector Random::Vector() {
    return Dynacoe::Vector(
        Spread(-1, 1),
        Spread(-1, 1),
        Spread(-1, 1)
    );
}

