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


#ifndef H_DC_SHAPE2D_INCLUDED
#define H_DC_SHAPE2D_INCLUDED

#include <Dynacoe/Components/Render2D.h>
#include <Dynacoe/Image.h>
#include <Dynacoe/Color.h>
namespace Dynacoe {

/** \brief An aspect that can express basic 2D objects, such as images and shapes.
 *
 *
 */
class Shape2D : public Render2D {
  public:
    Shape2D();

    /** \name  Formation
     *
     *   Forming aspects denotes what visual the aspect should assume.
     */

    Color color;

    ///\{
    /// \brief Forms the aspect into a rectangle.
    ///
    /// The top-left corner of the rectangle is the position
    /// of the Aspect from GetPosition(). Width and height are expressed in pixels.
    /// @param width The width of the rectangle in pixels.
    /// @param height The height of the rectangle in pixels.
    void FormRectangle  (float width, float height);

    /// \brief Forms the aspect into the shape and visual of an image.
    ///
    /// The scale is a ratio of the source image dimensions.
    /// If the Image has multiple frames, the image is drawn as
    /// an animation.
    /// @param image The image to display as the aspect. If image does not refer to
    /// a valid Image, no action is taken. If the Image has multiple frames, the subsequent frame
    /// is drawn upon next Graphics::Draw of the aspect.
    void FormImage      (AssetID image, float forcedWidth = -1.f, float forcedHeight = -1.f);




    /// \brief Forms the aspect into a frame of an image.
    ///
    /// Behaves like FormImage, but a specific frame index is
    /// specified to draw rather than the image as an animation.
    /// @param image The image to form as the aspect. If image does not refer to
    /// a valid Image, no action is taken
    /// @param frame The frame of the image to use, starting at 0. If the
    /// frame is negative, the aspect is drawn as if FormImage() had been called.
    void FormImageFrame (AssetID image, int frame, float forcedWidth = -1.f, float forcedHeight = -1.f);

    /// \brief Forms the aspect into an estimated circle made of triangles.
    ///
    /// @param radius The radius of the circle. Measured in pixels.
    /// @param numIterations Number of sides that the circle
    /// should have. The more iterations, the smoother the
    /// circle will become, but the more resource-intensive
    /// the Aspect becomes as well.
    void FormCircle     (float radius, int numIterations);


    /// \brief Forms the aspect into a collection of triangle primitives.
    ///
    /// The points are interpreted as pixels and are expected to be in groups of 3.
    /// If a non-multiple-of-three number of points is given,
    /// the remainder points are ignored.
    /// @param pts The points of the triangles.
    void FormTriangles(std::vector<Dynacoe::Vector> & pts);

    /// \brief Forms a collection of line segments.
    ///
    /// Every two points form a line segment. If an odd number of points are given, the last point is ignored.
    /// @param pts The points of the lines.
    void FormLines      (const std::vector<Dynacoe::Vector> & pts);

    ///\}

    void OnDraw();
    std::string GetInfo();

  private:
    AssetID id;
    int idFrame;
    Color realColor;
    float forcedWidth;
    float forcedHeight;

    void SetFrameActive(const Image::Frame & f);
};
}



#endif
