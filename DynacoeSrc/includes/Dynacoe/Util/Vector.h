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

#ifndef H_Dynacoe_POINT_INCLUDED
#define H_Dynacoe_POINT_INCLUDED

/*	Dynacoe::Vector
 *
 *	A hybrid positional vector class
 *
 *  Johnathan Corkery, 2014
 */

#include <string>
#include <vector>

namespace Dynacoe {

/**
 *  \brief 3D and 2D positional vector.
 *
 *  A hybrid class suitable for both 2D and 3D
 *  positional and physical operations.
 *
 */
struct Vector {
    /// \brief Initializes x, y, and z to 0.
	///
	Vector();


	/// \brief Defines all coordinates.
    ///
	Vector(float x, float y, float z = 0.f);



    /// \brief Defines up to 3 coordinates. If fewer than 3 are given, the remaining
    /// components are left at 0
    Vector(const std::vector<float> &);

    /// \brief Attempts to retrieve coordinates from formatted text.
    ///
    /// THe format of the text is 2 to 3 floating point numbers separated by
    /// any counts of either whitespace or generally accepted separation characters:
    ///    ,.<>(){}[]
    ///
    /// The point here is to allow instantiation from natural mathematical notations
    /// which depend on the usage of the Vector. Examples of valid input:
    ///
    /// Vector("{1, 4, 5}");
    /// Vector("1 4 5");
    /// Vector("<1 4 5>");
    /// Vector("[1 4 5]");
    ///
    Vector(const std::string &);


    /// \name Positions
    ///
    ///\{
	float x;
	float y;
	float z;
    ///\}

    /// \brief Returns the length of the vector.
    ///
    /// This is equivalent to distance(Vector()).
	float Length() const;

	/// \brief Returns the distance of this vector to the other vector.
	///
	float Distance(const Vector &) const;


    /// \brief Divides all corresponding members and returns the outcome.
    ///
	Vector operator/(const Vector &) const;
    /// \brief Multiplies all corresponding members and returns the outcome.
    ///
	Vector operator*(const Vector &) const;
    /// \brief Adds all corresponding members and returns the outcome.
    ///
	Vector operator+(const Vector &) const;
    /// \brief Subtracts all corresponding members and returns the outcome.
    ///
	Vector operator-(const Vector &) const;


    /// \name Arithmetic operations.
    ///
    /// Vector supports the following arithmetic operations,
    /// For these, the input and operation are applied to
    /// x, y, and z.
    ///@{

        Vector operator/(float) const;
        Vector operator*(float) const;
        Vector operator+(float) const;
        Vector operator-(float) const;
	///@}


    bool operator==(const Dynacoe::Vector &) const;
    bool operator!=(const Dynacoe::Vector &) const;


    /// \name Arithmetic setting operations.
    ///
    /// These are equivalent to the Arithmetic operations,
    /// but *this is set to the result and returned.
    ///\{
        Vector & operator/=(const Vector &);
        Vector & operator*=(const Vector &);
        Vector & operator+=(const Vector &);
        Vector & operator-=(const Vector &);


        Vector & operator/=(float);
        Vector & operator*=(float);
        Vector & operator+=(float);
        Vector & operator-=(float);
	///\}

    /// \brief Negates all components.
	Vector operator-() const;

	/// \brief Returns a copy of the Vector with a magnitude of 1.
	///
	Vector Normalize() const;

	/// \brief returns the dot product of the two vectors.
	///
	/// The dot product is defined as
	/// (a.length() * b.length() * cos(\theta))
	/// where theta is the angle between the 2 vectors
	float Dot(const Vector &) const;

	/// \brief Returns the 2D cross product.
	///
	/// A cross product is computed as if the z components
    /// were equal to zero. This gives the magnitude, or Z of
    /// the cross product.
	float CrossFlat(const Vector &) const;


	/// \brief Returns the cross product
	Vector Cross(const Vector &) const;








    /// \name X axis rotation
    ///
    /// If a vector has length zero in these dimensions
    /// , the angle is always zero.
    /// The rotation is about the directional vector (0, 1, 0) and
    /// Positive rotation is towards the positive Z direction.
    ///\{

        /// \brief returns the YZ angle between the 2
        /// vectors in relation to the origin in degrees
        float RotationXDiff(const Vector &) const;


        /// \brief returns the angle between the 2
        /// vectors in relation to (*this) in degrees
        float RotationXDiffRelative(const Vector &) const;


        /// \brief returns the angle of the vector in degrees
        ///

        float RotationX() const;

    ///\}







    /// \name Y axis rotation
    ///
    /// If a vector has length zero in these dimensions
    /// , the angle is always zero.
    /// The rotation is about the directional vector (0, 0, 1)
    /// Positive rotation is towards the positive X direction.
    ///\{

        /// \brief Returns the zx angle between the 2
        /// vectors in relation to the origin in degrees
        float RotationYDiff(const Vector &) const;


        /// \brief returns the angle between the 2
        /// vectors in relation to (*this) in degrees
        float RotationYDiffRelative(const Vector &) const;


        /// \brief returns the angle of the vector in degrees.
        ///
        float RotationY() const;
    ///\}





	/// \name Z rotation axis
	///
	/// These deals with angles between the x and y axes.
    /// XY is most useful for 2D angles and transformations.
    /// If a vector has length zero in these dimensions
    /// , the angle is always zero.
    /// The rotation is about the directional vector (1, 0, 0).
    /// Positive rotation is towards the positive Y direction.
	///\{


        /// \brief returns the XY angle between the 2
        /// vectors in relation to the origin in degrees
        float RotationZDiff(const Vector &) const;


        /// \brief returns the angle between the 2
        /// vectors in relation to (*this) in degrees
        float RotationZDiffRelative(const Vector &) const;


        /// \brief returns the angle of the vector in degrees
        ///
        float RotationZ() const;
    ///\}





	/// \brief Returns the result of rotating the vector
	/// by the specified degrees about the center
    ///\{
	Vector RotateZ(float);
	Vector RotateY(float);
	Vector RotateX(float);

	Vector RotateZFrom(const Vector &, float);
	Vector RotateYFrom(const Vector &, float);
	Vector RotateXFrom(const Vector &, float);
    ///\}

    /// \brief Returns a version of the vector with the
    /// fractional values of each component dropped.
	Vector Floor() const;

    /// \brief Shorthand for (*this) = normalize().
    ///
	Vector & SetToNormalize();

	/// \brief Shorthand for (*this) = floor().
	Vector & SetToFloor();


    /// \name Partial setting
    ///
	/// Sets x y and z to newly appointed values.
	///
    ///\{
        Vector & operator()(float, float);
        Vector & operator()(float, float, float);
        Vector & operator()(const Vector &);
    ///\}



};

};

#endif
