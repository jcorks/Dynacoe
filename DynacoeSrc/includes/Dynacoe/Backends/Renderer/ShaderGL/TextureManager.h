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

#ifndef H_DC_BACKENDS_TEXTUREMANAGER
#define H_DC_BACKENDS_TEXTUREMANAGER

#include <Dynacoe/Backends/Renderer/ShaderGL_Multi.h>
#include <set>

class HugeTexture;
namespace Dynacoe {
class RenderBuffer;
class TextureManager {
  public:
    TextureManager();

    // Add new texture, id is returned
    int NewTexture(int w, int h, GLubyte * data, int newTex);
    int NewTexture(int w, int h, GLubyte * data);

    // Remove texture
    void DeleteTexture(int tex);

    // Update data of an existing texture
    void UpdateTexture(int tex, GLubyte * data);

    // Filtering options (applicable when rendering)
    void SetFilter(Renderer::TexFilter f);
    Renderer::TexFilter GetFilter();

    int * GetSubTextureBounds(int id) {
        return texImageBounds+id * 4;
    };




    float MapTexCoordsToRealCoordsX(float texX, int tex);
    float MapTexCoordsToRealCoordsY(float texY, int tex);

    static int GetActiveTextureSlot();

    GLuint GetTexture();

    int GetTextureW();
    int GetTextureH();

    void GetTextureData(int tex, uint8_t*);

    int GetLastNewID() { return lastID; }
    
    void ComputeTextureBindingData(const std::vector<std::pair<int, int>> & textures, RenderBuffer * out, RenderBuffer * info);

  private:  
  
  
    

    void GarbageCollectBang();
    

    HugeTexture * master;
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
}

#endif



#endif
