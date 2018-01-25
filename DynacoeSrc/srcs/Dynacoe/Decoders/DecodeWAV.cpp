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

#include <Dynacoe/Decoders/DecodeWAV.h>
#include <vorbis/vorbisfile.h>
#include <Dynacoe/Modules/Sound.h>
#include <Dynacoe/Util/Iobuffer.h>
#include <Dynacoe/AudioBlock.h>
#include <climits>
#include <iostream>

using std::string;
using std::cout;
using std::endl;
using namespace Dynacoe;
using Dynacoe::InputBuffer;

Asset * DecodeWAV::operator()(
        const std::string & fileName, 
        const std::string &,
        const uint8_t * buffer, 
        uint64_t sizeB
    ) {



    AudioBlock * out = new AudioBlock(fileName);
    InputBuffer buf;
    buf.OpenBuffer(std::vector<uint8_t>(buffer, buffer+sizeB));
    if (buf.ReachedEnd()) {
        Console::Warning() <<"failed to load file " << fileName << "! Is it there?\n";
        return NULL;
    }    

    string proper = "";
    int headerSize = 0;
    uint32_t size;

    for(int i = 0; i < 4; ++i) {
        proper += buf.Read<uint8_t>();
    }

    if (proper != "RIFF") {
        Console::Warning()  << "Couldn't load the WAV " << fileName.c_str()
               << ", Bad RIFF identifier header.\n";
        return NULL;
    }
    proper = "";

    for(int i = 0; i < 4; ++i) {
        buf.Read<uint8_t>(); // toss out whole file size. not useful here!
    }

    for(int i = 0; i < 4; ++i) {
        proper += buf.Read<uint8_t>();
    }

    if (proper != "WAVE") {
        Console::Error()  << "Couldn't load WAV; Bad WAVE identifier header.\n";
        return NULL;
    }

    proper = "";

    while(!buf.ReachedEnd()) {
        string chunkID = "";
        int chunkSize;

        for(int i = 0; i < 4; ++i) {
            chunkID += buf.Read<uint8_t>();
            ++headerSize;
        }

        if (
            chunkID == "DISP") {

        } else if (chunkID != "data") {

            chunkSize = buf.Read<uint32_t>();
            headerSize += 4;
            for(int i = 0; i < chunkSize; ++i) {
                buf.Read<uint8_t>();
                ++headerSize;
            }
            //cout << "Chunk ID " << chunkID << ", " << chunkSize << "bytes" << endl;

        } else {
            size = buf.Read<uint32_t>();
            headerSize += 4;
            break;
        }
    }

    if (buf.ReachedEnd()) {
        Console::Info()  << "Couldn't load WAV; failed to find DATA id.\n";
        return NULL;
    }


    // Console::Info() << BeginInfo << "Size of WAV header is " << headerSize << " additional bytes before data.";
    //int sizeOfStdVal = sizeof(AudioSample);
    


    /*
    for (int i = 0; i < size; ++i) {
        data[i] = buf.Read<uint8_t>();
    }
    */
    
    std::vector<uint8_t> data = buf.ReadBytes(size);

    out->Append((AudioSample*)(&data[0]), size/sizeof(AudioSample));
    
    out->SetVolume(.9);
    


    return out;

}



