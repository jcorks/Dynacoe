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

#if (defined DC_BACKENDS_GLES_X11)

#include <GLES2/gl2.h>
#include <Dynacoe/Backends/Renderer/RendererES/Texture_ES.h>
#include <Dynacoe/Backends/Renderer/StaticState.h>
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer.h>
#include <cstring>
#include <set>

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




class TextureAtlas  {
  public:
    TextureAtlas() {
        glGenTextures(1, &glID);
        int active;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
        glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);
        w = DBUFFER_DEFAULT_INIT_LENGTH;
        h = DBUFFER_DEFAULT_INIT_LENGTH;



        glBindTexture(GL_TEXTURE_2D, glID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     w, h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxLength);
        glActiveTexture(active);


        realData = new uint8_t[w*h*4];
    }

    ~TextureAtlas() {
        glDeleteTextures(1, &glID);
        delete[] realData;
    }


    // Resize the main space to reqW x reqH
    void Resize(GLint reqW, GLint reqH) {
        int active;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
        glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);


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
        GLubyte * copy = new GLubyte[newW * newH * 4];
        memcpy(copy, realData, w*h*4);
        delete[] realData;
        realData = copy;


        // create new tex and emplace as a sub image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newW, newH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, copy);



        w = newW;
        h = newH;


        //cout << "[DBUFFER] Resized GUT (" << newW << "x" << newH << ") (" << getTexCacheSize() << "MB)\n";

        glActiveTexture(active);

    }

    // Replaces a portion of the large texture
    void Emplace(int x, int y, int tw, int th, const uint8_t * data) {
        int active;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
        glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);



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

        glActiveTexture(active);
    }

    uint8_t * GetRawData() const {
        return realData;
    }

    // Reads a portion of the large texture.
    void Read(int x, int y, int tw, int th, uint8_t * data) {

        for(int i = 0; i < th; ++i) {
            memcpy(data+i*tw*4, realData+(4*(x + (y+i)*w)), tw*4);
        }
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

    uint8_t * realData;
};






class Dynacoe::Texture_ESImplementation {
  public:
    Texture_ESImplementation() {
        texImageBounds = new GLint[NUM_FLOATS_PER_TEX * DBUFFER_DEFAULT_INIT_TCB_SIZE];
        numTexPhys = DBUFFER_DEFAULT_INIT_TCB_SIZE;
        numTexUsed = 0;
        cursorX = 0;
        cursorY = 0;
        cursorHeight = 0;
        master = new TextureAtlas();
    }

    void GarbageCollectBang();

    int * GetSubTextureBounds(int id) {
        return texImageBounds+id * 4;
    };


    int NewTexture(int w, int h, const uint8_t * data, int newTexID) {

        if (w > master->MaxLength() ||
            h > master->MaxLength() ||
            w < 0 || h < 0) return DBUFFER_ERROR_INVALID_DIMENSIONS;


        // Move origin if width exeeded. THat is, we dont move the cursorY
        // until the width is fully expanded.
        if (master->Width() == master->MaxLength()) {
            if (w +  cursorX >= master->MaxLength()) { // the resize request likely wont be able to get the image width-wise


                //std::cout << "[DBUFFER]: Reached max resizeble width, but going to increase height" << std::endl;

                // go to next row;
                cursorY += cursorHeight+1;
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


        cursorX += w+1;
        glBindTexture(GL_TEXTURE_2D, 0);




        return newTexID;
    }
    

    TextureAtlas * master;

    
    int cursorX;
    int cursorY;
    int cursorHeight;
    int getNewTex();    

    std::set<int> garbageBag;
    int * texImageBounds;

    int numTexPhys;
    int numTexUsed;
    GLuint subTex;
    int subTexW;
    int subTexH;
    std::set<int> inactiveTex;
    std::vector<int> lastBindings;
    int lastID;
    
};









Texture_ES::Texture_ES() {
    ES = new Texture_ESImplementation();
}

int Texture_ES::NewTexture(int w, int h, const uint8_t * data) {
    return ES->NewTexture(w, h, data, ES->getNewTex());
}

int Texture_ES::NewTexture(int w, int h, const uint8_t * data, int nt) {
    return ES->NewTexture(w, h, data, nt);
}







void Texture_ES::DeleteTexture(int tex) {
    if (tex >= 0 && tex < ES->numTexUsed) {
        ES->garbageBag.insert(tex);
        ES->inactiveTex.insert(tex);
    }
}


void Texture_ES::UpdateTexture(int tex, const uint8_t * data) {
    int SCx = ES->texImageBounds[NUM_FLOATS_PER_TEX * tex];
    int SCy = ES->texImageBounds[NUM_FLOATS_PER_TEX * tex + 1];
    int SCw = ES->texImageBounds[NUM_FLOATS_PER_TEX * tex + 2];
    int SCh = ES->texImageBounds[NUM_FLOATS_PER_TEX * tex + 3];


    //glBindTexture(GL_TEXTURE_2D, master->ID());
    //glTexSubImage2D(GL_TEXTURE_2D, 0, SCx, SCy, SCw, SCh, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //glBindTexture(GL_TEXTURE_2D, 0);
    ES->master->Emplace(SCx, SCy, SCw, SCh, data);
}



void Texture_ES::SetFilter(Renderer::TexFilter f) {
	GLenum e;
	switch(f) {
		case Renderer::TexFilter::Linear: e = GL_LINEAR; break;
		case Renderer::TexFilter::NoFilter: e = GL_NEAREST; break;
	}

    int active;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
    glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);


    glBindTexture(GL_TEXTURE_2D, ES->master->ID());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, e);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, e);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(active);
}

