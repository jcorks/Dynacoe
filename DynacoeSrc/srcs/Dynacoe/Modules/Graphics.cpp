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


static const float DISPLAY_PIXEL_COORD_RATIO       =   1/(256.f);


using namespace Dynacoe;
using namespace std;





float *           Graphics::transformResult;
float *           Graphics::transformResult2;
float *           Graphics::quadVertices2D;

AssetID             Graphics::fontID;

bool                Graphics::autoRefresh;
int                 Graphics::filter;


Image *             Graphics::errorImage;
Renderer *        Graphics::drawBuffer;

AssetID             Graphics::lastDisplayID;



Graphics::GraphicsState     Graphics::state;
Graphics::FontSpec          Graphics::defaultFontSpec;


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


static std::string fSearch(const std::string &);


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

        //GetCamera2D().node.local.reverse = true;

        Graphics::storeSystemImages(); // TODO: make proper. It should be that images stored with no renderer initialized are just cached until a display is given.



        setDisplayMode(Renderer::Polygon::Triangle,
                       Renderer::Dimension::D_2D,
                       Renderer::AlphaRule::Allow);

}

Backend * Graphics::GetBackend() {
    return Graphics::GetRenderer();
}

void Graphics::InitAfter() {
    Engine::AttachManager(state.currentCamera3D);
    Engine::AttachManager(state.currentCamera2D);
    Engine::AttachManager(state.currentCameraTarget);


}



void Graphics::DrawBefore() {


}

void Graphics::DrawAfter() {

}


void Graphics::RunBefore() {

}

void Graphics::RunAfter() {
}
























static Renderer::Render2DStaticParameters params2D;


void Graphics::UpdateCameraTransforms(Camera * c) {
    Camera * cam2D = &Graphics::GetCamera2D();
    if (cam2D == c) {
        Graphics::GetRenderer()->Render2DVertices(params2D);
        params2D.contextWidth  = cam2D->Width();
        params2D.contextHeight = cam2D->Height();
        params2D.contextTransform = cam2D->GetGlobalTransform().GetData();
    }
}


void Graphics::Draw(Render2D & aspect) {    
    aspect.CheckUpdate();
    
    Camera * cam2d = &GetCamera2D();
    if (!cam2d) return;


    setDisplayMode(aspect.GetPolygon(),
                   Renderer::Dimension::D_2D,
                   aspect.mode == Render2D::RenderMode::Translucent ? Renderer::AlphaRule::Translucent : Renderer::AlphaRule::Allow);

    if (round(params2D.contextWidth) != cam2d->Width() ||
        round(params2D.contextHeight) != cam2d->Height()) {
        UpdateCameraTransforms(cam2d);
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


void Graphics::setDisplayMode(Renderer::Polygon p, Renderer::Dimension d, Renderer::AlphaRule a) {
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

    if (d)
        d->Update();

    Camera * c = &GetRenderCamera();
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













void Graphics::storeFont() {
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
        cam->node.SetReverseTranslation(false);
    }
    state.currentCamera2D = c.GetID();
    c.node.SetReverseTranslation(true);
    c.Invalidate();
}

void Graphics::SetRenderCamera(Camera & c) {
    Camera * cam = &c;
    if (!cam) return;

    drawBuffer->Render2DVertices(params2D);



    Display * d = ViewManager::Get(ViewManager::GetCurrent());



    Framebuffer * fb = cam->GetFramebuffer();
    drawBuffer->AttachTarget(fb);

    std::vector<ViewID> views = ViewManager::ListViews();
    for(uint32_t i = 0; i < views.size(); ++i) {
        ViewManager::Get(views[i])->AttachSource(fb);
    }


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


void Graphics::Flush2D() {
    drawBuffer->Render2DVertices(params2D);
}


/// statics ///





string fSearch(const string & file) {
    Filesys fs;
    string cd = fs.GetCWD();
    fs.ChangeDir(Engine::GetBaseDirectory());
    string fullPath = fs.FindFile(file);
    fs.ChangeDir(cd);
    return fullPath;
}
