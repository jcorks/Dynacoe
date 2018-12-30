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
const int DBUFFER_RESIZED                     =   -4;
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
        hasDeleted = false;

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
        textures.clear();
        cursorX = 0;
        cursorY = 0;
        cursorHeight = 0;

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

    GLuint ID() {
        /*
        int tex;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex);
        glBindTexture(GL_TEXTURE_2D, glID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, realData);
        glBindTexture(GL_TEXTURE_2D, tex);
        */
        return glID;

    }


    int NewSubTexture(int w, int h, const uint8_t * data, int * resized) {

        /*
            Layout:
    
            -   Cursor X/Y determines where the topleft of the next image is 
            -   cursorHeight is the highest height span of the current 
                strip of textures for a row

        */


        if (w > MaxLength() ||
            h > MaxLength() ||
            w < 0 || h < 0) return DBUFFER_ERROR_INVALID_DIMENSIONS;


        // If adding the current texture would exceed the max length in the 
        // horizontal direction, then we move the cursor left back to 0 and move
        // it down.
        if (w +  cursorX >= MaxLength()) { 
            cursorY  = cursorHeight+1;
            cursorX  = 0;
            printf("%d %d exceeds max length of %d\n",
                w,
                cursorX,
                MaxLength()
            ); 
        }

        // if at any time a cursor + local width/height exhausts the space alloted in the 
        // atlas, we need to expand it.
        if (w + cursorX > Width() ||
            h + cursorY > Height()) {


            printf(
                "%dx%d needs to resize to accomodate %dx%d @ (%d,%d)...",
                Width(),
                Height(),
                w,
                h,
                cursorX,
                cursorY 
            );


            // Here, resize should be able to accomodate the incoming 
            // texture. If a width or height is specified that is smaller 
            // than the current w/h
            Resize(w + cursorX, h + cursorY);


            printf(
                "resized to %dx%d\n",
                Width(),
                Height()
              
            );


            *resized = true;
        }




        // if this check fails, resizing was not fully effective
        // and the texture was not able to be added
        if ((w + cursorX > Width() ||
             h + cursorY > Height())) {
            return DBUFFER_ERROR_NO_MORE_TEXTURES;
        }


        // Place the texture as a sub texture of the GUT
        Emplace(cursorX, cursorY, w, h, data);

        // update row ceiling (AKA record height)
        if (cursorHeight < cursorY + h) cursorHeight = cursorY + h;



        textures.push_back({cursorX, cursorY, w, h});
        cursorX += w+1;


        return textures.size()-1;
    }



    void DeleteSubTexture(int i) {
        if (i < 0) {
            return;
        }

        textures[i].inUse = false;
        hasDeleted = true;
    }


    int * GetSubTextureBounds(int localID) {
        return &textures[localID].x;
    };

    void CleanupDeleted() {
        if (!hasDeleted) return;
        hasDeleted = false;
        uint8_t * dataCopy = new uint8_t[w * h * 4]; 
        uint8_t * subTex = new uint8_t[w * h * 4];
        uint8_t * dataCopy_root;
        
    
        memcpy(dataCopy, realData, w*h*4);
        std::vector<TextureInfo> textureInfoCopy = textures;
        int oldW = w;
        int oldH = h;

        Reset();

        for(uint32_t i = 0; i < textureInfoCopy.size(); ++i) {
            TextureInfo & info = textureInfoCopy[i];
            if (!info.inUse) {
                textures.push_back({});
                printf("%d was deleted and subsequently skipped\n", i);
                continue;
            }
            dataCopy_root = dataCopy + (info.x + info.y*oldW)*4;
            
            for(uint32_t y = 0; y < info.h; ++y) {
                memcpy(
                    subTex+info.w*4*y,
                    dataCopy_root,
                    info.w*4
                );
                dataCopy_root+=oldW*4;
            } 
            int r;
            NewSubTexture(info.w, info.h, subTex, &r);
        }
        delete[] subTex;
        delete[] dataCopy;  
        
        

    }




  private:

    GLuint glID;
    GLint maxLength;
    GLint w;
    GLint h;

    int cursorX;
    int cursorY;
    int cursorHeight;
    bool hasDeleted;

    uint8_t * realData;


    struct TextureInfo {
        TextureInfo() :
            x(0),
            y(0),
            w(0),
            h(0),
            inUse(false)
        {}

        TextureInfo(int _x, int _y, int _w, int _h) :
            x(_x),
            y(_y),
            w(_w),
            h(_h),
            inUse(true)
        {}

        void Set(int _x, int _y, int _w, int _h) {
            x = _x;
            y = _y;
            w = _w;
            h = _h;
            inUse = true;
        }
        int x;
        int y;
        int w;
        int h;
        char inUse;
    };


    std::vector<TextureInfo> textures;


};



void PRINT_BYTE_BLOCK_UVS(TextureAtlas * atlas, float xm, float ym, float x2m, float y2m) {
    PRINT_BYTE_BLOCK(
        atlas->GetRawData(),
        xm*atlas->Width(),
        ym*atlas->Height(),
        (x2m-xm)*atlas->Width(),
        (y2m-ym)*atlas->Height(),
        atlas->Width()
    );
    
}


struct LocalTextureHandle {
    LocalTextureHandle() :
        localID(0),
        atlasIndex(0),
        atlas(nullptr)
    {}
    int localID;
    int atlasIndex;
    TextureAtlas * atlas;
};



