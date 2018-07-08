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

#if ( defined DC_BACKENDS_SHADERGL_X11 || defined DC_BACKENDS_SHADERGL_WIN32 )

#include <Dynacoe/Backends/Renderer/ShaderGL/TextureManager.h>
#include <Dynacoe/Backends/Renderer/StaticState.h>
#include <Dynacoe/Backends/Renderer/ShaderGL/RenderBuffer.h>
#include <cstring>

using namespace Dynacoe;

const int DBUFFER_DEFAULT_INIT_LENGTH         = 32;


const int   NUM_FLOATS_PER_TEX   = 4;
const GLenum DBUFFER_GUT_TEX_ACTIVE = GL_TEXTURE0;
const GLenum MAIN_DISPLAY_TEX_ACTIVE = GL_TEXTURE1;
const GLenum USER_TEX_ACTIVE = GL_TEXTURE2;

const int DBUFFER_ERROR_INVALID_DIMENSIONS    =   -1;
const int DBUFFER_ERROR_NO_MORE_TEXTURES      =   -2;
const int DBUFFER_ERROR_NO_MORE_MEMORY        =   -3;
const int DBUFFER_DEFAULT_INIT_DEPTH          = 64;
const int DBUFFER_DEFAULT_TESSATE_LENGTH      = 64;

// lower the ratio, less the chance of distortion, but waste memory is higher
// A tolerance value of 0 specifies not to restrict.
const float DBUFFER_TEX_TOLERANCE_RATIO       = 0.f;
const float DBUFFER_DEFAULT_RESIZE_FACTOR     = 1.2f;
// tex coord buffer
const int DBUFFER_DEFAULT_INIT_TCB_SIZE       = 64;




// The data layout algorithm for the GUT is currently as follows:

/*

   A greedy approach to GUT packing.
   Johnathan Corkery, 2014

    ALGORITHM


   +----+-----+-----+--------------------------+
   |    |     |     |                          |
   |    |     |     |<- subimages              |
   +----+     |     |                          |
   |    |     |     |                          |
   |    +-----|     |                          | <-- GUT
   +----------+-----+--------------------------+

   -The GUT grows in width with incoming textures.
    Any new textures are placed in the top-left most space
    that is unused.
   -The height of the GUT is equivalent to the height of the
    texture with the largest height.
   -If the bounds of either the width or height are greater than
    the current GUTs bounds, the GUT will resize if able.


   -If the texture length implementation limit is reached width-wise,
    The implementation shifts:

   +----+-----+-----+--------------------------+
   |    |     |     |                          |
   |    |     |     |                          |
   +----+     |     |                          |
   |    |     |     +--------------------------+
   |    +-----|     |                          |
   #-----+    +-----+                          | <-- old height ceiling
   |     |                                     |
   |     | <-- new subimage                    |
   |     |                                     |
   +-----+-------------------------------------+ <-- new height ceiling (also height of GUT)
                                               ^
                                               |
                                               maximum texture width allowable by renderer




   After the implementation shifts, no more width extensions
   are possible. Instead the algorithm is shifted in the y
   direction by the current height ceiling. The new "top-left"
   becomes (0, current GUT height ceiling) marked by the hash symbol.

   -The height of the GUT then becomes the height of the origin +
    the highest image of that row.


   -Once in this stage of the algorithm, resizes of the GUT
    becomes much more rare, but much more costly in both
    time and space. Consider this when adding images.



    -Once the GUT can no longer resize in the y direction,
     THe algorithm will continue to grow width-wise with the width limit
     is reached.

    -If neither width nor height can resize, the GUT is considered complete.
    Once complete, the texture cache will no longer grow.

    ERRORS

    -If a texture requires dimensional space that
    cannot be accomadated, but could if the GUT contained fewer images already
    it returns a NO_MORE_TEXTURES error.

    -If a texture requires dimensional space that cannot be
    accomadated otherwise, it should return an INVALID_TEXTURE error

    -If a texture requires physical memory space that cannot
     be accomadated, a NO_MORE_MEMORY error is returned.


     PATHOLOGICAL CASES

    -The primary worry is amount of wasted space that is generated with this
     algorithm.
    -The most wasted space in general would be a sample of mostly short height-wise iages and
     very few tall height-wise images, as the GUT would not fill the space that
     the short images waste.
    -The general best case scenario is a sample of square images of the same size.
     Thus, in general, the most waste will be created with rectangles of varrying, obtuse
     dimensions and dimension ratios.


    ALTERNATIVE IMPLEMENTATIONS

    -Using a 2D texture is the most straightforward way to implement this; however,
     you are bound by a relatively small number of images due to the hardware
     texture size limit. A work-around would be to use a 3D image as the base.

     The algorithm is nearly identical with a few significant differences:
     Use the 3D image as a 2D image array. Once the width has been expended to the
     renderers' allowable limit, instead of moving the origin, proceed to the next z-layer
     and repeat the algorithm. This would require keeping track of the z-layer to fully
     define a texture along with the texture coordinates, but the bounds on
     memory are much higher and resizes could potentially be less resource-intensive.
     With the 2D texture as a base, it is unlikely that one will be able to use
     anywhere close to the amount of memory available on the GPU, but it is slightly
     faster per call, as less information is needed to fully define a texture.

   +----+-----+-----+------------+-------------+
   |    |     |     |            |             |
   |    |     |     |            |             | <-- z-layer slice 0
   +----+     |     +------------+             |
   |    |     |     |            |             |
   |    +-----|     |            +-------------+
   +----------+-----+--------------------------+

   +----+--+--+-----+--------------------------+
   |    |  |        |                          |
   |    |  |        |                          | < z-layer slice 1
   +----+  +--------+                          |
   |    |  |                                   |
   |    +--+                                   |
   +-------------------------------------------+

    MISC.

   -The more square the textures, the more efficiently space is used.
   -There are much better placement algorithms, but this requires little computation
    while not wasting too much space. Pathologicals are neither
    common nor too detrimental, unless on much older machines,
    which wouldn't support a lot of this functionality without llvm /
    gallium / or whatever slow magic is out there nowadays.
    */



