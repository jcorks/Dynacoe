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

#ifndef H_DC_Transform3D_INCLUDED
#define H_DC_Transform3D_INCLUDED

#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Util/TransformMatrix.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>

namespace Dynacoe{

/**
 *
 * \brief A hierarchical transform object.
 *
 * Allows for chaining transformation information including
 * position, rotation, and scaling. Because Transforms are hierarchical,
 * each has a local transform and a global transform.
 *
 *
 */
class Transform {
  public:


    Transform();
    ~Transform();

    
    
    /// \brief Gets a read-only, quick reference to the rotation component.
    ///
    const Vector & GetRotation() const;

    /// \brief Gets a read-only, quick reference to the position component.
    ///
    const Vector & GetPosition() const;

    /// \brief Gets a read-only, quick reference to the scale component.
    ///
    const Vector & GetScale() const;

    /// \brief Returns a writable reference to the rotation component. 
    /// Once called, flags the node to recalculate its transforms
    /// on the following frame.
    Vector & Rotation();

    /// \brief Returns a writable reference to the position component. 
    /// Once called, flags the node to recalculate its transforms
    /// on the following frame.
    Vector & Position();

    /// \brief Returns a writable reference to the scale component. 
    /// Once called, flags the node to recalculate its transforms
    /// on the following frame.
    Vector & Scale();


    /// \brief Sets whether to reverse the node's interpretation of 
    /// of translation for transform calculation. Rarely useful.
    ///
    void SetReverseTranslation(bool);
    




    class OnTransformUpdate {
      public:
        virtual ~OnTransformUpdate(){}

        virtual void operator()(Transform * source) = 0;
        
        // If returns true, the Transform that contains thsi 
        // functor also owns the rights to free it up.
        // Usually this happens when the Transform is distroyed.
        virtual bool OwnedByTransform() = 0;
    };


    
    /// \brief Gets the transform matrix representative
    /// of this Transform. At this time, any requests to modify 
    /// the transform are processed. If any such request was made, 
    /// the transform is updated and any OnTransformUpdate functors 
    /// are called.
    const TransformMatrix & GetMatrix();

    /// \brief Adds a transform update.
    void AddTransformCallback(OnTransformUpdate *);

    /// \brief removes a transform callback
    void RemoveTransformCallback(OnTransformUpdate *);

    /// \brief Returns whether an update to this transform is 
    /// pending.
    bool NeedsUpdate() const;
  
  private:

    
    void ComputeLocal();

    TransformMatrix localTransform;

    bool reverse;
    bool needsUpdate;
    Vector position;
    Vector rotation;
    Vector scale;
    
    std::vector<OnTransformUpdate*> callbacks;
};
}



#endif
