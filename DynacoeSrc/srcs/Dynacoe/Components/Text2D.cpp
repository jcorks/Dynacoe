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


#include <Dynacoe/Components/Text2D.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/FontAsset.h>
#include <Dynacoe/Util/RefBank.h>
#include <Dynacoe/Util/Math.h>
#include <cassert>
#include "console.otf.h"


#ifdef DC_OS_LINUX
    #include <ft2build.h>
    #include <ftbitmap.h>
    #include <ftglyph.h>
    #include FT_FREETYPE_H
#else
    #include <ft2build.h>
    #include <freetype/ftbitmap.h>
    #include <freetype/ftglyph.h>
    #include FT_FREETYPE_H
#endif

using namespace Dynacoe;





// TODO: Move GlyphCache as a standalone class, where it alone interacts with FT.
//       Could be a formal backend too
// TODO: adbstract FT_Face to just FontID
class GlyphCache {
  public:


    int  Store(FT_Face * face, int size, char c) {
        GlyphID id(face, size, c);

        auto iter = ids.find(id);
        if (iter != ids.end()) {
            //std::cout << "Cache hit: '" << c << "' -> " << iter->second << std::endl;
            bank.Deposit(iter->second);
            return (iter->second); // cache hit!
        }




        // get th source glyph
        FT_GlyphSlot glyphSrc = (*face)->glyph;

        // problem, set glyph as bad char
        int status;
        if ((status = FT_Set_Pixel_Sizes((*face), 0, size)) &&
            (status = FT_Load_Char((*face), c, FT_LOAD_RENDER))) {
            // failed to load the glyph. Mark it with the bad char.
            return -1;
        }


        // form texture
        Dynacoe::Color srcColor("white");
        uint8_t * texData = new uint8_t[
            glyphSrc->bitmap.width *
            glyphSrc->bitmap.rows  *
            4
        ];


        int pitch = (glyphSrc->bitmap.pitch);
        pitch = (pitch < 0 ? -1 : 1) * pitch;
        // get the data *grumble*


        for(uint32_t y = 0; y < glyphSrc->bitmap.rows; ++y) {
            for(uint32_t x = 0; x < glyphSrc->bitmap.width; ++x) {
                switch(glyphSrc->bitmap.pixel_mode) {
                  case FT_PIXEL_MODE_GRAY:
                    srcColor.a = *(glyphSrc->bitmap.buffer + y*pitch + x);
                    break;
                  case FT_PIXEL_MODE_MONO:
                    srcColor.a = 255 * (((*(glyphSrc->bitmap.buffer + y*pitch)) << x) & 0x1);
                    break;
                }
                *((uint32_t*)(texData+(y*glyphSrc->bitmap.width + x)*4)) = srcColor.Int();
            }
        }


        int tex = Graphics::GetRenderer()->AddTexture(
            glyphSrc->bitmap.width,
            glyphSrc->bitmap.rows,
            texData
        );
        delete[] texData;
        ids[id] = tex;
        bank.Deposit(tex);;
        //std::cout << "Add to cache: '" << c << "' -> " << tex << std::endl;
        return tex;

    }

    void Remove(FT_Face * face, int size, char c) {
        GlyphID id(face, size, c);

        auto iter = ids.find(id);
        if (iter != ids.end()) {
            bank.Withdraw(iter->second);
        } else {
            return;
        }

        if (!bank.GetBalance(iter->second)) {
            //std::cout << "Removed '" << c << "' -> " << iter->second << std::endl;
            Graphics::GetRenderer()->RemoveTexture(iter->second);
            ids.erase(iter);
        }
    }

    //TODO:
    int GetKerning(FT_Face * face, char c, char b);

    int GetFontBoundWidth(FT_Face * face);

    int GetFontBoundHeight(FT_Face * face);



  private:


    struct GlyphID {
        GlyphID(FT_Face * face_, int size_, char c_) :
            face(face_),
            size(size_),
            c(c_) {


        }
        FT_Face * face;
        int       size;
        char      c;

        bool operator<(const GlyphID & other) const {
            return face < other.face ||
                   c < other.c ||
                   size < other.size;
        }


    };


    RefBank<int> bank;
    std::map<GlyphID, int> ids; // glyphID to tex.

};


static GlyphCache glyphCache;

class Dynacoe::TextState {
  public:
    const int     bad_glyph_slot = 0;
    const uint8_t bad_glyph_char = '?';



    TextState() {
        glyphW = 0;
        glyphL = 0;
        glyphU = 0;
        fontSize = 0;
        fontFace = nullptr;
        mode = Text2D::SpacingMode::Kerned;
    }