class HugeTexture  {
  public:
    HugeTexture() {
        glGenTextures(1, &glID);
        glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);
        w = DBUFFER_DEFAULT_INIT_LENGTH;
        h = DBUFFER_DEFAULT_INIT_LENGTH;
        glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);



        glBindTexture(GL_TEXTURE_2D, glID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     w, h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxLength);

        glActiveTexture(USER_TEX_ACTIVE);
    }

    ~HugeTexture() {
        glDeleteTextures(1, &glID);
    }


    // Resize the main space to reqW x reqH
    void Resize(GLint reqW, GLint reqH) {
        glBindTexture(GL_TEXTURE_2D, glID);
        int newW = w, newH = h;
        // width-wise resize;
        if (reqW < maxLength) {
            while(reqW > newW) {
                newW *= DBUFFER_DEFAULT_RESIZE_FACTOR;
                //cout << "Requested new size(W) of " << newW << "x" << newH << endl;
            }
        }


        if (reqH < maxLength) {
            while(reqH > newH) {
                newH *= DBUFFER_DEFAULT_RESIZE_FACTOR;
            }
            //cout << "Requested new size(h) of " << newW << "x" << newH << endl;

        }

        if (newW > maxLength) newW = maxLength;
        if (newH > maxLength) newH = maxLength;


        // lets copy the old data
        GLubyte * copy = new GLubyte[w * h * 4];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, copy);

        // create new tex and emplace as a sub image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newW, newH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, copy);



        w = newW;
        h = newH;


        //cout << "[DBUFFER] Resized GUT (" << newW << "x" << newH << ") (" << getTexCacheSize() << "MB)\n";

        delete[] copy;
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Replaces a portion of the large texture
    void Emplace(int x, int y, int tw, int th, uint8_t * data) {

        glBindTexture(GL_TEXTURE_2D, glID);
        if (data != NULL)
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else {
            //data = (uint8_t*)malloc(tw*th*4);
            //printf("%d, %d\n", tw, th);
            //memset(data, 255, tw*th*4);
            //glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, data);
            //free(data);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Reads a portion of the large texture.
    // TODO: make not the slowest thing in the world ever
    // TODO: needs fixing
    void Read(int x, int y, int tw, int th, uint8_t * data) {
        //// UHHHHHH
        glBindTexture(GL_TEXTURE_2D, glID);
        uint8_t * srcData = new uint8_t[w*h*4];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, srcData);
        for(int i = 0; i < th; ++i) {
            memcpy(data+i*tw*4, srcData+(4*(x + (y+i)*w)), tw*4);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        delete[] srcData;
    }

    GLint Width() const {return w;}
    GLint Height()const {return h;}
    GLint MaxLength(){return maxLength;}

    GLuint ID() {return glID;}



  private:

    GLuint glID;
    GLint maxLength;
    GLint w;
    GLint h;
};








TextureManager::TextureManager() {



    texImageBounds = new GLint[NUM_FLOATS_PER_TEX * DBUFFER_DEFAULT_INIT_TCB_SIZE];
    numTexPhys = DBUFFER_DEFAULT_INIT_TCB_SIZE;
    numTexUsed = 0;
    cursorX = 0;
    cursorY = 0;
    cursorHeight = 0;
    master = new HugeTexture();

}

int TextureManager::NewTexture(int w, int h, GLubyte * data) {
    return NewTexture(w, h, data, getNewTex());
}
int TextureManager::NewTexture(int w, int h, GLubyte * data, int newTexID) {



    if (w > master->MaxLength() ||
        h > master->MaxLength() ||
        w < 0 || h < 0) return DBUFFER_ERROR_INVALID_DIMENSIONS;


    // Move origin if width exeeded. THat is, we dont move the cursorY
    // until the width is fully expanded.
    if (master->Width() == master->MaxLength()) {
        if (w +  cursorX >= master->MaxLength()) { // the resize request likely wont be able to get the image width-wise


            //std::cout << "[DBUFFER]: Reached max resizeble width, but going to increase height" << std::endl;

            // go to next row;
            cursorY += cursorHeight;
            cursorX  = 0;


        }
        GarbageCollectBang();
    }

    // resize if needed and can.
    if (w + cursorX > master->Width() ||
        h + cursorY > master->Height()) {
        master->Resize(w + cursorX, h + cursorY);
    }




    // if this check fails, resizing was not fully effective
    // and the texture was not able to be added
    if ((w + cursorX > master->Width() ||
         h + cursorY > master->Height())) {
        if (garbageBag.empty()) {
            std::cout << "Couldn't fit image" << std::endl;
            return DBUFFER_ERROR_NO_MORE_TEXTURES;
        } else { // last resort: try to free some space for the incoming texture and try again.
            return NewTexture(w, h, data, getNewTex());
        }
    }


    // Place the texture as a sub texture of the GUT
    master->Emplace(cursorX, cursorY, w, h, data);

    // update row ceiling (AKA record height)
    if (cursorHeight < cursorY + h) cursorHeight = h;






    GLint * newCoordSet = &texImageBounds[NUM_FLOATS_PER_TEX * newTexID];
    newCoordSet[0] = cursorX;
    newCoordSet[1] = cursorY;
    newCoordSet[2] = w;
    newCoordSet[3] = h;




    /*
    newCoordSet[0] = master.currentW;
    newCoordSet[1] = master->Height();
    newCoordSet[2] = (master->Width()+ w);
    newCoordSet[3] = master->Height();
    newCoordSet[4] = (master->Width()+ w);
    newCoordSet[5] = (master->Height() + h);
    newCoordSet[6] = master->Width();
    newCoordSet[7] = (master->Height() + h);
    */


    // return the index to those texture coordinates.
    // the tex coords are what the index symbolize, the
    // coords will define the boundaries for the image from the GUT


    cursorX += w;
    glBindTexture(GL_TEXTURE_2D, 0);




    return newTexID;
}






void TextureManager::DeleteTexture(int tex) {
    if (tex >= 0 && tex < numTexUsed) {
        garbageBag.insert(tex);
        inactiveTex.insert(tex);
    }
}


void TextureManager::UpdateTexture(int tex, GLubyte * data) {
    int SCx = texImageBounds[NUM_FLOATS_PER_TEX * tex];
    int SCy = texImageBounds[NUM_FLOATS_PER_TEX * tex + 1];
    int SCw = texImageBounds[NUM_FLOATS_PER_TEX * tex + 2];
    int SCh = texImageBounds[NUM_FLOATS_PER_TEX * tex + 3];


    //glBindTexture(GL_TEXTURE_2D, master->ID());
    //glTexSubImage2D(GL_TEXTURE_2D, 0, SCx, SCy, SCw, SCh, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //glBindTexture(GL_TEXTURE_2D, 0);
    master->Emplace(SCx, SCy, SCw, SCh, data);
}



void TextureManager::SetFilter(Renderer::TexFilter f) {
	GLenum e;
	switch(f) {
		case Renderer::TexFilter::Linear: e = GL_LINEAR; break;
		case Renderer::TexFilter::NoFilter: e = GL_NEAREST; break;
	}

    glBindTexture(GL_TEXTURE_2D, master->ID());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, e);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, e);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Renderer::TexFilter TextureManager::GetFilter() {
    GLint out;
    glBindTexture(GL_TEXTURE_2D, master->ID());
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &out);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (out == GL_NEAREST) return Renderer::TexFilter::NoFilter;
	return Renderer::TexFilter::Linear;
}




