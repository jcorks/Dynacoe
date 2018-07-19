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

#ifndef DCOLOR_H_INCLUDED
#define DCOLOR_H_INCLUDED

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>



namespace Dynacoe {

/// \brief  A standard object representing an RGBA color
///
class Color {
  public:
      
      
    /// \brief A component of Color. Is from 0.f to 1.f but is stored 
    /// as a byte. When used, Component s are implicitly casted to floats
    ///
    class Component {
      public:
        Component(){}
        Component(const double & f) {data = f*255;}
        Component(const float & f) {data = f*255;}
        Component(const uint8_t & f) {data = f;}
        Component(const int & f) {data = f;}
        Component(const unsigned int & f) {data = f;}

        Component & operator+=(const Component & other) {data += other.data; return *this;}
        Component & operator-=(const Component & other) {data -= other.data; return *this;}
        Component & operator*=(const Component & other) {*this = (float)*this * (float)other; return *this;}
        Component & operator/=(const Component & other) {*this = (float)*this / (float)other; return *this;}


        operator float() const {
            return data/255.f;
        }

        /// \brief Returns the byte-value form of the component
        ///
        uint8_t Byte() const {
            return data;
        }
        
        bool operator==(const Component & other) const {return data == other.data;}
        bool operator!=(const Component & other) const {return data != other.data;}

      private:
        uint8_t data;
    };


    Color();
    Color(uint32_t);
    Color(int red, int green, int blue, int alpha = 255);
    Color(float redf, float greenf, float bluef, float alphaf = 1.f);
    Color(const std::vector<Component> & c);

    /// \brief Sets the color to the color referred to by string.
    ///
    /// @param name In the case that the string starts with a '#', the
    /// remainder of the string will be read as a hexidecimal
    /// integer referring to the values of the hex string. For example
    /// Color("#FF00FFFF") is equivalent to Color(255, 0, 255, 255)
    /// It is also valid to specify only 3 components:
    /// Color("#FF00FF") is equivalent to Color(255, 0, 255, 255). If the string does not
    /// begin with a '#', the string is impied to represent a string that has been
    /// mapped with Define().
    ///
    /// Strings passed are not case-sensitive.
    Color(const std::string & name );
    Color(const char * sname);
    Color(const Color &);

    /// \brief Returns an unsigned, 32-bit integer where each byte refers to
    /// a color component in RGBA form.
    ///
    /// The first byte is the red component, then green, blue, and alpha.
    uint32_t Int() const;


    Color & operator= (const Color &);

    /// \brief Sets the color.
    ///
    Color & Define(int red, int green, int blue, int alpha);

    /// \brief Sets the color.
    ///
    Color & Define(uint32_t c);

    Color operator+ (const Color &) const;
    Color operator- (const Color &) const;
    Color operator* (const Color &) const;
    Color operator/ (float) const;
    Color operator* (float) const;
    Color & operator+=(const Color &);
    Color & operator-=(const Color &);
    bool  operator==(const Color &) const;
    bool  operator!=(const Color &) const;


    ///  \brief The red amount.
    ///
    Component r;
    ///  \brief The green amount.
    ///
    Component g;
    ///  \brief The green amount.
    ///
    Component b;
    ///  \brief The blue amount.
    ///
    Component a;


    /// \brief Defines a new color to be recognized by name.
    ///
    /// String names are not case-sensitive.
    static void Define(const std::string &, const Color &);


    bool operator<(const Color & b) const {
        return Int() < b.Int();
    }

    /// \brief Returns the string form of the Color.
    ///
    std::string ToString() const;

};

};

#endif