    struct GlyphInfo {
        int xOffset;  // offset from the current origin
        int yOffset;  // ^
        int yNext;    // How much origin should be moved for the next character
        int xNext;    // ^
        int width;    // THe width in pixels of the character
        int height;   // the height in pixels of the character
        int tex;      // Texture handle
        char c;       // the actual character
        bool empty;   // //returns if the glyph visual should be overridden with degenerate vertices

    };


    void SetMode(Text2D::SpacingMode m) {
        mode = m;
    }


    void GetMonospaceInfo(const char * str, int i, GlyphInfo * space) {
        space->tex = CharToTexture(str[i]);
        int curSlot = charMap[str[i]];
        space->c = str[i];
        // offset from the top
        space->yOffset  = (glyphU) - glyphs_u[curSlot];
        // offset needed to center the glyph.
        space->xOffset  = (glyphW - glyphs_w[curSlot])/2;
        space->xNext    = glyphW;
        space->yNext    = (str[i] == '\n' ? glyphU+glyphL : 0);
        space->empty    = (isspace(str[i]));
        space->width    = glyphs_w[curSlot];
        space->height   = glyphs_u[curSlot] + glyphs_l[curSlot];
    }


    void GetBitmapInfo(const char * str, int i, GlyphInfo * space) {
        space->tex = CharToTexture(str[i]);
        int curSlot = charMap[str[i]];
        space->c = str[i];
        space->yOffset = (glyphU) - glyphs_u[curSlot];
        // offset needed to center the glyph.
        space->xOffset = 0;
        //space->xNext += (*fontFace)->glyph->advance.x/64;
        space->xNext   = glyphs_w[curSlot];
        space->yNext    = (str[i] == '\n' ? glyphU+glyphL : 0);
        space->empty    = (isspace(str[i]));
        space->width    = glyphs_w[curSlot];
        space->height   = glyphs_u[curSlot] + glyphs_l[curSlot];
    }


    void GetKerningInfo(const char * str, int i, GlyphInfo * space) {
        space->tex = CharToTexture(str[i]);
        int curSlot = charMap[str[i]];
        space->c = str[i];
        space->yOffset = (glyphU) - glyphs_u[curSlot];
        // offset needed to center the glyph.
        space->xOffset = glyphs_hb[curSlot];
        if (FT_HAS_KERNING((*fontFace)) && i > 0) {
            static FT_Vector kernResult;
            FT_Get_Kerning(
                *fontFace,
                FT_Get_Char_Index(*fontFace, str[i-1]),
                FT_Get_Char_Index(*fontFace, str[i]),
                FT_KERNING_DEFAULT,
                &kernResult
            );
            space->xNext   = kernResult.x;
        } else {
            space->xNext = 0;
        }
        space->xNext += glyphs_xa[curSlot];

        space->yNext    = (str[i] == '\n' ? glyphU+glyphL : 0);
        space->empty    = (isspace(str[i]));
        space->width    = glyphs_w[curSlot];
        space->height   = glyphs_u[curSlot] + glyphs_l[curSlot];
    }