void TextureManager::GarbageCollectBang() {
    return;
    if (garbageBag.empty()) return;

    //std::cout << "Attempting to remove " << garbageBag.size() << std::endl;

    // Copy old info
    int oldW = master->Width();
    int oldH = master->Height();
    GLubyte * copy = new GLubyte   [oldW * oldH * 4];
    GLubyte * subCopy = new GLubyte[oldW * oldH * 4];


    glBindTexture(GL_TEXTURE_2D, master->ID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, copy);


    // reset texture.
    delete master;
    master = new HugeTexture;
    cursorX = 0;
    cursorY = 0;
    cursorHeight = 0;



    // re-insert old textures.
    int * oldSubBounds;
    for(int i = 0; i < numTexUsed; ++i) {
        // skip over deleted texture / empty slots
        if (garbageBag.find(i)  != garbageBag.end() ||
            inactiveTex.find(i) != inactiveTex.end()) {
            //std::cout << "Skipping over tex i" << std::endl;
            continue;
        }

        oldSubBounds = GetSubTextureBounds(i);

        /*std::cout << i << " -> "
            << oldSubBounds[0] << ", "
            << oldSubBounds[1] << ", "
            << oldSubBounds[2] << ", "
            << oldSubBounds[3] << ", " << std::endl; */

        // copy old tex into subCopy
        for(int n = 0; n < oldSubBounds[3]; ++n) {
            memcpy(
                subCopy + 4*(oldSubBounds[2]*n),
                copy    + 4*(oldSubBounds[0] + (n+oldSubBounds[1])*oldW),
                oldSubBounds[2]*4
            );
        }

        // post image normally. Should overwrite bounds properly too.
        NewTexture(oldSubBounds[2], oldSubBounds[3], subCopy, i);

    }

    garbageBag.clear();
    //std::cout << "Reduced from " << oldW << "x" << oldH << " -> " << master->Width() << "x" << master->Height() << std::endl;

    delete[] copy;
    delete[] subCopy;
    /*
    if (garbageBag.empty()) return;


    GLubyte * copy = new GLubyte[master.w * master.h * 4];
    glBindTexture(GL_TEXTURE_2D, master.glID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, copy);
    GLuint oldBounds[numTexPhys*NUM_FLOATS_PER_TEX];
    std::vector<int> oldGarbage = garbageBag;
    garbageBag.clear();
    memcpy(oldBounds, texImageBounds, sizeof(GLuint) * numTexPhys * NUM_FLOATS_PER_TEX);
    int numOldTex = numTexUsed;
    int GUThOld = master.h;
    int GUTwOld = master.w;

    // reset GUT attributes
    master.w = DBUFFER_DEFAULT_INIT_LENGTH;
    master.h = DBUFFER_DEFAULT_INIT_LENGTH;
    master->Width()= 0;
    master->Height() = 0;


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 master.w, master.h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    // reInsert every sub texture that wasn't flagged for deletion
    for(int i = 0; i < numOldTex; ++i) {
        bool isFlagged = false;
        for(int n = 0; n < (int) oldGarbage.size(); ++n) {
            if (oldGarbage[n] == i) {
                isFlagged = true;
                break;
            }


        }

        if (!isFlagged) {
            int SCx = oldBounds[NUM_FLOATS_PER_TEX * i];
            int SCy = oldBounds[NUM_FLOATS_PER_TEX * i + 1];
            int SCw = oldBounds[NUM_FLOATS_PER_TEX * i + 2];
            int SCh = oldBounds[NUM_FLOATS_PER_TEX * i + 5];
            SCw -= SCx;
            SCh -= SCy;
            GLubyte * subCopy = new GLubyte[SCw * SCh * 4];

            for(int y = SCy; y < SCy + SCh; ++y) {
                for(int x = SCx; x < SCx + SCw; ++x) {
                    for(int n = 0; n < 4; ++n) {
                        subCopy[((y - SCy)*SCw + (x - SCx)) * 4 + n] =
                           copy[(y*(GUTwOld) + x) * 4 + n];

                    }
                }
            }


            NewTexture(SCw, SCh, subCopy, i);
            delete[] subCopy;
        } else {
            numTexUsed--;
        }

    }
    //cout << "...DONE removing images." << endl;


    delete[] copy;
    */
}


