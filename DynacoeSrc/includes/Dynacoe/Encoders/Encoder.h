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

#ifndef DC_H_ENCODER_INCLUDED
#define DC_H_ENCODER_INCLUDED

#include <Dynacoe/Modules/Assets.h>


/* Reads file contents and outputs an Asset of some kind */

namespace Dynacoe {
class Encoder {
  public:
    
    // returns whether or not the encoding was successful
    virtual bool operator()(Asset * asset, const std::string & str, const std::string & path) {return true;};



    Encoder(Assets::Type type_, const std::string & extension_) 
        : type(type_), ext(extension_){};

    Assets::Type GetType() const {return type;}
    std::string GetExtension() const {return ext;}

  private:
    Assets::Type type;
    std::string ext;
    

};
}


#endif
