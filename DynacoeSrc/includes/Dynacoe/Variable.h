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

#ifndef H_DC_VARIABLE_INLCUDED
#define H_DC_VARIABLE_INLCUDED

#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Util/Vector.h>
#include <string>

namespace Dynacoe {


/// \brief Abstraction for a variable in Dynacoe. 
/// 
/// The variable that it 
/// represents MUST have at least as long of a lifetime as this
/// variable's, as Variable has no inkling as to whether the variable 
/// it represents has been free'd/ deleted.
///
class Variable {
  public:

    /// \brief The type of the variable represented.
    ///
    enum class Type {
        Boolean,    ///< bool
        Int,        ///< int
        Float,      ///< float
        Vector,      ///< Dynacoe::Vector
        String,     ///< String 
        Address,    ///< pointer
        Unknown     ///< Error type. Set only when the variable is built with the default constructor and never Set.
    };


    /// \name Construction 
    ///
    ///\{
    Variable();
    Variable(const std::string & name, bool        & v);
    Variable(const std::string & name, int         & v);
    Variable(const std::string & name, float       & v);
    Variable(const std::string & name, std::string & v);
    Variable(const std::string & name, void *        v);
    Variable(const std::string & name, Vector       & v);
    ///\}

    friend bool operator<(const Variable &, const Variable & );

    /// \brief reutrns the name of the variable.
    ///
    std::string GetName();

    /// \brief Returns the type of the variable.
    ///
    Type GetType();
    
    /// \brief Returns a text reduction of the  variable.
    ///
    std::string ToString();

    /// \brief Attempts to set the value of the Variable to 
    /// the value denoted by the string. The function will guess 
    /// the type and value represented. If a value and type cannot be gleaned from the string 
    /// or if the type cannot be converted to this variables type, no action is taken. 
    ///    
    void Set(const std::string &);


  private:
    
    std::string name;
    void * data;
    Type type;
    

};

bool operator<(const Variable &, const Variable &);
}


#endif