Renderer::TexFilter Texture_ES::GetFilter() {
    int active;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
    glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);

    GLint out;
    glBindTexture(GL_TEXTURE_2D, ES->master->ID());
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &out);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (out == GL_NEAREST) return Renderer::TexFilter::NoFilter;

    glActiveTexture(active);

	return Renderer::TexFilter::Linear;
}

int * Texture_ES::GetSubTextureBounds(int id) {
    return ES->GetSubTextureBounds(id);
};


void Texture_ESImplementation::GarbageCollectBang() {
    if (garbageBag.empty()) return;

    int active;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
    glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);


    //std::cout << "Attempting to remove " << garbageBag.size() << std::endl;

    // Copy old info
    int oldW = master->Width();
    int oldH = master->Height();
    GLubyte * subCopy = new GLubyte[oldW * oldH * 4];
    uint8_t * copy = master->GetRawData();




    // reset texture.
    TextureAtlas * masterOld = master;
    master = new TextureAtlas;
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

    delete[] subCopy;
    delete masterOld;


    glActiveTexture(active);
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


float Texture_ES::MapTexCoordsToRealCoordsX(float texX, int tex) {
    return (texX*ES->texImageBounds[4*tex+2] + ES->texImageBounds[4*tex]) / ES->master->Width();
}

float Texture_ES::MapTexCoordsToRealCoordsY(float texY, int tex) {
    return (texY*ES->texImageBounds[4*tex+3] + ES->texImageBounds[4*tex+1]) / ES->master->Height();
}

int Texture_ES::GetActiveTextureSlot() {
    return GL_TEXTURE0+2;
}

GLuint Texture_ES::GetAtlasTextureID() {
    return ES->master->ID();
}

int Texture_ES::GetAtlasTextureW() { return ES->master->Width(); }
int Texture_ES::GetAtlasTextureH() { return ES->master->Height(); }


void Texture_ES::GetTextureData(int tex, uint8_t * data) {
    int * texBounds = GetSubTextureBounds(tex);

    ES->master->Read(texBounds[0], texBounds[1], texBounds[2], texBounds[3], data);
}

int Texture_ESImplementation::getNewTex() {
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



void Texture_ES::ComputeTextureBindingData(const std::vector<std::pair<int, int>> & textures, RenderBuffer * texInfo, RenderBuffer * texEnabled) {
    static const int   TextureUniform_TextureListFloatIndex   = 4;
    static const int   TextureUniform_GUTWFloatIndex          = 0;
    static const int   TextureUniform_GUTHFloatIndex          = 1;
    static const float TextureUniform_EndOfBuffer = -1.f;
    static const float TextureUniform_InUse = 1.f;

    static std::vector<float> staging_texInfo;
    static std::vector<float> staging_texEnabled;

    if (staging_texEnabled.size() < textures.size()*4) {
        staging_texInfo.resize(textures.size()*4);
        staging_texEnabled.resize(textures.size()*4);
    }
    


    float value;
    float localBounds[4];
    GLint * bounds;

    uint32_t numTextures = textures.size();

    // clear previous drawing's bindings.
    float * staging_texEnabled_iter = &staging_texEnabled[0];
    for(uint32_t i = 0; i < numTextures; ++i) {
        *staging_texEnabled_iter = TextureUniform_EndOfBuffer;
        staging_texEnabled_iter+=4;
    }

    ES->lastBindings.clear();
    // Just like with dynamic rendering, the texture handle is only really
    // used to get the proper coordinates of the atlas, which are passed to the shader

    float w = (float)GetAtlasTextureW();
    float h = (float)GetAtlasTextureH();

    for(uint32_t i = 0; i < numTextures; ++i) {
        bounds = GetSubTextureBounds(textures[i].second);
        localBounds[0] = bounds[0] / w;
        localBounds[1] = bounds[1] / h;
        localBounds[2] = (bounds[2]+bounds[0]) / w;
        localBounds[3] = (bounds[3]+bounds[1]) / h;

        memcpy(
            &staging_texInfo[TextureUniform_TextureListFloatIndex + (textures[i].first)*4],
            localBounds,
            4*sizeof(float)
        );
            
        staging_texEnabled[(textures[i].first)*4] = TextureUniform_InUse;
        ES->lastBindings.push_back(textures[i].first);
    }




    // also needs to update GUT dims
    float val;
    staging_texInfo[TextureUniform_GUTWFloatIndex] = GetAtlasTextureW();
    staging_texInfo[TextureUniform_GUTHFloatIndex] = GetAtlasTextureH();


    texInfo   ->UpdateData(&staging_texInfo   [0], 1, numTextures);
    texEnabled->UpdateData(&staging_texEnabled[0], 1, numTextures);


}


int Texture_ES::GetLastNewID() { return ES->lastID; }
    

#endif
