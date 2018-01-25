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

#include <Dynacoe/Decoders/DecodePNG.h>
#include <Dynacoe/Image.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Util/Iobuffer.h>
#ifdef DC_SUBSYSTEM_WIN32
    #include "libpng12/png.h"
#else
    #define PNG_SKIP_SETJMP_CHECK // libpng already includes setjmp.h
    #include <libpng/png.h>
#endif
#include <string>

using namespace Dynacoe;
using std::string;
using std::vector;
using Dynacoe::Vector;
using Dynacoe::InputBuffer;
// main



struct PNGDataSource {
    uint64_t size;
    const uint8_t * buffer;
    uint64_t position;

};

// set to replace internal fread done by libpng.
// Each call is a request be libpng to read "size" bytes
static void png_memory_read_data(
    png_structp          png,
    png_bytep *          fetchData,
    png_size_t           fetchSize
) {
    PNGDataSource * src = (PNGDataSource*)png_get_io_ptr(png);

    // no more data, just give nuthin'
    if (fetchSize + src->position >= src->size) {
        memset(fetchData, fetchSize, 0);
        return;
    }


    memcpy(
        fetchData,
        src->position + src->buffer,
        fetchSize
    );

    src->position += fetchSize;
}

Asset * DecodePNG::operator()(const std::string & str, const std::string &, const uint8_t * buffer, uint64_t size) {
    AssetID out;
    UncompressedImage * preImage = NULL;
    Image * img = NULL;


    // get into uncompressed format
    uncompressPNG(str, buffer, size, preImage);
    if (preImage == NULL) {
        Console::Error()<<("[DecodePNG]: could not interpret image "); Console::Error()<<(str.c_str()) << "\n";
        return nullptr;
    }


    // generate proper Image
    makeImage(preImage, img, str);
    disposeOf(preImage);
    return img;

}





















void DecodePNG::expandFromRGBtoRGBA(uint8_t *& data, int size){

    uint8_t * copy = new uint8_t[size];
    memcpy(copy, data, size);

    for(int i = 0; i < size / 4; ++i) {
        for(int n = 0; n < 4; ++n) {
            if (n != 3) // all except alpha
                data[i*4 + n] = copy[(i)*3 + n];
            else
                data[i*4 + n] = 255;
        }
    }

    delete[] copy;
}

void DecodePNG::convertFromBGRAtoRGBA(uint8_t *& data, int size)  {
    uint8_t * copy = new uint8_t[size];
    memcpy(copy, data, size);

    // BGRA
    // RGBA

    for(int i = 0; i < size / 4; ++i) {
        data[i*4 + 0] = copy[i*4 + 2];
        data[i*4 + 1] = copy[i*4 + 1];
        data[i*4 + 2] = copy[i*4 + 0];
        data[i*4 + 3] = copy[i*4 + 3];
    }

    delete[] copy;
}

void DecodePNG::disposeOf(UncompressedImage *& img) {
    for(int i = 0; i < img->numFrames; ++i) {
        delete[] img->frames[i].data;
    }
    delete[] img->frames;
    delete img;
}


/*

 member function for Image

void DecodePNG::makeTileset(UncompressedImage *& img, int wImages, int hImages) {
    int subW = img->w / wImages;
    int subH = img->h / hImages;

    // gather output
    UncompressedImage * out = new UncompressedImage;
    out->w = subW;
    out->h = subH;
    out->numFrames = wImages * hImages * img->numFrames; // every frame will be tileset'd
    out->frames = new UncompressedFrame[wImages * hImages];

    for(int frameset = 0; frameset < img->numFrames; ++frameset) {
        uint8_t * subImages[wImages * hImages];
        for(int i = 0; i < wImages*hImages; ++i)
            subImages[i] = new uint8_t[4 * subW * subH];

        for(int cImage = 0; cImage < wImages * hImages; ++cImage) {
            int startX = subW*(cImage % wImages);
            int startY = subH*(cImage / hImages);

            for(int y = 0; y < subH; ++y) {
                for(int x = 0; x < subW; ++x) {


                    uint8_t * pixel = &img->frames[frameset].data[(startX + x + (startY + y)*img->w) * 4];
                    for(int i = 0; i < BYTES_PER_PIXEL; ++i) {
                        //subImages[cImage][BYTES_PER_PIXEL * (x + y * subH) + i] = pixel[i];
                        subImages[cImage][BYTES_PER_PIXEL * (x + y * subW) + i] = pixel[i];

                    }
                }
            }
        }

        // insert each sub image as a frame
        for(int i = 0; i < wImages*hImages; ++i) {
            out->frames[frameset + i].data = new uint8_t[subH*subW*4];
            memcpy(out->frames[frameset + i].data, subImages[i], 4 * subH * subW);

            delete[] subImages[i];
        }

    }

    disposeOf(img);
    img = out;
}
*/