    // produces raw vrtices
    void Render(
        const char * str, uint32_t strLen, int size, FT_Face * id, // in
        //float r, float g, float b, float a,
        const std::vector<Color> & colorStorage, const std::vector<uint32_t> & colorIndices, 
        std::vector<Renderer::Vertex2D> & vertices, 
        std::vector<Vector> & pos,                // out
        Vector & dimensions
    ) {
        uint32_t neededCount = strLen*6; // just assume no spaces for worst case alloc
        



        // errHandling
        if (!id || !neededCount || !strLen || size < 1) return;

        // uh oh, new font! Reset cached info
        FT_Set_Pixel_Sizes(*id, 0, size);
        if (fontFace != id || fontSize != size) {
            //std::cout << "(" << fontFace << ")->" << id << '|'
            //          << "o(" << fontSize << ")->" << size << std::endl;
            DumpGlyphCache();

            fontFace = id;
            fontSize = size;

            RebaseSizing();

        }



        // Re-alloc
        /*if (vertices.size() < neededCount) {
            vertices.resize(neededCount);
            pos.resize(neededCount+1);
        }*/
        vertices.clear();
        pos.clear();



        // gen vertices for each character
        Renderer::Vertex2D * vtex;
        int xIter = 0;
        int yIter = 0;

        static Renderer::Vertex2D empty = Renderer::Vertex2D(
            0, 0, 0, 0, 0, 0, 0, 0, 0
        );

        int colorIndex = 0;
        const Color * curColor = nullptr;
        float r, g, b, a;
        dimensions = {0, 0};
        for(uint32_t i = 0; i < strLen; ++i) {
            static GlyphInfo info;
            pos.push_back(Vector(xIter, yIter));



            if (mode == Text2D::SpacingMode::Monospace) {
                GetMonospaceInfo(str, i, &info);
            } else if (mode == Text2D::SpacingMode::Bitmap) {
                GetBitmapInfo(str, i, &info);
            } else {
                GetKerningInfo(str, i, &info);
            }




            if (info.empty) {
            
                
            } else {
                
                curColor = &colorStorage[colorIndices[(i >= colorIndices.size() ? colorIndices.size()-1 : i)]];
                r = curColor->r;
                g = curColor->g;
                b = curColor->b;
                a = curColor->a;


                // topleft
                vertices.push_back(Renderer::Vertex2D(
                    info.xOffset+xIter, info.yOffset+yIter,
                    r, g, b, a,
                    info.tex, 0, 0
                ));
                
                vertices.push_back(Renderer::Vertex2D(
                    info.xOffset+xIter+info.width, info.yOffset+yIter,
                    r, g, b, a,
                    info.tex, 1.f, 0.f
                ));
                                
                
                vertices.push_back(Renderer::Vertex2D(
                    info.xOffset+xIter+info.width, info.yOffset+yIter+info.height,
                    r, g, b, a,
                    info.tex, 1.f, 1.f
                ));
                
                vertices.push_back(vertices[vertices.size() - 1]);
                
                vertices.push_back(Renderer::Vertex2D(
                    info.xOffset+xIter, info.yOffset+yIter+info.height, 
                    r, g, b, a,
                    info.tex, 0.f, 1.f
                ));
                
                vertices.push_back(vertices[vertices.size() - 5]);

                if (info.xOffset+xIter+info.width  > dimensions.x) dimensions.x = info.xOffset+xIter+info.width;
                if (info.yOffset+yIter+info.height > dimensions.y) dimensions.y = info.yOffset+yIter+info.height;
            }





            xIter += info.xNext;
            if (info.yNext) {
                yIter += info.yNext;
                xIter = 0;
            }

            //FT_Load_Char(*fontFace, curChar, FT_LOAD_RENDER);
            //xIter += (*fontFace)->glyph->advance.x/64;

        }
        pos.push_back(Vector(xIter, yIter));




    }

  private:
    // Redetermine what the monospace sizing should be
    void RebaseSizing() {
        // glyphW = 0, glyphH = 0;
        //for each glyph
            // if glyph width > glyphW, glyphW = glyph width
            // if glyph heigh > glyphH, glyphH = glyph Height

        int status = FT_Set_Pixel_Sizes(*fontFace, 0, fontSize);
        assert(status==0);
        FT_GlyphSlot glyph = (*fontFace)->glyph;
        glyphW = 0;
        glyphU = 0;
        glyphL = 0;
        //for(uint32_t i = 'A'; i < 'Z'; ++i) {

        for(uint32_t i = 33; i < 126; ++i) {
            status = FT_Load_Char(*fontFace, i, FT_LOAD_RENDER);
            assert(status==0);
            if (glyphW < glyph->bitmap.width)
                glyphW = glyph->bitmap.width;
            if (glyphU < glyph->metrics.horiBearingY/64)
                glyphU = glyph->metrics.horiBearingY/64;
            if (glyphL < glyph->bitmap.rows - glyph->metrics.horiBearingY/64)
                glyphL = glyph->bitmap.rows - glyph->metrics.horiBearingY/64;
        }

    }

    // delete all textures, reset image cache
    void DumpGlyphCache() {
        // first, remove all tex refs
        for (uint32_t i = 0; i < glyphsSrc.size(); ++i) {
            glyphCache.Remove(fontFace, fontSize, glyphsSrc[i]);
        }

        glyphs.clear();
        glyphsSrc.clear();
        glyphs_w.clear();
        glyphs_xa.clear();
        glyphs_hb.clear();
        glyphs_u.clear();
        glyphs_l.clear();
        charMap.clear();
        charMap.resize(UINT8_MAX, -1);
    }

