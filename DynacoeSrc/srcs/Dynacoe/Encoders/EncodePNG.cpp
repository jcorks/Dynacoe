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


#include <Dynacoe/Encoders/EncodePNG.h>
#ifdef DC_SUBSYSTEM_X11
    #include <libpng/png.h>
#else
    #ifdef _MBCS
        #include <png.h>
    #elif ANDROID
        #include <libpng/png.h>
    #else
        #include <libpng12/png.h>
    #endif
#endif
#include <Dynacoe/Image.h>

using namespace Dynacoe;

EncodePNG::EncodePNG() :
    Encoder(Assets::Type::Image, "png"){};

bool EncodePNG::operator()(Asset * src, const std::string & str, const std::string & path) {

    FILE * fp = fopen(path.c_str(), "wb");
    if (!fp) return false;
    
    png_structp  png     = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop    pngInfo = png_create_info_struct(png);

    if (!png || !pngInfo) return false;


    Image * img = (Image*)src;

    std::vector<uint8_t> frame = img->frames[0].GetData();
    uint32_t height = img->frames[0].Height();
    uint32_t width  = img->frames[0].Width();
    uint8_t ** rows = new uint8_t*[height];
    for(uint32_t i = 0; i < height; ++i) {
        rows[i] = &frame[width * i * 4];
    }


    png_init_io(png, fp);
    png_set_IHDR(
        png,
        pngInfo,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );
    png_write_info(png, pngInfo);
    png_write_image(png, rows);
    png_write_end(png, nullptr);

    delete[] rows;
    fclose(fp);

    return true;

}