void DecodePNG::makeImage(UncompressedImage * src, Image *& out, const std::string & name) {
    out = new Image(name);


    for(int i = 0; i < src->numFrames; ++i) {
        vector<uint8_t> w(src->frames[i].data,
                          src->frames[i].data + src->w * src->h * 4);
        out->frames.push_back(Image::Frame(src->w, src->h, w));
    }
}



static void LibPNGErrorHandler(png_struct *, png_const_charp msg) {
    printf("Uh oh: %s\n", msg);
}

void DecodePNG::uncompressPNG(
        const string & path,
        const uint8_t * buffer,
        uint64_t bufferSize,
        UncompressedImage *& img) {

    int numPixels;
    png_bytep * data;
    png_byte * image;
    img = NULL;


    PNGDataSource srcData;
    srcData.buffer = buffer;
    srcData.size = bufferSize;
    srcData.position = 0;

    png_structp pngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, LibPNGErrorHandler, NULL);
    png_infop info = png_create_info_struct(pngStruct);
    png_infop end =  png_create_info_struct(pngStruct);

    if (!pngStruct || !info || !end ) {
        numPixels = -2;
        Console::Error()<<("[DecodePNG][PNG] PNG is invalid\n");
        return;
    }


    png_set_read_fn(
        pngStruct,
        &srcData,
        (png_rw_ptr)png_memory_read_data
    );

    png_read_info(pngStruct, info);


    int w = png_get_image_width(pngStruct, info);
    int h = png_get_image_height(pngStruct, info);
    int bitDepth = png_get_bit_depth(pngStruct, info);
    int colorType = png_get_color_type(pngStruct, info);
    int rowBytes =  png_get_rowbytes(pngStruct,info);



    if (bitDepth == 16) png_set_strip_16(pngStruct);


    if (colorType != PNG_COLOR_TYPE_RGBA && colorType != PNG_COLOR_TYPE_RGB) {
        Console::Error()<<("[DecodePNG][PNG] Unsupported PNG: ");
        if (colorType == PNG_COLOR_TYPE_GRAY) {
            Console::Error()<<("Grayscale\n");
        } else if (colorType == PNG_COLOR_TYPE_PALETTE) {
            Console::Error()<<("Palette\n");
        } else if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
            Console::Error()<<("Grayscale (with alpha)\n");
        }

        numPixels = -3;
        return;
    }



    png_read_update_info(pngStruct, info); // update the info if png needed to be stripped


    data = new png_bytep[h];
    numPixels  = h * rowBytes;



    image = new png_byte[h * rowBytes];



    for(int i = 0; i < h; ++i) {
        //image[h - 1 - i] = data + (i *rowBytes);
        data[i] = image + (i *rowBytes);
    }

    png_read_image(pngStruct, data);
    png_destroy_read_struct(&pngStruct, &info, NULL);


    if (!numPixels) return;


    img = new UncompressedImage;
    img->w = w;
    img->h = h;
    img->numFrames = 1;
    img->frames = new UncompressedFrame[1];
    img->frames[0].data = new uint8_t[w*h*4];
    memcpy(img->frames[0].data, image, h* rowBytes);

    if (colorType == PNG_COLOR_TYPE_RGB) {
        //Console::Info()<<("[DecodePNG][PNG] Expanding RGB image...\n");
        expandFromRGBtoRGBA(img->frames[0].data, w*h*4);
    }

    delete[] data;
    delete[] image;

}
