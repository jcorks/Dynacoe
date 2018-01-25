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

#ifndef DIMAGE_H_INCLUDED
#define DIMAGE_H_INCLUDED


#include <Dynacoe/Color.h>
#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Modules/Assets.h>


#include <vector>
#include <string>

/* The standard Image class for Dynacoe */

namespace Dynacoe {
class Renderer;
/**
 * \brief Class for abstracting visual data blocks.
 *
 * All loaded image files are converted into an Image.
 * An Image is a list of frames, where each frame is
 * 2-Dimensional matrix of 4-component RGBA colors. Every layer
 * of an image has the same dimensions. A typical uses of multiple layers of an image
 * are tilesets and animations. In most cases, the pixel data for the image
 * is stored on the GPU.
 *
 * Often Image s with multiple frames are displayed as Images
 *
 */
class Image : public Asset{

  public:

    /// \brief A 2D matrix of pixels
    ///
    class Frame {
      public:

        /// \brief Creates an empty Frame.
        ///
        Frame();
        Frame(int textureObjectHandle);

        /// \brief Creates an empty Frame with a specified width/height
        ///
        Frame(uint32_t w, uint32_t h);
        
        /// \brief Creates a Frame from the given raw data.
        ///
        Frame(uint32_t w, uint32_t h, const std::vector<uint8_t> & data);

        /// \brief Creates a Frame as a copy of another.
        ///
        Frame(const Frame & other);

        Frame & operator=(const Frame & other);
        ~Frame();

        /// \brief Returns the raw pixel data of a frame.
        ///
        std::vector<uint8_t> GetData() const;

        /// \brief Updates a frame with the given pixel data. Note that the data 
        /// must match in dimensions the current Width/Height of the Frame.
        ///
        void SetData(const std::vector<uint8_t> & data);

        /// \brief Returns the width of the frame.
        ///
        uint32_t Width() const;

        /// \brief Returns the height of the frame.
        ///
        uint32_t Height() const;


        /// \brief Splits the frame into many sub-frames, each with the specified smallWidth and smallHeight.
        ///
        std::vector<Frame> Detilize(uint32_t smallWidth, uint32_t smallHeight) const;

        /// \brief Returns the object handle to be used with the Renderer.
        ///
        int GetHandle() const;
      private:
        int object;
    };

    ///\brief The frames of the Image.
    ///
    std::vector<Frame> frames;


    /// \brief Currently active frame. Changes over time.
    ///
    Frame & CurrentFrame();


    Image(const std::string &);
  private:
    uint32_t index;
};
};

#endif