    // returns the texture that represents the char c.
    // This function will cache any glyph textures already generated
    int CharToTexture(uint8_t c) {
        // cache hit, we're good
        if (charMap[c] > 0) return glyphs[charMap[c]];


        // First set the error glyph
        if (!glyphs.size() && c != bad_glyph_char) {
            int badCharTex = CharToTexture(bad_glyph_char);
        }


        // get th source glyph
        FT_GlyphSlot glyphSrc = (*fontFace)->glyph;

        int status;
        // problem, set glyph as bad char
        if (status = FT_Load_Char((*fontFace), c, FT_LOAD_RENDER)) {
            // failed to load the glyph. Mark it with the bad char.
            charMap[c] = bad_glyph_slot;
            return charMap[c];
        }


        if (!glyphSrc->bitmap.buffer ||
            (glyphSrc->bitmap.buffer && (!glyphSrc->bitmap.rows || !glyphSrc->bitmap.pitch))){
            charMap[c] = bad_glyph_slot;
            return charMap[c];
        }


        // map new texture to text characters
        glyphs.push_back(glyphCache.Store(fontFace, fontSize, c));
        glyphsSrc.push_back(c);
        glyphs_w.push_back(glyphSrc->bitmap.width);
        glyphs_xa.push_back(glyphSrc->advance.x/64);
        glyphs_hb.push_back(glyphSrc->metrics.horiBearingX/64);
        glyphs_u.push_back(glyphSrc->metrics.horiBearingY/64);
        glyphs_l.push_back(glyphSrc->bitmap.rows - glyphSrc->metrics.horiBearingY/64);
        charMap[c] = glyphs.size()-1;


        return glyphs[charMap[c]];
    }


    FT_Face*             fontFace;

    int                  fontSize;
    std::vector<int>     charMap; // char to texSlot
    std::vector<int>     glyphs;  // texSlot to tex
    std::vector<int>     glyphsSrc;
    std::vector<int>     glyphs_w;
    std::vector<int>     glyphs_hb;
    std::vector<int>     glyphs_xa;
    std::vector<int>     glyphs_l;
    std::vector<int>     glyphs_u;


    int                  glyphW;
    int                  glyphU; // glyph "upper": pixels above the baseline
    int                  glyphL; // glyph "lower": pixels below the baseline
    Text2D::SpacingMode mode;

};









class FontCache {
  public:

    TextState * Get(int size, FT_Face * fontFace, Text2D::SpacingMode space) {
        if (size <= 0 || !fontFace) return NULL;
        FontID id(size, fontFace, space);


        /*
        std::cout << "IncRefCount (" << size
                  << "|" << fontFace << "|" << (int)space
                  << ")->";
        */
        auto iter = ids.find(id);

        // hit, return existing text state
        if (iter != ids.end()) {
            bank.Deposit(iter->second);

            return iter->second;
        }

        TextState * state = new TextState;

        bank.Deposit(state);

        ids[id] = state;
        return state;
    }

    
    void Remove(int size, FT_Face * fontFace, Text2D::SpacingMode space) {
        FontID id(size, fontFace, space);

        //return;

        auto iter = ids.find(id);
        if (iter != ids.end()) {
            bank.Withdraw(iter->second);
        } else {

            return;
        }

        /*
        std::cout << "DecRefCount (" << size
                  << "|" << fontFace << "|" << (int)space
                  << ")->";
        std::cout << iter->second << "("
                  << bank.GetBalance(iter->second)
                  << ")" << std::endl;
        */

        if (!bank.GetBalance(iter->second)) {
            delete iter->second;
            ids.erase(iter);
        }
    }



  private:
    struct FontID {
        FontID(int size_, FT_Face * face_, Text2D::SpacingMode space_) :
            size(size_),
            face(face_),
            space(space_)
        {}

        int size;
        FT_Face * face;
        Text2D::SpacingMode space;


        bool operator<(const FontID & other) const {
            return face < other.face ||
                   size < other.size ||
                   space < other.space;
        }


    };

    std::map<FontID, TextState *> ids;
    RefBank<TextState*> bank;
};


static FontCache fontCache;






// Notes:
//  -   charMap['i'] contains the texture that resolves the utf-8 character 'i'
//  -   charMap[\0] contains the texture that resolves unfound glyphs. If the


const uint8_t      bad_char_c           = '?';
const Color        default_text_color_c = Color(255, 255, 255, 255);

Text2D::Text2D(const std::string & str, const Color & clr) : Render2D("Text2D") {
    static AssetID defaultFont;
    numVertices = 0;
    numVerticesAllocd = 0;

    fontFace  = nullptr;
    fontSize  = 12;
    fontSpacing = SpacingMode::Kerned;

    modeInst = nullptr;

    if (!defaultFont.Valid()) {
        std::vector<uint8_t> v(sizeof(__console_otf_h_buffer));
        memcpy(&v[0], __console_otf_h_buffer, v.size());
        defaultFont = Assets::LoadFromBuffer(
            "otf",
            "DYNACOE_console.otf",
            v
        );
    }
    SetTextColor(clr);
    SetFont(defaultFont);
    
    text = str;
}

