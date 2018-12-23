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

#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Util/Math.h>
#include <vector>
#include <cmath>
#include <sstream>
#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Util/Random.h>
#include <Dynacoe/FontAsset.h>
#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Util/Time.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Modules/ViewManager.h>
#include <Dynacoe/Components/Text2D.h>
#include <unordered_map>

#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Util/Vector.h>
#include <Dynacoe/Util/TransformMatrix.h>
#include <Dynacoe/Util/Filesys.h>


static void storeSystemImages();



static const float DISPLAY_PIXEL_COORD_RATIO       =   1/(256.f);


using namespace Dynacoe;
using namespace std;



static Renderer * drawBuffer;




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





// The default coords for a quad being drawn via triangles
static float quadTex[] {
    0.f, 0.f,
    0.f, 1.f,
    1.f, 1.f,

    0.f, 0.f,
    1.f, 1.f,
    1.f, 0.f
};



/*
    DisplayTransform: A condensed implementation of the original OpenGL
    matrix transformation functions.

    It gives you more control, as you decide if / when
    these transformations are uploaded to the renderer.

    You can even give Transform the points that you wish
    to eventually render and obtain the corresponding
    homogenous points.

    Though, how you wish to optimize is up to you.

    Johnathan Corkery, 2014

*/



static void storeFont();
static void setDisplayMode(Renderer::Polygon p, Renderer::Dimension, Renderer::AlphaRule a);




void Graphics::Init() {
        static bool spawned = false;

        if (spawned) return;
        spawned = true;
        errorImage = new Image( "BadImage");
        fontID = AssetID();


        autoRefresh = true;
        filter = true;





        // immediate init
        quadVertices2D  = new float[24];
        transformResult = new float[24];
        transformResult2 = new float[24];

        quadVertices2D[2] = quadVertices2D[6] = quadVertices2D[10] = quadVertices2D[14] = quadVertices2D[18] = quadVertices2D[22] = 0.f;
        quadVertices2D[3] = quadVertices2D[7] = quadVertices2D[11] = quadVertices2D[15] = quadVertices2D[19] = quadVertices2D[23] = 1.f;





        drawBuffer = (Renderer*)Backend::CreateDefaultRenderer();






        TransformMatrix identity;
        identity.SetToIdentity();
        identity.ReverseMajority();



        state.alpha = Renderer::AlphaRule::Opaque;
        state.polygon = Renderer::Polygon::Line;
        state.dim = Renderer::Dimension::D_2D;


        //state.currentCamera2D = GetCamera2D().GetID();
        SetCamera2D(GetCamera2D());
        state.currentCamera2D.IdentifyAs<Camera>()->SetType(Camera::Type::Orthographic2D);
        state.currentCamera3D = GetCamera3D().GetID();
        state.currentCamera3D.IdentifyAs<Camera>()->BindTransformBuffers(
            drawBuffer->GetStaticViewingMatrixID(),
            drawBuffer->GetStaticProjectionMatrixID()
        );
        state.currentCamera3D.IdentifyAs<Camera>()->SetType(Camera::Type::Perspective3D);
        SetRenderCamera(*state.currentCamera2D.IdentifyAs<Camera>());

        //GetCamera2D().Node().local.reverse = true;

        storeSystemImages(); // TODO: make proper. It should be that images stored with no renderer initialized are just cached until a display is given.



        setDisplayMode(Renderer::Polygon::Triangle,
                       Renderer::Dimension::D_2D,
                       Renderer::AlphaRule::Allow);

}


void Graphics::InitAfter() {
    Engine::AttachManager(state.currentCamera3D);
    Engine::AttachManager(state.currentCamera2D);
    Engine::AttachManager(state.currentCameraTarget);


}


























static Renderer::Render2DStaticParameters params2D;





