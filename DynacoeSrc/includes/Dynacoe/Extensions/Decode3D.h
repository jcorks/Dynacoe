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
#ifdef DC_EXTENSION_EXTRA_DECODERS
#ifndef H_DC_DECODE_3D
#define H_DC_DECODE_3D

#include <Dynacoe/Decoders/Decoder.h>
#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Material.h>


namespace Dynacoe {
class Mesh;
class Decode3D : public Decoder {
  public:
    Decode3D() : Decoder(Assets::Type::Model, {
        "obj",
        "dae",
        "blend",
        "xml",
        "bvh",
        "3ds",
        "ase",
        "ply",
        "dxf",
        "ifc",
        "nff",
        "smd",
        "vta",
        "mdl",
        "md2",
        "md3",
        "mdc",
        "md5mesh",
        "md5anim",
        "md5camera",
        "x",
        "q3o",
        "q3s",
        "raw",
        "ac",
        "stl",
        "dxf",
        "irrmesh",
        "off",
        "ter",
        "material",
        "ms3d",
        "lwo",
        "lws",
        "lxo",
        "csm",
        "cob",
        "scn"
    }) {}
    Asset * operator()(
        const std::string & name, 
        const std::string & ext,
        const uint8_t * buffer, 
        uint64_t size
    );

};
}


#endif
#endif
