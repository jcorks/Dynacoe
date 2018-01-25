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

#ifndef DC_H_TRANSFORM_MATRIX_INCLUDED
#define DC_H_TRANSFORM_MATRIX_INCLUDED

#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Util/Vector.h>


namespace Dynacoe {

/// \brief A transform representated as a row-major matrix.  
///
class TransformMatrix {
  public:
    TransformMatrix(const TransformMatrix &);
    TransformMatrix & operator=(const TransformMatrix &);
    TransformMatrix(float *);
    TransformMatrix(); // identity, row major

    /// \brief Transforms the given point and returns its result;
    Vector Transform(const Vector & p) const;

    /// \brief Returns a string containing info on the transform.
    ///
    Dynacoe::Chain Print() const;

    /// \brief Transposes the matrix.
    ///
    void Transpose();

    /// \brief Inverts the matrix.
    ///
    void Inverse();

    /// \brief Reverse the majority of the matrix.
    ///
    void ReverseMajority();

    /// \brief Returns the internal representation of the TransformMatrix.
    ///
    float * GetData();



    /// \brief Multiplies 2 matrices.
    ///
    TransformMatrix operator*(const TransformMatrix &) const;
    
    /// \brief Rotates the matrix about the Euler angles psi, theta, and phi.
    ///
    void RotateByAngles(float x, float y, float z);

    /// \brief Rotates about a vector.
    ///
    void RotateByVector(const Dynacoe::Vector &);

    /// \brief Expresses a translation by x, y, and z
    ///
    void Translate(float x, float y, float z);

    /// \brief Expresses a scaling in the x, y, and z directions.
    ///
    void Scale(float x, float y, float z);


    /// \brief Sets the matrix to the identity matrix, removing all transformations.
    ///
    void SetToIdentity();
    
  private:
    float data[16];
};
}

#endif  