void Graphics::Draw(Render2D & aspect) {    
    aspect.CheckUpdate();
    
    Camera * cam2d = &GetCamera2D();
    if (!cam2d) return;


    setDisplayMode(aspect.GetPolygon(),
                   Renderer::Dimension::D_2D,
                   aspect.mode == Render2D::RenderMode::Translucent ? Renderer::AlphaRule::Translucent : Renderer::AlphaRule::Allow);

    if (round(params2D.contextWidth) != cam2d->Width() ||
        round(params2D.contextHeight) != cam2d->Height()) {

        UpdateCameraTransforms();
    }

    drawBuffer->Queue2DVertices(
        &aspect.GetVertexIDs()[0],
        aspect.GetVertexIDs().size()
    );


}


void Graphics::Draw(RenderMesh & aspect) {

    // flush out any queued dynamic actions
    setDisplayMode(aspect.GetRenderPrimitive(),
                   Renderer::Dimension::D_3D,
                   Renderer::AlphaRule::Allow);

    drawBuffer->Render2DVertices(params2D);

    aspect.RenderSelf(drawBuffer);

    // if applicable, recursively draw children
    /*
    if (aspect.NumChildren()) {
        RenderMesh * ptr;
        for(uint32_t i = 0; i < aspect.NumChildren(); ++i) {
            if ((ptr = (RenderMesh*)&aspect.GetChild(i)) &&
                ptr != &aspect)
                Draw(*ptr);
        }
    }*/

}





Renderer * Graphics::GetRenderer() {
    return drawBuffer;
}

void Graphics::SetRenderer(Renderer * r) {
    drawBuffer = r;
}







void Graphics::Commit() {

    //GetRenderCamera().GetFramebuffer()->RunCommand("dump-texture", nullptr);
    //GetRenderCamera().GetFramebuffer()->RunCommand("fill-debug", nullptr);

    drawBuffer->Render2DVertices(params2D);
    setDisplayMode(Renderer::Polygon::Triangle,
                   Renderer::Dimension::D_2D,
                   Renderer::AlphaRule::Allow);

 
    Display * d = ViewManager::Get(ViewManager::GetCurrent());
    Framebuffer * f = Graphics::GetRenderCamera().GetFramebuffer();
    if (d)
        d->Update(f);

        
    Camera * c = &GetRenderCamera();
    c->SwapBuffers();
    drawBuffer->AttachTarget(c->GetFramebuffer());
    if (c && c->autoRefresh) {
        c->Refresh();
    }
}
















void Graphics::DrawEachFrame(bool doIt) {
    autoRefresh = doIt;
}

bool Graphics::DrawEachFrame() {
    return autoRefresh;
}














void Graphics::EnableFiltering(bool doIt) {
    if (!doIt) {
        drawBuffer->SetTextureFilter(Renderer::TexFilter::NoFilter);
        filter = false;
    } else {
        drawBuffer->SetTextureFilter(Renderer::TexFilter::Linear);
        filter = true;
    }
}


/*
Image * Graphics::_getPNG(string path) {
    InputBuffer i(path);
    if (!i.Size()) return NULL;

    return _getTilesetPNG(path, 1, 1);
}

Image * Graphics::_getTilesetPNG(string path, int wImages, int hImages) {

    RawImage_PNG img(path, wImages, hImages, drawBuffer);
    return img.getImage();

}


void Graphics::drawTranslucent(bool doIt) {
    translucent = doIt;
}
*/



















void Graphics::SetCamera3D(Camera & c) {
    state.currentCamera3D = c.GetID();
    state.currentCamera3D.IdentifyAs<Camera>()->BindTransformBuffers(
        drawBuffer->GetStaticViewingMatrixID(),
        drawBuffer->GetStaticProjectionMatrixID()
    );
}

void Graphics::SetCamera2D(Camera & c) {
    auto cam = state.currentCamera2D.IdentifyAs<Camera>();
    if (cam) {
        cam->Node().SetReverseTranslation(false);
    }
    state.currentCamera2D = c.GetID();
    c.Node().SetReverseTranslation(true);
    c.Invalidate();
}

