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

#ifndef H_DC_FRAMEBUFFER_INCLUDED
#define H_DC_FRAMEBUFFER_INCLUDED

#include <Dynacoe/Backends/Backend.h>
namespace Dynacoe {


/// \brief Contains a visual state.
///
/// The primary use of this is efficient communication between the
/// Renderer and the Display. The Renderer needs something to write to
/// when rendering and the Display needs something to read from when
/// updating. Instead of having them coupled with each other, the Framebuffer allows
/// for more flexibility of when each backend commits its action.
/// It also allows for storing and maintaining multiple targets
/// for rendering.
class Framebuffer : public Backend {
  public:

    /// \brief Types refer to the internal class by which visual information is
    /// passed to the Display.
    ///
    enum class Type {
        RGBA_PixelArray, ///< (uint8_t *) RGBA-ordered pixel array with no padding, matched to the dimensions given
        GLFBPacket,      ///< (GLRenderTarget **)  A pointer to a pointer to
                         /// a GLRenderTarget instance.
        Unknown,         ///< The framebuffer's data contents are unknown and should not be relied on.
    };


    /// \brief Resizes the framebuffer. Success is returned.
    ///
    bool Resize(int newW, int newH) {
        if (!OnResize(data, newW, newH))
            return false;

        w = newW;
        h = newH;
        return true;
    }

    /// \brief Returns the source data that reflects the framebuffer's data
    /// in the context of the implemented child. For example, on an OpenGL-variant
    /// backend, the handle is likely an OpenGL texture object id.
    ///
    void * GetHandle() {return data;}

    ///\brief Returns the width of the framebuffer.
    ///
    int Width() {return w;}

    /// \brief Returns the height of the framebuffer.
    ///
    int Height() {return h;}

    /// \brief Returns what type the handle refers to
    ///
    Type GetHandleType() {return type;}

    /// \brief returns a RGBF pixel reduction by setting the
    /// buffer given. if this isnt possible, false is returned.
    /// the buffer should be of size Width*Height*4. Note that on
    /// hardware-accelerated implementations, calling this could be very costly.
    /// Alpha color information is always 1.f
    virtual bool GetRawData(uint8_t *) = 0;

    /// \brief Sets whether to interpret the Framebuffer's data
    /// in a filtered way.
    ///
    /// It is not defined by Dynacoe as to exactly what this means, other than some sort of pixel
    /// interpolation to make the visual data smoother than otherwise.
    /// This is most applicable when using the Framebuffer where
    /// its natural size cannot be expressed (i.e. displaying the framebuffer
    /// visual on a system window of a different size). It should also be mentioned,
    /// that any module that utilizes the Framebuffer may choose to ignore the
    /// the filtered hint. As such, you should expect the filtered setting
    /// to be a purely cosmetic effect. The default is true.
    void SetFilteredHint(bool filter) {if (filtered!=filter) {filtered=filter; OnFilterChange(filter);}}

    /// \brief Returns whether to interpret the Framebuffer's data in a filtered
    /// way.
    ///
    /// See SetFilteredHint().
    bool GetFilteredHint() {return filtered;}

  protected:
    // can only create new framebuffers from a direct constructor
    Framebuffer(Type type_, int w_, int h_, void * data_ ) :
        type(type_),
        w(w_),
        h(w_),
        data(data_),
        filtered(true)
            {}

    // called on resize event to actually perform the resize action
    // on the data.
    virtual bool OnResize(void * source, int newW, int newH) = 0;

    // Called when SetFilteredHint  is called
    virtual void OnFilterChange(bool) = 0;
  private:
    int w;
    int h;
    void * data;
    Type type;
    bool filtered;
};
}

#endif