Text2D::~Text2D() {
    fontCache.Remove(fontSize, (FT_Face*)fontFace, fontSpacing);
}




void Text2D::SetFont(AssetID id) {
    if (!id.Valid()) return;
    if (modeInst) fontCache.Remove(fontSize, (FT_Face*)fontFace, fontSpacing);
    fontFace = Assets::Get<FontAsset>(id).fontFace;
    if (!fontFace || fontSize<=0) return;

    modeInst =    fontCache.Get(fontSize, (FT_Face*)fontFace, fontSpacing);


    ReRender();
}

void Text2D::SetFontSize(int size) {
    if (modeInst) fontCache.Remove(fontSize, (FT_Face*)fontFace, fontSpacing);
    fontSize = size;
    if (!fontFace || fontSize<=0) return;


    modeInst =    fontCache.Get(fontSize, (FT_Face*)fontFace, fontSpacing);


    ReRender();

}




void Text2D::SetSpacingMode(SpacingMode m) {
    if (modeInst) fontCache.Remove(fontSize, (FT_Face*)fontFace, fontSpacing);
    fontSpacing = m;
    if (!fontFace || fontSize<=0) return;


    modeInst =    fontCache.Get(fontSize, (FT_Face*)fontFace, fontSpacing);
    modeInst->SetMode(fontSpacing);
    ReRender();
}

void Text2D::SetTextColor(const Color & color) {
    colorLookup.clear();
    colorStorage.clear();
    colorIndex.clear();

    colorIndex.push_back(0);
    colorStorage.push_back(color);
    if (!fontFace || fontSize<=0) return;
    ReRender();

}

void Text2D::SetTextColor(int charBegin, int charEnd, const Color & color) {
    if (charBegin > charEnd) {
        std::swap(charBegin, charEnd);
    }

    auto iter = colorLookup.find(color);
    if (iter == colorLookup.end()) {
        colorStorage.push_back(color);
        colorLookup[color] = colorStorage.size()-1;
        iter = colorLookup.find(color);
    }
    uint32_t index = iter->second;

    while(charEnd >= colorIndex.size()) {
        colorIndex.push_back(colorIndex[colorIndex.size()-1]);
    }

    for(int i = charBegin; i <= charEnd; ++i) {
        colorIndex[i] = index;
    }


    // rerender if we can
    if (!fontFace || fontSize<=0) return;
    ReRender();

}


Vector Text2D::GetCharPosition(int i) {
    if (!offsets.size()) return Vector();
    Math::Clamp(i, 0, offsets.size()-1);
    return offsets[i];
}


Vector Text2D::GetDimensions() {
    if (text != srcText) {
        srcText = text;
        if (modeInst) fontCache.Remove(fontSize, (FT_Face*)fontFace, fontSpacing);


        if (!fontFace || fontSize<=0) return dimensions;
        modeInst =    fontCache.Get(fontSize, (FT_Face*)fontFace, fontSpacing);

        ReRender();
    }
    return dimensions;
}


//// private:


static Color defaultC = "white";


void Text2D::ReRender() {
    if (!Graphics::GetRenderer())return;
    /*
    for(int i = 0; i < srcText.Length(); ++i) {
        if (unusedColor[i])
            colorTable[i] = defaultC;
    }
    */
    std::vector<Renderer::Vertex2D> vertices;
    modeInst->Render(
        srcText.c_str(), srcText.size(), fontSize, (FT_Face*)fontFace,
        colorStorage, colorIndex,
         vertices, offsets, dimensions
    );
    SetVertices(vertices);
    SetPolygon(Renderer::Polygon::Triangle);
}

void Text2D::OnDraw() {
    if (text != srcText) {
        srcText = text;
        if (modeInst) fontCache.Remove(fontSize, (FT_Face*)fontFace, fontSpacing);


        if (!fontFace || fontSize<=0) return;
        modeInst =    fontCache.Get(fontSize, (FT_Face*)fontFace, fontSpacing);

        ReRender();
    }
    Graphics::Draw(*this);
}

std::string Text2D::GetInfo() {
    return Chain() <<
        "\"" << text << "\"\n" <<
        "Font Size: " << fontSize << "\n" <<
        "Dimensions: " << GetDimensions()
    ;
}
