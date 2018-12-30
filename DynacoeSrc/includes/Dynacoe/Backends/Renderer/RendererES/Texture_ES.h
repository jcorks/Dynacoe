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
#include <Dynacoe/Backends/Renderer/RendererES/RenderBuffer_ES.h>

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
    int * GetSubTextureBounds(int id) const;



    // converts local texture coordinates to global texture coordinates.
    void TranslateCoords(float * texX, float * texY, int tex) const;


    // Gets all occipied active texture slots and IDs utilized
    // by this texture manager by populating given arrays. This is guaranteed to be no 
    // more than 128. These slots can be used to directly 
    // populate a glUniform vector.
    int GetActiveTextureSlots(int * slots, int * ids);

    // Gets the slot index for the texture given. This slot should be used 
    // in GLSL to get the correct sampler.
    int GetSlotForTexture(int texture) const;




    // Gets the raw data of the given texture. It assumes that data can 
    // support w*h*4 bytes for that texture
    void GetTextureData(int tex, uint8_t*);

    // Returns the last added texture
    int GetLastNewID();
    


    using OnRebaseTextures = void (*)(void* data);

    // Adds a callback to called when global coordinates for 
    // local texture changes and need an update. This is usually 
    // relevant for when adding a texutre causes the size of the atlas to change 
    // or the layout to get rearranged for garbage collection
    void AddRebaseCallback(OnRebaseTextures, void * data);
    void RemoveRebaseCallback(OnRebaseTextures, void * data);

  private:  
  
  
    
    Texture_ESImplementation * ES;
    

};
}

#endif



#endif
