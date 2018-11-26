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


#ifndef H_DC_ASSET_ID_INCLUDED
#define H_DC_ASSET_ID_INCLUDED

namespace Dynacoe {

/// \brief Assets are referred to by an AssetID. The AssetID uniquely refers to an Asset
/// stored within memory.
///
class AssetID {

  public:
    AssetID() : type(NO_TYPE), handle(-1) {}
    AssetID(int h, int t) {handle = h; type = t;};

    /// \brief Returns whether this AssetID refers to an actual
    /// Asset.
    bool Valid() const {return type != NO_TYPE;}

    /// \brief Returns whether 2 AssetID s are equivalent.
    ///
    bool operator==(const AssetID & a) {
        if (!Valid() && !a.Valid()) return true;
        return Valid() && a.Valid() && (handle == a.handle) && type == a.type;
    }

    enum AssetType {
        NO_TYPE,
        IMAGE,
        FONT,
        SOUND,
        SEQUENCE,
        MODEL,
        PARTICLE,
        RAWDATA,
        ACTOR,
        NUMTYPES
    };


    int GetHandle() const { return handle;}
    int GetType() const { return type; }


    

  private:



    int handle;
    int type;
};

}

#endif
