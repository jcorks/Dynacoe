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

#ifndef DC_H_CAMERA_INCLUDED
#define DC_H_CAMERA_INCLUDED

#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <Dynacoe/Util/TransformMatrix.h>
#include <Dynacoe/Entity.h>
// The Camera transformations apply to the 3D context.




namespace Dynacoe {

/**
 *  \brief The viewport object
 *
 * The Camera is the symbolic viewing port for Dynacoe.
 * It acts as the bridge between the rendered scene and the viewer.
 * Any number of Camera s can be maintained, but only one may be used at a time.
 * See Graphics::SetCamera3D(), Graphics::SetCamera2D, and Graphics::SetRenderCamera()
 *
 */
class Camera : public Entity {
  public:
    Camera();
    Camera(Node*);

    /// \brief The type of Camera
    enum class Type {
        Perspective3D, ///< Indicates use for 3D perspective viewing 
        Orthographic2D ///< brief Indicates use for 2D orthographics viewing
    };

    void BindTransformBuffers(RenderBufferID mview, RenderBufferID proj);


    /// \brief Sets the Type of Camera.
    ///
    void SetType(Type);

    /// \brief  Forces clearing of the results of drawing stored within the Camera.
    ///
    void Refresh();

    /// \brief Sets whether to call Refresh() automatically
    ///
    bool autoRefresh;



    /// \brief Sets the Camera's target.
    ///
    /// When called, the Camera will orient itself so that it
    /// "looks" at the given point in 3D space.
    /// @param pos The position to face.
    void SetTarget(const Dynacoe::Vector & pos);



    /// \brief Converts a point representing a pixel on a display to
    /// a point in 3D space.
    ///
    /// @param point2D The point to convert.
    /// @param disance The distance to project the Vector to. In the case of a 3D perspective camera: Since the source is 2D, the
    /// Z value is ignored. However, when projecting, since there is no source Z value,
    /// The actual depth from the Camera's point of view is undefined when projecting.
    /// Thus a distance from the viewpoint is needed.
    Dynacoe::Vector TransformScreenToWorld(const Dynacoe::Vector & point, float distance);

    /// \brief Converts a point in transformed space to a pixel position on the display.
    ///
    /// @param point The source point to project onto the Display.
    Dynacoe::Vector TransformWorldToScreen(const Dynacoe::Vector & point);


    /// \brief Convenience function to set the rendering resultion
    ///
    /// @param w Width in pixels
    /// @param h Height in pixels
    void SetRenderResolution(int w, int h);


    /// \brief Returns the width of the Camera's framebuffer.
    /// 
    int Width();

    /// \brief Returns the height of the Camera's framebuffer.
    /// 
    int Height();



    /// \brief Returns the viewing matrix of the Camera.
    ///
    TransformMatrix GetViewTransform();

    /// \brief Returns the projection matrix of the Camera.
    ///
    TransformMatrix GetProjectionTransform();

    /// \brief Returns the framebuffer for the camera.
    ///
    Framebuffer * GetFramebuffer();

    /// \brief Returns a copy of the last rendered visual stored
    /// to this camera.
    ///
    AssetID CopyDisplay();


    void OnStep();
  private:
    static Camera * activeRef;

    void UpdateView();
    void UpdateProjection();
    void Activate(Renderer *);
    void Deactivate();

    int lastW, lastH;

    Type type;
    TransformMatrix projectionMatrix;
    Framebuffer * fb;


    RenderBufferID modelView;
    RenderBufferID projection;
    void Initialize();
};
}

#endif