class Dynacoe::Texture_ESImplementation {
  public:
    Texture_ESImplementation() {
        atlases.push_back(new TextureAtlas());
        hasDeleted = false;
        numTexUsed = 0;
    }

    void GarbageCollectBang();

    int NewTexture(int w, int h, const uint8_t * data, int newTexID) {
        int resized = false;
        int localID = -1;

        int index = 0;
        TextureAtlas * master = atlases[0];

        while(localID < 0) {
            localID = master->NewSubTexture(w, h, data, &resized); 
                
            switch(localID) {
              case DBUFFER_ERROR_INVALID_DIMENSIONS:
              case DBUFFER_ERROR_NO_MORE_MEMORY:
                return -1;
              case DBUFFER_ERROR_NO_MORE_TEXTURES:
                // time to switch main textures;
                index++;            
                if (index >= atlases.size()) {
                    atlases.push_back(new TextureAtlas);
                }
                master = atlases[index];
                
                break;
              default:;
            }
        }
        
        while(newTexID >= masterIDtoLocalID.size()) {
            masterIDtoLocalID.push_back({});
        }
        masterIDtoLocalID[newTexID].localID    = localID;
        masterIDtoLocalID[newTexID].atlas      = atlases[index];
        masterIDtoLocalID[newTexID].atlasIndex = index;

        


        if (resized) {
            for(uint32_t i = 0; i < rebaseCB.size(); ++i) {
                rebaseCB[i].first(rebaseCB[i].second);
            }
        }

        return newTexID;
    }
    



    std::vector<LocalTextureHandle> masterIDtoLocalID;    
    std::vector<TextureAtlas*> atlases;
    std::vector<std::pair<Texture_ES::OnRebaseTextures, void*>> rebaseCB;
    
    int hasDeleted;
    int numTexUsed;

};









Texture_ES::Texture_ES() {
    ES = new Texture_ESImplementation();
}

int Texture_ES::NewTexture(int w, int h, const uint8_t * data) {
    ES->GarbageCollectBang();
    return ES->NewTexture(w, h, data, ES->numTexUsed++);
}

int Texture_ES::NewTexture(int w, int h, const uint8_t * data, int nt) {
    return ES->NewTexture(w, h, data, nt);
}







void Texture_ES::DeleteTexture(int tex) {
    if (tex >= 0 && tex < ES->numTexUsed) {
        ES->hasDeleted = true;
        LocalTextureHandle & handle = ES->masterIDtoLocalID[tex];
        handle.atlas->DeleteSubTexture(handle.localID);
    }

    
}


void Texture_ES::UpdateTexture(int tex, const uint8_t * data) {
    int * texBounds = GetSubTextureBounds(tex);
    LocalTextureHandle & handle = ES->masterIDtoLocalID[tex];

    handle.atlas->Emplace(
        texBounds[0], 
        texBounds[1], 
        texBounds[2], 
        texBounds[3], 
        data
    );
}



void Texture_ES::SetFilter(Renderer::TexFilter f) {
	GLenum e;
	switch(f) {
		case Renderer::TexFilter::Linear: e = GL_LINEAR; break;
		case Renderer::TexFilter::NoFilter: e = GL_NEAREST; break;
	}

    for(uint32_t i = 0; i < ES->atlases.size(); ++i) {
        TextureAtlas * atlas = ES->atlases[i];
        int active;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
        glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);


        glBindTexture(GL_TEXTURE_2D, atlas->ID());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, e);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, e);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(active);
    }
}

Renderer::TexFilter Texture_ES::GetFilter() {
    int active;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
    glActiveTexture(DBUFFER_GUT_TEX_ACTIVE);

    GLint out;
    glBindTexture(GL_TEXTURE_2D, ES->atlases[0]->ID());
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &out);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(active);

    if (out == GL_NEAREST) return Renderer::TexFilter::NoFilter;
	return Renderer::TexFilter::Linear;
}

int * Texture_ES::GetSubTextureBounds(int id) const {
    LocalTextureHandle & handle = ES->masterIDtoLocalID[id];
    return handle.atlas->GetSubTextureBounds(handle.localID);
};


void Texture_ESImplementation::GarbageCollectBang() {
    if (!hasDeleted) return;

    for(uint32_t i = 0; i < atlases.size(); ++i) {
        atlases[i]->CleanupDeleted();
    }
    hasDeleted = false;
    for(uint32_t i = 0; i < rebaseCB.size(); ++i) {
        rebaseCB[i].first(rebaseCB[i].second);
    }    
    
}


void Texture_ES::TranslateCoords(float * texX, float * texY, int tex) const {
    LocalTextureHandle & handle = ES->masterIDtoLocalID[tex];
    int * localC = GetSubTextureBounds(tex);
    *texX = (*texX*localC[2] + localC[0]) / handle.atlas->Width();
    *texY = (*texY*localC[3] + localC[1]) / handle.atlas->Height();

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

int Texture_ES::GetSlotForTexture(int tex) const {    
    if (tex < 0) return 0;
    LocalTextureHandle & handle = ES->masterIDtoLocalID[tex];
    return handle.atlasIndex;
}




void Texture_ES::GetTextureData(int tex, uint8_t * data) {
    int * texBounds = GetSubTextureBounds(tex);
    LocalTextureHandle & handle = ES->masterIDtoLocalID[tex];
    

    handle.atlas->Read(texBounds[0], texBounds[1], texBounds[2], texBounds[3], data);
}


    
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