float TextureManager::MapTexCoordsToRealCoordsX(float texX, int tex) {
    return (texX*texImageBounds[4*tex+2] + texImageBounds[4*tex]) / master->Width();
}

float TextureManager::MapTexCoordsToRealCoordsY(float texY, int tex) {
    return (texY*texImageBounds[4*tex+3] + texImageBounds[4*tex+1]) / master->Height();
}

int TextureManager::GetActiveTextureSlot() {
    return GL_TEXTURE0+2;
}

GLuint TextureManager::GetTexture() {
    return master->ID();
}

int TextureManager::GetTextureW() { return master->Width(); }
int TextureManager::GetTextureH() { return master->Height(); }


void TextureManager::GetTextureData(int tex, uint8_t * data) {
    int * texBounds = GetSubTextureBounds(tex);

    master->Read(texBounds[0], texBounds[1], texBounds[2], texBounds[3], data);
}

int TextureManager::getNewTex() {
    if (inactiveTex.empty()) {
        // Store the texture coordinates cooresponding to this texture.
        if (numTexUsed >= numTexPhys) {

            //We need at least as many floats as one entry takes.
            int newSize = numTexPhys *DBUFFER_DEFAULT_RESIZE_FACTOR;
            GLint * copy = new GLint[newSize * NUM_FLOATS_PER_TEX];
            memcpy(copy, texImageBounds, numTexPhys * NUM_FLOATS_PER_TEX * sizeof(GLuint));
            delete[] texImageBounds;

            numTexPhys = newSize;
            texImageBounds = copy;
        }

        //cout << "Returning new tex id: " << numTexUsed << endl;
        return numTexUsed++;
    }



    // else get an old, inactive texture slot
    int out = *(inactiveTex.begin());
    inactiveTex.erase(inactiveTex.begin());
    //cout << "Returning inactive id " << out << endl;
    lastID = out;
    return out;
}



