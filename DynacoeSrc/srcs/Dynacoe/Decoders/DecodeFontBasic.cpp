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

#include <Dynacoe/Decoders/DecodeFontBasic.h>
#include <Dynacoe/FontAsset.h>
#ifdef DC_OS_LINUX
    #include <ft2build.h>
    #include <ftbitmap.h>
    #include <ftglyph.h>
    #include FT_FREETYPE_H
#else
    #include <ft2build.h>
    #include <freetype/ftbitmap.h>
    #include <freetype/ftglyph.h>
    #include FT_FREETYPE_H
#endif

using namespace Dynacoe;

static FT_Library FTlib;
bool DecodeFontBasic::inited = false;

Asset * DecodeFontBasic::operator()(
    const std::string & path,
    const std::string &,
    const uint8_t * buffer,
    uint64_t size
) {
    uint8_t * dataCopy = new uint8_t[size];
    memcpy(dataCopy, buffer, size);
    if (!inited) {
        if (FT_Init_FreeType(&FTlib)) {
            Console::Error() <<("[DecodeFontBasic]: Freetype library failed to initialize!\n");
            return nullptr;
        }
        inited = true;
    }

    FT_Face * fontFace = new FT_Face();
    FontAsset * fontFaceAsset = new FontAsset(path, (void*)fontFace, dataCopy);

    if (FT_New_Memory_Face(FTlib, dataCopy, size, 0, fontFace)) {
        Console::Error() <<(std::string("[DecodeFontBasic]: The font") + path + "exists, but could not be loaded\n");
        return nullptr;
    }

    return fontFaceAsset;
}
