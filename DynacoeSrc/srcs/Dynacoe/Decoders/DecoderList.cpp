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

#include <Dynacoe/Modules/Assets.h> 

#include <Dynacoe/Decoders/DecodePNG.h>
#include <Dynacoe/Decoders/DecodeOGG.h>
#include <Dynacoe/Decoders/DecodeWAV.h>
#include <Dynacoe/Decoders/DecodeOBJ.h>
#include <Dynacoe/Decoders/DecodePLY.h>

#include <Dynacoe/Decoders/DecodeParticle.h>
#include <Dynacoe/Decoders/DecodeFontBasic.h>
#include <Dynacoe/Decoders/DecodeRawData.h>

#ifdef DC_EXTENSION_EXTRA_DECODERS
    #include <Dynacoe/Extensions/Decode3D.h>
#endif


using namespace Dynacoe;

void Assets::LoadDecoders() {
    LoadDecoder(new DecodeOGG());
    LoadDecoder(new DecodePNG());
    LoadDecoder(new DecodeWAV());
    LoadDecoder(new DecodeOBJ());   
    LoadDecoder(new DecodeParticle());
    LoadDecoder(new DecodeFontBasic());
    LoadDecoder(new DecodeRawData());
    
    #ifdef DC_EXTENSION_EXTRA_DECODERS
        LoadDecoder(new Decode3D());
    #endif
}