void Graphics::SetRenderCamera(Camera & c) {
    Camera * cam = &c;
    if (!cam) return;

    drawBuffer->Render2DVertices(params2D);



    Display * d = ViewManager::Get(ViewManager::GetCurrent());



    Framebuffer * fb = cam->GetFramebuffer();
    drawBuffer->AttachTarget(fb);



    state.currentCameraTarget = c.GetID();
    c.Step();
}


Camera & Graphics::GetCamera3D() {
    if (!state.currentCamera3D.IdentifyAs<Camera>()) {
        state.currentCamera3D = Entity::Create<Camera>();
        state.currentCamera3D.Identify()->SetName("Camera3D");
    }
    return *state.currentCamera3D.IdentifyAs<Camera>();
}


Camera & Graphics::GetCamera2D() {
    if (!state.currentCamera2D.IdentifyAs<Camera>()) {
        state.currentCamera2D = Entity::Create<Camera>();
        state.currentCamera2D.Identify()->SetName("Camera2D");

    }
    return *state.currentCamera2D.IdentifyAs<Camera>();
}


Camera & Graphics::GetRenderCamera() {
    if (!state.currentCameraTarget.IdentifyAs<Camera>()) {
        state.currentCameraTarget = Entity::Create<Camera>();
        state.currentCameraTarget.Identify()->SetName("RenderCamera");
    }
    return *state.currentCameraTarget.IdentifyAs<Camera>();
}




/// statics ///


void storeFont() {
    /*
    string path = Params::GetDynacoeDirectory() + STR(IMG_PATH) + "font.png";

    Image * img = _getTilesetPNG(path, 21, 21);
    img->path = "font$SYS";
    addToCache(img);
    fontID = img->index;


    defaultFontSpec.height          = getImage(fontID).height;
    defaultFontSpec.width           = getImage(fontID).width / 2.0;

    defaultFontSpec.fontFlyphScaleWidth = .5f;
    */

    defaultFontSpec.height          = Assets::Get<Image>(fontID).frames[0].Height();
    defaultFontSpec.width           = Assets::Get<Image>(fontID).frames[0].Width();
    defaultFontSpec.fontGlyphScaleWidth = 1.0f;

    //^ how much space does the width of the glyph take up the frame?

}




void Graphics::Flush2D() {
    drawBuffer->Render2DVertices(params2D);
}

void setDisplayMode(Renderer::Polygon p, Renderer::Dimension d, Renderer::AlphaRule a) {
	if (state.polygon != p || state.alpha != a || state.dim != d) {

        // Settings with which to draw have changed, so we commit what we have and start over
        drawBuffer->SetDrawingMode(state.polygon, state.dim, state.alpha);
		drawBuffer->Render2DVertices(params2D);

	    state.alpha = a;
        state.polygon = p;
        state.dim = d;
        drawBuffer->SetDrawingMode(state.polygon, state.dim, state.alpha);

	}

}

void Graphics::UpdateCameraTransforms() {
    Camera * cam2d = &Graphics::GetCamera2D(); 
    Graphics::GetRenderer()->Render2DVertices(params2D);
    params2D.contextWidth  = cam2d->Width();
    params2D.contextHeight = cam2d->Height();
    params2D.contextTransform = cam2d->GetGlobalTransform().GetData();
}

//Attempts to store the image(s) specified by the 32-bit pixel buffer given.
//
// Hidden denotes whether
// or not the compiled Image is stored in the standard image list or
// the private System one.
static AssetID storePixels(std::vector<uint32_t*> & pixels, std::string ID, int w, int h, bool hidden) {
    //Image * out = new Image();
    AssetID id = Assets::New(Assets::Type::Image, ID+(hidden?"$SYS" : ""));
    Image * out = &Assets::Get<Image>(id);
    for(uint32_t i = 0; i < pixels.size(); i++) {
        std::vector<uint8_t> data((uint8_t*)pixels[i], ((uint8_t*)pixels[i])+w*h*4);
        out->frames.push_back(Image::Frame(w, h, data));
    }
    return id;
}



#include <Dynacoe/SystemImages>