void TextureManager::ComputeTextureBindingData(const std::vector<std::pair<int, int>> & textures, RenderBuffer * texInfo, RenderBuffer * texEnabled) {
    static const int   TextureUniform_TextureListFloatIndex   = 4;
    static const int   TextureUniform_GUTWFloatIndex          = 0;
    static const int   TextureUniform_GUTHFloatIndex          = 1;
    static const float TextureUniform_EndOfBuffer = -1.f;
    static const float TextureUniform_InUse = 1.f;


    float value;
    float localBounds[4];
    GLint * bounds;


    // clear previous drawing's bindings.
    for(uint32_t i = 0; i < lastBindings.size(); ++i) {
        texEnabled->UpdateData(
            &TextureUniform_EndOfBuffer,
            i*4,
            1
        );
    }
    lastBindings.clear();
    // Just like with dynamic rendering, the texture handle is only really
    // used to get the proper coordinates of the GUT, which are passed to the shader


    for(uint32_t i = 0; i < textures.size(); ++i) {
        bounds = GetSubTextureBounds(textures[i].second);
        localBounds[0] = bounds[0];
        localBounds[1] = bounds[1];
        localBounds[2] = bounds[2];
        localBounds[3] = bounds[3];

        texEnabled->UpdateData(
            &TextureUniform_InUse,
            (textures[i].first)*4,
            1
        );

        texInfo->UpdateData(
            localBounds,
            TextureUniform_TextureListFloatIndex + (textures[i].first)*4,
            4
        );
        lastBindings.push_back(textures[i].first);
    }




    // also needs to update GUT dims
    float val;
    val = GetTextureW();
    texInfo->UpdateData(
        &val,
        TextureUniform_GUTWFloatIndex,
        1
    );


    //cout << "GUTW: " << val << endl;

    val = GetTextureH();
    texInfo->UpdateData(
        &val,
        TextureUniform_GUTHFloatIndex,
        1
    );

    //cout << "GUTH: " << val << endl;





}

#endif
