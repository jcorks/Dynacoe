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

#if (DC_BACKENDS_GLES_X11)

#ifndef H_DC_BACKENDS_TEXTUREMANAGER
#define H_DC_BACKENDS_TEXTUREMANAGER

#include <GLES2/gl2.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer.h>

namespace Dynacoe {
class Texture_ESImplementation;


// Texture manager that couples normal texture behavior with a texture atlas,
// allowing multiple texture bindings per draw.

class Texture_ES {
  public:
    Texture_ES();
    ~Texture_ES();
    // Add new texture, id is returned
    int NewTexture(int w, int h, const uint8_t * data, int newTex);
    int NewTexture(int w, int h, const uint8_t * data);

    // Remove texture
    void DeleteTexture(int tex);

    // Update data of an existing texture
    void UpdateTexture(int tex, const uint8_t * data);

    // Filtering options (applicable when rendering)
    void SetFilter(Renderer::TexFilter f);
    Renderer::TexFilter GetFilter();

    
    // Gets an integer array representing the texel bounds of the given 
    // texture id. The array is 4 values of the following:
    // X (from topleft in texels)
    // Y (from topleft in texels)
    // width (in texels)
    // height (in texels)
    int * GetSubTextureBounds(int id);



    // converts local texture coordinates to global texture coordinates.
    float MapTexCoordsToRealCoordsX(float texX, int tex);
    float MapTexCoordsToRealCoordsY(float texY, int tex);

    // The active texture slot in opengl ES that the atlas should occupy. THis does not change.
    static int GetActiveTextureSlot();

    // Gets the texture id of the texture atlas
    GLuint GetAtlasTextureID();

    // Gets the dimensions of the atlas
    int GetAtlasTextureW();
    int GetAtlasTextureH();

    // Gets the raw data of the given texture. It assumes that data can 
    // support w*h*4 bytes for that texture
    void GetTextureData(int tex, uint8_t*);

    // Returns the last added texture
    int GetLastNewID();
    

    // Fills a set of render buffers based on given texture pairs.
    // THe texture pairs follow from the pair speicified in StaticState.
    // The 2 buffers:
    //
    // 1) textureCoords
    //
    // This buffer holds the atlas texture coordinates for each converted 
    // texture pair. These are normalized UVs in this order:
    // x0, y0, x1, y1
    void ComputeTextureBindingData(
        const std::vector<std::pair<int, int>> & textures, 
        RenderBuffer * textureCoords, 
        RenderBuffer * enableList
    );

  private:  
  
  
    
    Texture_ESImplementation * ES;
    

};
}

#endif



#endif
