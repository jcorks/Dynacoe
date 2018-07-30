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

#include <Dynacoe/Util/Transform.h>

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
class SpatialTransformUpdate;
class Spatial {
  public:
    Spatial();
    virtual ~Spatial();

    // Gets the transform for this spatial object alone.
    // Modifications to it affect the global transform calculations
    inline Transform & Node() {return *node;}
    
    // Gets the transform matrix that represets this Transform and its 
    // child parent transforms. If changes have been queued for children 
    // transforms or this one, those changes are applied.
    TransformMatrix & GetGlobalTransform();

    // Swaps the built in transform for the given transform 
    // pass nullptr to return the transform back to the default.
    void ReplaceTransform(Transform *);
    
    // Sets the given spatial object as a child of this one
    void SetAsParent(Spatial *);
    
    // invalidates this transfrom manually, causing itself and children to update 
    // upon next request for its transform.
    void Invalidate();

    void CheckUpdate();
    
    //Returns a 16-float array that signifies the internal transform array.
    //This is intended for quick renderer upload. This will always
    //reflect the global transform
    void UpdateModelTransforms(RenderBufferID id);
	
	
    
  protected:
    virtual void OnUpdateTransform(){}
    
  private:
    Spatial * parent;
    std::vector<Spatial*> children;
    Transform * transformOwned;
    TransformMatrix global;
    bool needsUpdate;
    SpatialTransformUpdate * transformUpdate;
    void SendUpdateSignal();
	Transform * node;
};
}



#endif
