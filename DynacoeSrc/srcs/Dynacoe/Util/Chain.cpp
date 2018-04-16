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


#include <Dynacoe/Util/Chain.h>
#include <cmath>
#include <cfloat>
#include <iostream>


const int chain_default_float_precision_c = 2;
const int chain_working_cstr_length_bytes = 256;
using namespace Dynacoe;

static char fromAlBhedTable[] = {
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,  

    'E', 'P', 'S', 'T', 'I', 'W', 'K', 'N', 'U', 'V', 'G', 'C', 'L', 'R', 'Y', 'B', 'X', 'H', 'M', 'D', 'O', 'F', 'Z', 'Q', 'A', 'J',
    0, 0, 0, 0, 0, 0,
    'e', 'p', 's', 't', 'i', 'w', 'k', 'n', 'u', 'v', 'g', 'c', 'l', 'r', 'y', 'b', 'x', 'h', 'm', 'd', 'o', 'f', 'z', 'q', 'a', 'j',


    0, 0, 0, 0, 0,
};


static char toAlBhedTable[] = {
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 

    'Y', 'P', 'L', 'T', 'A', 'V', 'K', 'R', 'E', 'Z', 'G', 'M', 'S', 'H', 'U', 'B', 'X', 'N', 'C', 'D', 'I', 'J', 'F', 'Q', 'O', 'W',
    0, 0, 0, 0, 0, 0,
    'y', 'p', 'l', 't', 'a', 'v', 'k', 'r', 'e', 'z', 'g', 'm', 's', 'h', 'u', 'b', 'x', 'n', 'c', 'd', 'i', 'j', 'f', 'q', 'o', 'w',

    0, 0, 0, 0, 0,
};

static char working_buffer[chain_working_cstr_length_bytes];



Chain::Chain()  {
    bufferPos = 0;
    links = 0;
    cLink = nullptr;
    delimiters = " \t\r\n";
}

Chain::~Chain() {
    if (cLink) {
        delete cLink;
    }
}

Chain::Chain(const char * str) : Chain() {
    *this = Chain(std::string(str));
}

Chain::Chain(char c) : Chain() {
    data = c;
}

Chain::Chain(int i) : Chain() {
    working_buffer[0] = 0;
    snprintf(working_buffer, chain_working_cstr_length_bytes, "%d", i);
    *this = Chain(working_buffer);
}

Chain::Chain(uint32_t i) : Chain() {
    working_buffer[0] = 0;
    snprintf(working_buffer, chain_working_cstr_length_bytes, "%u", i);
    *this = Chain(working_buffer);
}

Chain::Chain(uint64_t i) : Chain() {
    working_buffer[0] = 0;
    snprintf(working_buffer, chain_working_cstr_length_bytes, "%llu", i);
    *this = Chain(working_buffer);
}

Chain::Chain(double f) : Chain() {
    working_buffer[0] = 0;
    snprintf(
        working_buffer, 
        chain_working_cstr_length_bytes, 
        "%.*f", 
        chain_default_float_precision_c,
        f
    );
    *this = Chain(working_buffer);
}

Chain::Chain(const Vector & p) : Chain() {    
    *this = Chain() << p.x << ", " << p.y << ", " << p.z;
}

Chain::Chain(const Chain & c) : Chain() {
    *this = c;
}

Chain::Chain(const std::string & other) : Chain() {
    data = other;
}

Chain::Chain(const std::vector<Chain> & other) : Chain() {
    for(uint32_t i = 0; i < other.size(); ++i) {
        *this << other[i];
    }
}


Chain & Chain::operator<<(const Chain & other) {
    data += other.data;
    return *this;
}


std::string Chain::TranslateFromAlBhed() const{
    std::string newData;
    for (int i =0; i < data.size(); ++i) {
        if (fromAlBhedTable[data[i]])
            newData+= fromAlBhedTable[data[i]];
        else
            newData+= data[i];
        
    }
    return newData;
}


std::string Chain::TranslateToAlBhed() const{
// ypltavkrezgmshubxncdijfqow
    std::string newData;
    for (int i =0; i < data.size(); ++i) {
        if (toAlBhedTable[data[i]])
            newData[i] += toAlBhedTable[data[i]];
        else
            newData[i] += data[i];
    }
    return newData;
}



Chain & Chain::operator=(const Chain & target) {
    data = target.data;
    delimiters = target.delimiters;
    bufferPos = target.bufferPos;
    links = target.links;
    if (cLink) {
        delete cLink;
        cLink = nullptr;
    }
    if (target.cLink) {
        cLink = new std::string();
        (*cLink) = *target.cLink;
    }
}

Chain & Chain::operator++(int nothing) {
    NextLink();
    return (*this);
}

void Chain::SetDelimiters(const std::string & d) {
    delimiters = d;
}

void Chain::NextLink() {
    bool initial = false;
    if (!cLink) { // since nextLink was called before reading it
              // skip the first link
        cLink = new std::string();
        initial = true;
    }
    if (data.empty()) return;


    // get rid of leading delimiters
    while (!curCharGood()) {
        bufferPos++;
    }

    char * copy = new char[data.size() + 1];
    int copySize = 0;


    while(bufferPos < data.size()) {
        if (!curCharGood()) break;
        copy[copySize++] = data[bufferPos++];
    }
    copy[copySize] = '\0';
    (*cLink) = copy;
    if (initial)
        NextLink();
    delete[] copy;
}

bool Chain::curCharGood() {
    for(int i = 0; i < delimiters.size(); ++i) {
        if (data[bufferPos] == delimiters[i]) {
            return false;
        }
    }
    return true;
}

std::string Chain::GetLink() {
    if (!cLink) {
        cLink = new std::string();
        NextLink();
    }
    if (!cLink) {
        return "";
    }
    return (*cLink);
}


uint32_t Chain::GetLinkPos() {
    return bufferPos - GetLink().size();
}


bool Chain::LinksLeft() {
    if (!cLink) {
        cLink = new std::string();
        NextLink();
    }
    return (*cLink) != "";
}

void Chain::GoToFirst() {
    bufferPos = 0;
    delete cLink;
    cLink = NULL;
}



int Chain::AsInt() {
    int out = 0;
    sscanf(data.c_str(), "%d", &out);
    return out;
}


uint32_t Chain::AsUInt32() {
    uint32_t out = 0;
    sscanf(data.c_str(), "%u", &out);
    return out;
}

uint64_t Chain::AsUInt64() {
    uint64_t out = 0;
    sscanf(data.c_str(), "%llu", &out);
    return out;
}

double Chain::AsDouble() {
    double out = 0.0;
    sscanf(data.c_str(), "%lf", &out);
    return out;
}

float Chain::AsFloat() {
    float out = 0.0;
    sscanf(data.c_str(), "%f", &out);
    return out;
}

