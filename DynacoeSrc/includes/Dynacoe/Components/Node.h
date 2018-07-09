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

#ifndef H_DC_Node3D_INCLUDED
#define H_DC_Node3D_INCLUDED

#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Util/TransformMatrix.h>
#include <Dynacoe/Component.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>

namespace Dynacoe{

/**
 *
 * \brief A hierarchical transform object.
 *
 * Allows for chaining transformation information including
 * position, rotation, and scaling. Because Nodes are hierarchical,
 * each has a local transform and a global transform.
 *
 *
 */
class Node : public Component {
  public:


    Node();
    ~Node();

    
    
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
    


    /// \brief Gets the global transform of the node based on its 
    /// nodal ancestry.
    const TransformMatrix & GetGlobalTransform();




    std::string GetInfo();
    void OnStep();



    //Returns a 16-float array that signifies the internal transform array.
    //This is intended for quick renderer upload. This will always
    //reflect the global transform
    void UpdateModelTransforms(RenderBufferID id);

    void SetManualParent(Node *);

    void OnAttach();


  protected:


    // manually calls to update the transform.
    void updateTransform();

    // Runs after updateTransform is called
    virtual void OnTransformUpdate() {};

    // Runs before the aspect is drawn
    virtual void OnDraw() {}

  private:


    void UpdateParentReference();
    void ComputeLocal();

    TransformMatrix localTransform;
    TransformMatrix normalTransform;
    TransformMatrix globalTransform;

    //Transform transformLastState;

    LookupID id;
    LookupID parent;

    bool localTransformValid;
    bool parentTransformValid;
    bool reverse;

    Vector position;
    Vector rotation;
    Vector scale;
    

    Node * overrideParent;
    Node * parentNode;

    std::set<Node*> * manualChildren;

};
}



#endif
