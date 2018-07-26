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


#ifndef DC_GRAPHICS_INCLUDED
#define DC_GRAPHICS_INCLUDED










#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Color.h>
#include <Dynacoe/Image.h>
#include <Dynacoe/Particle.h>

#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Components/Render2D.h>
#include <Dynacoe/Components/RenderMesh.h>
#include <Dynacoe/Camera.h>

#include <string>
#include <iostream>
#include <fstream>
#include <map>





namespace Dynacoe {
class Assets;
class Renderer;









/** \brief The central class for rendering.
 *
 * All user requests or rendering are done through this class.
 * Users may draw 2D and 3D primitives upon request,
 * alter the 2D and 3D viewing transformations,
 * load 2D and 3D visual assets and more.
 *
 *
 */
class Graphics : public Module {
  public:



    /* Drawing */
    /// \name 2D Drawing
    ///
    /// All 2D drawing operations are described here. Often, 2D drawing will
    /// require a Dynacoe::Vector, which has 3 components. In these cases,
    /// the Z-component is ignored. In Dynacoe, the origin 0,0 is located
    /// at the top-left of the screen, and with 2D drawing, all positions are in
    /// device coordinates, or pixels.
    ///\{


    /// \brief Draws an Aspect to the current display.
    ///
    static void Draw(Render2D &);



    ///\}


    /// \name 3D Drawing
    ///
    /// The only way to draw 3D geometry is through RenderMesh instances.
    /// The aspect mesh determin's how the geometry will be drawn.
    ///\{

    /// \brief Draws an Aspect to the current display.
    ///
    static void Draw(      RenderMesh &);

    ///\}



    /**
     *
     * \name Manual drawing

     *  If you want to force a frame draw, first clear the buffer,
     *  draw the scene, then commit the changes to the screen */
    /// \{
    /// \brief Forces the rendered scene to be drawn on the screen. This is normally done for you.
    ///

    static void Commit();



    /// \brief Sets whether or not to automatically draw, commit, and clear.
    ///
    static void DrawEachFrame(bool doIt);

    /// \brief Returns whether each frame is being drawn, updated, and swapped automatically by Dynacoe or not.
    ///
    static bool DrawEachFrame();
    ///\}







    /// \brief Retrieve the Renderer instance.
    ///
    /// Returns a pointer to the currently active underlying renderer currently being used by
    /// the graphics plugin. You normally will not need to use such a thing, but it may come in handy.
    static Renderer * GetRenderer();

    /// \brief Sets the current Renderer instance.
    ///
    /// All subsequent commands will utilize the given renderer
    /// to perform drawing operations. You normally will not need to use this
    /// , but it may come in handy.
    static void SetRenderer(Renderer *);









    /**
      *
      * \name Screen Options
      *
      * The functions here change how information on the screen is displayed.
      *
      */
    ///\{


    /// \brief Disables or enables bilinear filtering of Images during rendering.
    /// Bilinear filtering makes stratched images look less grainy.
    /// The default is not to enable
    static void EnableFiltering(bool doIt);






    /// \brief Sets the current camera to view from for 3D renderings.
    ///
    /// If invalid, the current camera is not changed.
    static void SetCamera3D(Camera &);

    /// \brief Sets the current camera to view from for 2D renderings.
    ///
    /// If invalid, the current camera is not changed.
    static void SetCamera2D(Camera &);

    /// \brief Sets the current camera that processes all visuals and hands them to the display i.e. the window.
    ///
    /// If invalid, the current camera is not changed.
    static void SetRenderCamera(Camera &);

    /// \brief Returns the current cameras.
    ///
    ///\{
    static Camera & GetCamera3D();
    static Camera & GetCamera2D();
    static Camera & GetRenderCamera();
    ///\}

    
  private:
    friend class Assets;
    friend class GraphicsResizeCallback;
    friend class Image;

    static Renderer * drawBuffer;



    /* Drawing state and display management */
	static void setDisplayMode(Renderer::Polygon p, Renderer::Dimension, Renderer::AlphaRule a);

    // draw immediates
    static float * transformResult;
    static float * transformResult2;
    static float * quadVertices2D;





    //Identical to coebeefSys function of the same name.
    //internal use onlyx

    static AssetID fontID;


    // Initializes a new particle as a copy of a cached particle
    static bool autoRefresh;

    static int filter;

    static AssetID lastDisplayID;


    friend class Aspect3D;
    friend class Material;


    static void storeFont();

    static Image * errorImage;
    static std::vector<int> consoleTextures;



    struct GraphicsState {
        Renderer::Dimension dim;
        Renderer::Polygon polygon;
        Renderer::AlphaRule alpha;



        Entity::ID currentCamera3D;
        Entity::ID currentCamera2D;
        Entity::ID currentCameraTarget;
    };

    struct FontSpec {
        int height;
        int width;
        float fontGlyphScaleWidth;
    };

    static GraphicsState state;
    static FontSpec defaultFontSpec;
  public:
        static void storeSystemImages();


  public:
    std::string GetName() { return "Graphics"; }
    void Init(); void InitAfter(); void RunBefore(); void RunAfter(); void DrawBefore(); void DrawAfter();
    Backend * GetBackend();
    static void Flush2D();
    static void UpdateCameraTransforms(Camera * );
};
};



#endif
