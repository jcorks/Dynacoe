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


/*
    Texture_ES holds multiple texture atlasses, all of which are passed to the 
    

*/


#if (defined DC_BACKENDS_GLES_X11)

#include <GLES2/gl2.h>
#include <Dynacoe/Backends/Renderer/RendererES/Texture_ES.h>
#include <Dynacoe/Backends/Renderer/StaticState.h>
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer_ES.h>
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


char color_to_char(const uint8_t * b4) {
    float shade = (((0.0 + b4[0] + b4[1] + b4[2]) / 3.0)/255.0) * (0.0 + b4[3])/255.0;
    return 32 + (char)(shade*84);
}

static void PRINT_BYTE_BLOCK(const uint8_t * src, int xT, int yT, int w, int h, int rowLength) {
    printf("%dx%d @ (%d, %d)\n",
        w, h, xT, yT);

    char rowData[w+1];
    rowData[w] = 0;
    src += (xT + yT*w)*4;
    for(uint32_t row = 0; row < h; ++row) {
        for(uint32_t x = 0; x < w; ++x) {
            rowData[x] = color_to_char(src);
            src+=4;
        }
        printf("%s\n", rowData);
        src += 4*(rowLength - w);
    }
    printf("\n");
        
}

class TextureAtlas  {
  public:
    TextureAtlas() {
        w = -1;
        h = -1;
        Reset();
    }

    void Reset() {

        if (w < 0 || h < 0) {
            glGenTextures(1, &glID); 
        
            int active;
            glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
            glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);


            glBindTexture(GL_TEXTURE_2D, glID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxLength);
            glActiveTexture(active);


            maxLength = 2048; // guaranteed minimum
            
        } else {
            delete[] realData;

        }

        w = DBUFFER_DEFAULT_INIT_LENGTH;
        h = DBUFFER_DEFAULT_INIT_LENGTH;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     w, h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);


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

        if (newH < h) newH = h;
        if (newW < w) newW = w;
        if (newW > maxLength) newW = maxLength;
        if (newH > maxLength) newH = maxLength;


        
        // lets copy the old data
        /*
        GLubyte * copy = new GLubyte[newW * newH * 4];
        memcpy(copy, realData, w*h*4);
        delete[] realData;
        realData = copy;
        */

        uint8_t * copy = new uint8_t[newW * newH * 4];
        uint8_t * from = realData;
        uint8_t * to = copy;    
        for(uint32_t row = 0; row < h; ++row) {
            memcpy(to, from, w*4);
            to += newW*4;
            from += w*4;
        }        
        delete[] realData;
        realData = copy;

        // create new tex and emplace as a sub image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newW, newH, 0, GL_RGBA, GL_UNSIGNED_BYTE, realData);



        w = newW;
        h = newH;


        //cout << "[DBUFFER] Resized GUT (" << newW << "x" << newH << ") (" << getTexCacheSize() << "MB)\n";

        glActiveTexture(active);

    }

    // Replaces a portion of the large texture
    void Emplace(int x, int y, int tw, int th, const uint8_t * data) {
        if (!data) return;
        int active;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
        glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);



        glBindTexture(GL_TEXTURE_2D, glID);
        if (data != NULL) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, data);
                  uint8_t * to = realData+(x + y*w)*4;
            const uint8_t * from = data;
            for(uint32_t row = y; row < y+th; ++row) {
                memcpy(to, from, tw*4);
                to += w*4;
                from += tw*4;
            }

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
        atlases.push_back(master);
        textureLimitToAtlas.push_back(0);


        currentAtlasIndex = 0;  
    }

    void GarbageCollectBang();

    int * GetSubTextureBounds(int id) {
        return texImageBounds+id * 4;
    };


    int NewTexture(int w, int h, const uint8_t * data, int newTexID) {

        /*
            Layout:
    
            -   Cursor X/Y determines where the topleft of the next image is 
            -   cursorHeight is the highest height span of the current 
                strip of textures for a row

        */
        GarbageCollectBang();


        if (w > master->MaxLength() ||
            h > master->MaxLength() ||
            w < 0 || h < 0) return DBUFFER_ERROR_INVALID_DIMENSIONS;


        // If adding the current texture would exceed the max length in the 
        // horizontal direction, then we move the cursor left back to 0 and move
        // it down.
        if (w +  cursorX >= master->MaxLength()) { 
            cursorY  = cursorHeight+1;
            cursorX  = 0;
            printf("%d %d exceeds max length of %d\n",
                w,
                cursorX,
                master->MaxLength()
            ); 
        }

        // if at any time a cursor + local width/height exhausts the space alloted in the 
        // atlas, we need to expand it.
        if (w + cursorX > master->Width() ||
            h + cursorY > master->Height()) {


            printf(
                "%dx%d needs to resize to accomodate %dx%d @ (%d,%d)...",
                master->Width(),
                master->Height(),
                w,
                h,
                cursorX,
                cursorY 
            );


            // Here, resize should be able to accomodate the incoming 
            // texture. If a width or height is specified that is smaller 
            // than the current w/h
            master->Resize(w + cursorX, h + cursorY);


            printf(
                "resized to %dx%d\n",
                master->Width(),
                master->Height()
              
            );


            for(uint32_t i = 0; i < rebaseCB.size(); ++i) {
                rebaseCB[i].first(rebaseCB[i].second);
            }
        }




        // if this check fails, resizing was not fully effective
        // and the texture was not able to be added
        if ((w + cursorX > master->Width() ||
             h + cursorY > master->Height())) {
            master = new TextureAtlas;
            atlases.push_back(master);
            textureLimitToAtlas.push_back(0);
            currentAtlasIndex++;


            cursorX = 0;
            cursorY = 0;
            cursorHeight = 0;
            return DBUFFER_ERROR_NO_MORE_TEXTURES;
        }


        // Place the texture as a sub texture of the GUT
        master->Emplace(cursorX, cursorY, w, h, data);

        // update row ceiling (AKA record height)
        if (cursorHeight < cursorY + h) cursorHeight = cursorY + h;






        GLint * newCoordSet = &texImageBounds[NUM_FLOATS_PER_TEX * newTexID];
        newCoordSet[0] = cursorX;
        newCoordSet[1] = cursorY;
        newCoordSet[2] = w;
        newCoordSet[3] = h;




        cursorX += w+1;




        return newTexID;
    }
    

    TextureAtlas * master;
    std::vector<TextureAtlas*> atlases;
    std::vector<int> textureLimitToAtlas;
    int currentAtlasIndex;


    
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
    std::vector<int> lastBindings;
    int lastID;

    std::vector<std::pair<Texture_ES::OnRebaseTextures, void*>> rebaseCB;
    
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

    std::set<int> garbageBagCopy = garbageBag;
    garbageBag.clear();

    int active;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
    glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);


    //std::cout << "Attempting to remove " << garbageBag.size() << std::endl;

    // Copy old info
    int oldW = master->Width();
    int oldH = master->Height();
    GLubyte * subCopy = new GLubyte[oldW * oldH * 4];
    uint8_t * copy = new GLubyte[oldW * oldH * 4];

    memcpy(copy, master->GetRawData(), oldH * oldW * 4);

    std::vector<std::pair<Texture_ES::OnRebaseTextures, void*>> copyRebase = rebaseCB;
    rebaseCB.clear();

    // reset texture.
    master->Reset();
    cursorX = 0;
    cursorY = 0;
    cursorHeight = 0;



    // re-insert old textures.
    int * oldSubBounds;
    int oldTexture;
    int oldNumTexUsed = numTexUsed;
    numTexUsed = 0;
    for(int i = 0; i < oldNumTexUsed; ++i) {
        // skip over deleted texture / empty slots
        if (garbageBagCopy.find(i)  != garbageBagCopy.end()) {
            //std::cout << "Skipping over tex i" << std::endl;
            continue;
        }

        oldSubBounds = GetSubTextureBounds(i);

        std::cout << i << " -> "
            << oldSubBounds[0] << ", "
            << oldSubBounds[1] << ", "
            << oldSubBounds[2] << ", "
            << oldSubBounds[3] << ", " << std::endl; 

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


        oldSubBounds = GetSubTextureBounds(i);

        std::cout << i << " -> "
            << oldSubBounds[0] << ", "
            << oldSubBounds[1] << ", "
            << oldSubBounds[2] << ", "
            << oldSubBounds[3] << ", " << std::endl; 

    }

    //std::cout << "Reduced from " << oldW << "x" << oldH << " -> " << master->Width() << "x" << master->Height() << std::endl;

    delete[] subCopy;
    rebaseCB = copyRebase;

    glActiveTexture(active);
    for(uint32_t i = 0; i < rebaseCB.size(); ++i) {
        rebaseCB[i].first(rebaseCB[i].second);
    }


}


