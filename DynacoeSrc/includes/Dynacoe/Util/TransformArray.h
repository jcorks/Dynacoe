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

#ifndef DC_H_TRANSFORM_ARRAY_INCLUDED
#define DC_H_TRANSFORM_ARRAY_INCLUDED

#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Util/TransformMatrix.h>
#include <Dynacoe/Util/Chain.h>


namespace Dynacoe {


/// \brief A quicker and tinier alternative to TransformMatrix, but
/// must eventually be converted to a matrix for the renderer to process it.
///
class TransformArray {
  public:
    TransformArray();

    /// \brief Returns a reduction of the TransformArray into an equivalent TransformMatrix.
    ///
    TransformMatrix Compose(); 

    /// \brief Linearly interpolates between two transforms.
    ///
    TransformArray Lerp(const TransformArray &, float amt);

    /// \brief Sets the transform to a default state.
    ///
    void Reset();

    /// \brief Rotates the TransformArray about an axis.
    ///
    /// @param axisVector Defines the rotation axisVector. The line created by Vector(0, 0, 0) and the point axisVector creates the axis to rotate about. 
    /// @param amountDegrees The rotation amount to perform.
    void RotateByAxis(const Dynacoe::Vector & axisVector,  float amountDegrees);

    /// \brief Express a translation to apply to the transform in the x, y, and z directions.
    ///
    void Translate(float x, float y, float z);

    /// \brief Express a scaling transformation to apply to the transform in the x, y, and z directions.
    ///    
    void Scale(float x, float y, float z);  


    /// \brief Returns the total translation.
    ///
    Dynacoe::Vector GetTranslation();

    /// \brief Returns the scale.
    ///
    Dynacoe::Vector GetScale();
    /// \brief Returns a reduction of the quaternion into euler angles: psi, theta, and phi respectively.
    ///
    Dynacoe::Vector GetRotation();


    /// \brief  Returns a string containing infor on the state of the transform.
    ///
    Dynacoe::Chain Print();

  private:
    float data[3+4+3];
};
}

#endif
