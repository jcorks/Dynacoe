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

#include <Dynacoe/Decoders/DecodePLY.h>
#include <Dynacoe/Mesh.h>
#include <Dynacoe/Util/Iobuffer.h>
#include <Dynacoe/Model.h>
using namespace Dynacoe;
using Dynacoe::Vector;
using Dynacoe::Chain;
using Dynacoe::InputBuffer;
using std::vector;

Asset * DecodePLY::operator()(
    const std::string & fname,
    const std::string &,
    const uint8_t * buffer,
    uint64_t size
) {

    

    Model * out = nullptr;
    return out;
    /*
    
    InputBuffer in;
    in.OpenBuffer(std::vector<uint8_t>(buffer, buffer+size));
    Chain src = in.ReadString(in.Size());
    int stage = 0;


    Chain format   = "ascii";
    Chain version  = "0.f";
    Chain comments = "";
    vector<vector<float>> properties;
    vector<Chain> propertyNames;
    vector<vector<float>> elements;
    vector<Chain> elementNames;
    int curElement = 0;

    // header

    if (src.GetLink() != "ply") {
         Console::Info()  << ("Header incorrect. Missing \" ply \".\n");
    }
    src++;

    while (stage == 0 && src.LinksLeft()) {
        Chain curToken = src.GetLink(); src++;

        // reading format
        if (curToken.ToString() == "format") {
            if (src.GetLink() != "ascii" ||
                src.GetLink() != "binary_big_endian" ||
                src.GetLink() != "binary_little_endian") {
                    Chain out = "Header error: incorrect or unsupported format listed";
                    out << src.GetLink();
            } else  {
                format = src.GetLink();
            }
            src++;
            version = src.GetLink(); src++;

        } else if (curToken.ToString() == "comment") {
        // comments appended to one text buffer

            comments << src.GetLink(); src++;
        } else if (curToken.ToString() == "element") {
        // element <element name> <num entries>
        // can use the num entries to spot check the element list later

            curElement = elementNames.size();
            elementNames.push_back(src.GetLink()); src++;
            int size = atoi(src.GetLink().c_str());
            elements.push_back(vector<float>(size));




        } else if (curToken.ToString() == "property") {
        // a propoerty is always of the currently defined element

        }










    }

    return out;
    */


}