void Texture_ES::TranslateCoords(float * texX, float * texY, int tex) const {
    *texX = (*texX*ES->texImageBounds[4*tex+2] + ES->texImageBounds[4*tex]  ) / ES->master->Width();
    *texY = (*texY*ES->texImageBounds[4*tex+3] + ES->texImageBounds[4*tex+1]) / ES->master->Height();

}


int Texture_ES::GetActiveTextureSlots(int *slots, int * ids) {
    int out = ES->atlases.size();
    int base = GL_TEXTURE0+0;
    for(int i = 0; i < out; ++i) {
        slots[i] = base+i;
        ids[i] = ES->atlases[i]->ID();
    }
    return out;    
}

int Texture_ES::GetSlotForTexture(int tex) {
    int out = ES->atlases.size();
    for(int i = 0; i < out; ++i) {
        if (ES->textureLimitToAtlas[i] <= tex) {
            return i;
        }
    }
    return 0;
}




void Texture_ES::GetTextureData(int tex, uint8_t * data) {
    int * texBounds = GetSubTextureBounds(tex);

    ES->master->Read(texBounds[0], texBounds[1], texBounds[2], texBounds[3], data);
}

int Texture_ESImplementation::getNewTex() {
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

        textureLimitToAtlas[currentAtlasIndex]=numTexUsed;

        //cout << "Returning new tex id: " << numTexUsed << endl;
        return numTexUsed++;



}





int Texture_ES::GetLastNewID() { return ES->lastID; }
    
void Texture_ES::AddRebaseCallback(Texture_ES::OnRebaseTextures rebase, void * data) {
    ES->rebaseCB.push_back({rebase, data});
}

void Texture_ES::RemoveRebaseCallback(Texture_ES::OnRebaseTextures rebase, void * data) {
    for(uint32_t i = 0; i < ES->rebaseCB.size(); ++i) {
        if (ES->rebaseCB[i].first == rebase &&
            ES->rebaseCB[i].second == data) {
            ES->rebaseCB.erase(ES->rebaseCB.begin() + i);
            return;
        }
    }
}


#endif
