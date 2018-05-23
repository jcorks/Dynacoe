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

#include <Dynacoe/Decoders/DecodeParticle.h>
#include <Dynacoe/Particle.h>
#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Util/Iobuffer.h>
#include <fstream>


using namespace Dynacoe;
using namespace std;

Asset * DecodeParticle::operator()(
    const std::string & filename, 
    const std::string &,
    const uint8_t * buffer,
    uint64_t size)

 {
    Particle * p = new Particle(filename);

    InputBuffer input;
    input.OpenBuffer(std::vector<uint8_t>(buffer, buffer+size));
    

    // empty / didnt exist
    if (!input.Size() || !size) {
        Console::Error()<<("[DecodeParticle]: Couldn't load particle \'" + filename + "\'\n");
        return nullptr;
    }

    // wasteful but whatever; needs to be changed down the line.
    Chain lines(input.ReadString(input.Size()).c_str());
    lines.SetDelimiters("\n\r");

    //p->getAssetName();

    std::string keyword;
    float value;
    std::string valueStr;
    Chain line;
    line.SetDelimiters(" \t:");
    while (lines.LinksLeft()) {
        line = lines.GetLink(); lines++;
        line.SetDelimiters(" \t:");
        keyword  = line.GetLink(); line++;
        valueStr = line.GetLink(); line++;
        value    = atof(valueStr.c_str());
        
        // # comments
        if (keyword.size() && keyword[0] == '#')continue;
    
        if (keyword == "image_name") {
            p->Image_name = valueStr.c_str();
        } else if (keyword == "duration_max") {
            p->duration_max = value;
        } else if (keyword == "alpha_max") {
            p->alpha_max = value;
        } else if (keyword == "xScale_max") {
            p->xScale_max = value;
        } else if (keyword == "yScale_max") {
            p->yScale_max = value;
        } else if (keyword == "multiScale_max") {
            p->multiScale_max = value;
        } else if (keyword == "rotation_max") {
            p->rotation_max = value;
        } else if (keyword == "direction_max") {
            p->direction_max = value;
        } else if (keyword == "speed_max") {
            p->speed_max = value;
        } else if (keyword == "red_max") {
            p->red_max = value;
        } else if (keyword == "blue_max") {
            p->blue_max = value;
        } else if (keyword == "green_max") {
            p->green_max = value;
        } else if (keyword == "duration_min") {
            p->duration_min = value;
        } else if (keyword == "alpha_min") {
            p->alpha_min = value;
        } else if (keyword == "xScale_min") {
            p->xScale_min = value;
        } else if (keyword == "yScale_min") {
            p->yScale_min = value;
        } else if (keyword == "multiScale_min") {
            p->multiScale_min = value;
        } else if (keyword == "rotation_min") {
            p->rotation_min = value;
        } else if (keyword == "direction_min") {
            p->direction_min = value;
        } else if (keyword == "speed_min") {
            p->speed_min = value;
        } else if (keyword == "red_min") {
            p->red_min = value;
        } else if (keyword == "blue_min") {
            p->blue_min = value;
        } else if (keyword == "green_min") {
            p->green_min = value;
        } else if (keyword == "alpha_delta_min") {
            p->alpha_delta_min = value;
        } else if (keyword == "xScale_delta_min") {
            p->xScale_delta_min = value;
        } else if (keyword == "yScale_delta_min") {
            p->yScale_delta_min = value;
        } else if (keyword == "multiScale_delta_min") {
            p->multiScale_delta_min = value;
        } else if (keyword == "rotation_delta_min") {
            p->rotation_delta_min = value;
        } else if (keyword == "direction_delta_min") {
            p->direction_delta_min = value;
        } else if (keyword == "speed_delta_min") {
            p->speed_delta_min = value;
        } else if (keyword == "red_delta_min") {
            p->red_delta_min = value;
        } else if (keyword == "blue_delta_min") {
            p->blue_delta_min = value;
        } else if (keyword == "green_delta_min") {
            p->green_delta_min = value;
        } else if (keyword == "alpha_delta_max") {
            p->alpha_delta_max = value;
        } else if (keyword == "xScale_delta_max") {
            p->xScale_delta_max = value;
        } else if (keyword == "yScale_delta_max") {
            p->yScale_delta_max = value;
        } else if (keyword == "multiScale_delta_max") {
            p->multiScale_delta_max = value;
        } else if (keyword == "rotation_delta_max") {
            p->rotation_delta_max = value;
        } else if (keyword == "direction_delta_max") {
            p->direction_delta_max = value;
        } else if (keyword == "speed_delta_max") {
            p->speed_delta_max = value;
        } else if (keyword == "red_delta_max") {
            p->red_delta_max = value;
        } else if (keyword == "blue_delta_max") {
            p->blue_delta_max = value;
        } else if (keyword == "green_delta_max") {
            p->green_delta_max = value;
        } else {
            Console::Error()<<("[DecodeParticle]: Syntax error in particle file ") << filename.c_str() << ":" << "Unrecognized keyword: " << keyword.c_str();
            Console::Warning()<<("\n[DecodeParticle]: Ignoring rest of file.\n");
            delete p;
            return nullptr;
        }
        






    }



    return p;


}
