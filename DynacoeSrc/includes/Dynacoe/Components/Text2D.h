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


#ifndef H_DC_TEXT_2D_INCLUDED
#define H_DC_TEXT_2D_INCLUDED

#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Components/Render2D.h>
#include <Dynacoe/AssetID.h>
#include <Dynacoe/Color.h>

#include <map>


namespace Dynacoe {



class TextState;

/// \brief Aspect2D class that handles text rendering.
///
/// The Text2D class allows you to render text strings
/// onto the 2D context. Since it inherits from Aspect2D, all transformation
/// attributes are compatible are reflect text rendering operations
/// as if it were a whole image.
///
/// Note: If you with to render text in a 3D context, consider 
/// capturing rendered results through World::DrawToImage() function.
class Text2D : public Render2D {
  public:
    Text2D(const std::string & str = std::string(""), const Color & color = Color("white"));
    ~Text2D();

    /// \brief The mode by which text should be rendered
    ///
    enum class SpacingMode {
        Kerned,    ///< Attempts to produce the most natural spacing between text by taking into account any special character spacing rules determined by the font. This is the default.
        Monospace, ///< Space among characters is distributed uniformly regardless of input text. This is typically preferred for performance and for ease of determining space usage.
        Bitmap     ///< Spcifies space to be determined by the actual character extents only. This is at times useful, but may cause awkward spacing with certain fonts.

    };

    /// \brief Sets the active font to be the AssetID given
    ///
    /// @param font The font to render text with. There is no default font.
    void SetFont(AssetID font);

    /// \brief Sets the render resolution of the font.
    ///
    /// @param size The size that the font should be displayed as.
    void SetFontSize(int size);

    /// \brief The text that should be drawn.
    ///
    std::string text;


    /// \brief Sets the mode to render text against when drawn.
    ///
    /// @param mode The spacing mode of the text.
    void SetSpacingMode(SpacingMode mode);


    /// \brief Sets the character indices to be the colors specified.
    /// This setting persists across strings, so until another setting is 
    /// specified,  charBegin thru charEnd will be posted with the specified color.
    /// By default all text is white
    /// @param charBegin The first character to apply the color change to.
    /// @param charEnd The last character to apply the change to. All characters in between have the change applied to it.
    /// @param color The color to change to.
    void SetTextColor(int charBegin, int charEnd, const Color & color);
    void SetTextColor(const Color & color);


    /// \brief Returns the pixel position of the i'th character 
    /// in the stored string. 
    ///
    /// Often, it is useful to get the extents of the the text that you wish to render.
    /// This will tell you at what pixel offset from the aspect's position the character's 
    /// top-left corner will be. The first character is always (0, 0).
    /// @param i The index of the character to get the position of.
    Vector GetCharPosition(int i);


    /// \brief Returns the bounding box dimensions
    /// of the rendered text in pixels.
    ///
    Vector GetDimensions();


    void OnDraw();
    std::string GetInfo();

  private:
      


    void ReRender();
    
    
    TextState * modeInst;

    void * fontFace;
    int fontSize;
    SpacingMode fontSpacing;
    std::string srcText;
    
    std::map<Color, uint32_t> colorLookup;
    std::vector<Color> colorStorage;
    std::vector<uint32_t> colorIndex;

    Renderer::Vertex2D * vertices;
    std::vector<Vector> offsets;
    uint32_t numVertices;
    uint32_t numVerticesAllocd;
    Vector dimensions;
};
}    

#endif
