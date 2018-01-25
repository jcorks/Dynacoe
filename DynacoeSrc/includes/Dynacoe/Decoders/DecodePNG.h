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

#ifndef H_DC_DECODE_PNG
#define H_DC_DECODE_PNG

#include <Dynacoe/Decoders/Decoder.h>


/* Legacy decoder. Handles CIM, PNG, and BMP image reading */
namespace Dynacoe {
class DecodePNG : public Decoder{
  public:
    DecodePNG() : Decoder(Assets::Type::Image, std::vector<std::string>{"png"}){}
    
    Asset * operator()(
        const std::string & name, 
        const std::string & extension,
        const uint8_t * buffer, 
        uint64_t size
    );



  private:

    /* Image storage */

        // IS MY UNCOMPRESSED IMAGE WELL-FORMED?
        // Well, to be well-formed, it must have the following properties:
        // -always RGBA, (4-component unsigned byte)
        // -size of frame is always w * h * 4, any padding will be done internally by renderer
        // -numFrames, w ,h are always positive
        // -frames and frame data are accessible
        // -frames is dynamically allocated as an array (new[])
        // -frame data is dynamically allocated as an array
        struct UncompressedFrame {
            uint8_t * data;
        };

        struct UncompressedImage {
            UncompressedFrame * frames;
            int numFrames;
            int w;
            int h;
        };
        /* Uncomp'd Image Utils */
            // must have space for 32-bit image allocated already
            void expandFromRGBtoRGBA(uint8_t *&, int size) ;
            void convertFromBGRAtoRGBA(uint8_t *&, int size) ;


            // get ridda that Uncomp'd image
            void disposeOf(UncompressedImage *& img);
            // make a well-formed Image instance.
            // This requires interaction with the renderer
            void makeImage(UncompressedImage * src, Image *& out, const std::string &);
            // transforms each frame  of the original image to
            // be a series of miniature frames with a width and
            // height given by the sub division of w and height with
            // the number of slices given
            void makeTileset(UncompressedImage *&, int xDiv, int yDiv);


        // Uncompressed images and frames are only allocated by the designated uncompress fns
        void uncompressPNG(const std::string &, const uint8_t *, uint64_t, UncompressedImage *&);
};
}



#endif
