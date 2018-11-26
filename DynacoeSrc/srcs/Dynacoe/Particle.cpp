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


#include <Dynacoe/Particle.h>
#include <Dynacoe/Library.h>
#include <Dynacoe/Util/Math.h>
#include <cmath>

using namespace Dynacoe;


class EParticle : public Entity {
    public:
        Shape2D *  shape;

        EParticle();

        float x;
        float y;

        float xDelta;
        float yDelta;

        int counter;
        int frame;

        AssetID img;


        float duration;
        int alpha;
        float xScale;
        float yScale;
        float multiScale;
        float rotation;
        float direction;
        float speed;

        int red;
        int blue;
        int green;

        int alpha_delta;
        float xScale_delta;
        float yScale_delta;
        float multiScale_delta;
        float rotation_delta;
        float direction_delta;
        float speed_delta;

        int red_delta;
        int blue_delta;
        int green_delta;


        int layer;
        bool filter;
        bool paralax;

        bool destroy;
        void stepLogic();

        void OnStep() {
            if (destroy) {
                Remove();
            }

            if (counter++ >= duration) {
                destroy = true;
                return;
            }


            x += xDelta;
            y += yDelta;

            red += red_delta;
            blue += blue_delta;
            green += green_delta;

            alpha += alpha_delta;
            xScale += xScale_delta;
            yScale += yScale_delta;
            multiScale += multiScale_delta;
            rotation += rotation_delta;
            direction += direction_delta;

            speed += speed_delta;

            yDelta = sin(Math::Pi()/180 * -direction) * speed;
            xDelta = cos(Math::Pi()/180 * -direction) * speed;


            if (xScale < 0)
                xScale = 0;

            if (yScale < 0)
                yScale = 0;


            if (alpha < 0)
                alpha = 0;

            if (red < 0)
                red = 0;

            if (blue < 0)
                blue = 0;

            if (green < 0)
                green = 0;


            Vector center;
            if (img.Valid()) {
                Image & im = Assets::Get<Image>(img);
                if (frame > im.frames.size() - 1)
                    frame = 0;
                center = {(float)im.CurrentFrame().Width(), (float)im.CurrentFrame().Height()};
            }




            Node().Position() = {x, y};
            Node().Rotation().z = (rotation);
            shape->color = Color(red, green, blue, alpha);
            //imm.SetTranslucent(true);
            if (!img.Valid()) {
                shape->FormRectangle(125 * xScale * multiScale,
                                          125 * yScale * multiScale);
                shape->Node().Position() = -Vector(125 * xScale * multiScale,
                                          125 * yScale * multiScale)/2.f;


            } else {
                shape->Node().Position() = -(center/2);
                Node().Scale() = Vector(xScale * multiScale, yScale * multiScale, 1.f);
                shape->FormImageFrame(
                    img, frame
                );

            }
            frame++;
        }

};







EParticle::EParticle() {
        x = 0;
        y = 0;

        xDelta = 0;
        yDelta = 0;

        counter = 0;

        frame = 0;



        duration = 0;
        alpha = 0;
        xScale = 0;
        yScale = 0;
        multiScale = 0;
        rotation = 0;
        direction = 0;
        speed = 0;

        red = 0;
        blue = 0;
        green = 0;

        alpha_delta = 0;
        xScale_delta = 0;
        yScale_delta= 0;
        multiScale_delta = 0;
        rotation_delta = 0;
        direction_delta = 0;
        speed_delta = 0;

        red_delta = 0;
        blue_delta = 0;
        green_delta = 0;

        destroy = false;

        layer = 0;
        filter = true;
        paralax = true;

        shape = AddComponent<Shape2D>();


}



void ParticleEmitter2D::OnDraw() {


    if (!particleActiveList.size()) return;

    Renderer * drawBuffer = Graphics::GetRenderer();


    Renderer::Dimension d;
    Renderer::Polygon p;
    Renderer::AlphaRule a;

    drawBuffer->GetDrawingMode(&p, &d, &a);

    // dump all currently queued drawing operations
    Graphics::Flush2D();

    drawBuffer->SetDrawingMode(Renderer::Polygon::Triangle,
                               Renderer::Dimension::D_2D,
                               translucent ? Renderer::AlphaRule::Translucent : Renderer::AlphaRule::Allow);

    // particle drawing always uses bilinear filtering and
    // 'translucent' drawing. So we needed to dump the
    // drawings that uses a different state.
    Renderer::TexFilter prev = drawBuffer->GetTextureFilter();
    drawBuffer->SetTextureFilter(filter ?
        Renderer::TexFilter::Linear
      :
        Renderer::TexFilter::NoFilter);


    // queue all draw particle requests
    for(int i = 0; i < (int) particleActiveList.size(); i++) {
        if (!particleActiveList[i].Valid()) {
            particleActiveList.erase(i + particleActiveList.begin());
            i--;
            continue;
        }
        (particleActiveList[i].IdentifyAs<EParticle>())->Step();
        if (!particleActiveList[i].Valid()) {
            particleActiveList.erase(i + particleActiveList.begin());
            i--;
            continue;
        }
        (particleActiveList[i].IdentifyAs<EParticle>())->Draw();
    }

    Graphics::Flush2D();
    drawBuffer->SetDrawingMode(p, d, a);
    drawBuffer->SetTextureFilter(prev);

}


void ParticleEmitter2D::EnableFiltering(bool d) {
    filter = d;
}

void ParticleEmitter2D::EnableTranslucency(bool d) {
    translucent = d;
}









void ParticleEmitter2D::EmitParticle(AssetID i, int num) {
    if (!i.Valid()) {
        Console::Error()<<("[PARTICLE] Couldn't instantiate particle! Emit failed")<< Console::End();
        return;
    }
    Dynacoe::Vector v = GetGlobalTransform().Transform({});
    for(uint32_t n = 0; n < num; ++n) {
        EParticle * temp = instantiateParticle(&Assets::Get<Particle>(i));

        temp->x = v.x;
        temp->y = v.y;

        temp->Step();
        particleActiveList.push_back(temp->GetID());
    }
}



ParticleEmitter2D::ParticleEmitter2D() {
    filter = true;
    translucent = true;


}



EParticle * ParticleEmitter2D::instantiateParticle(const Particle * p) {

    EParticle * newP = Entity::CreateReference<EParticle>();
    newP->img = Assets::Query(Assets::Type::Image, p->Image_name.c_str());

    newP->xDelta = 0;
    newP->yDelta = 0;

    newP->counter = 0;


    newP->duration = Random::Spread(p->duration_min, p->duration_max);
    newP->alpha = Random::Spread(p->alpha_min, p->alpha_max);
    newP->xScale = Random::Spread(p->xScale_min, p->xScale_max);
    newP->yScale = Random::Spread(p->yScale_min, p->yScale_max);
    newP->multiScale = Random::Spread(p->multiScale_min, p->multiScale_max);
    newP->rotation = Random::Spread(p->rotation_min, p->rotation_max);
    newP->direction = Random::Spread(p->direction_min, p->direction_max);
    newP->speed = Random::Spread(p->speed_min, p->speed_max);

    newP->red = Random::Spread(p->red_min, p->red_max);
    newP->blue = Random::Spread(p->blue_min, p->blue_max);
    newP->green = Random::Spread(p->green_min, p->green_max);


    newP->alpha_delta = round(Random::Spread(p->alpha_delta_min, p->alpha_delta_max));
    newP->xScale_delta = Random::Spread(p->xScale_delta_min, p->xScale_delta_max);
    newP->yScale_delta = Random::Spread(p->yScale_delta_min, p->yScale_delta_max);
    newP->multiScale_delta = Random::Spread(p->multiScale_delta_min, p->multiScale_delta_max);
    newP->rotation_delta = Random::Spread(p->rotation_delta_min, p->rotation_delta_max);
    newP->direction_delta = Random::Spread(p->direction_delta_min, p->direction_delta_max);
    newP->speed_delta = Random::Spread(p->speed_delta_min, p->speed_delta_max);

    newP->red_delta = round(Random::Spread(p->red_delta_min, p->red_delta_max));
    newP->blue_delta = round(Random::Spread(p->blue_delta_min, p->blue_delta_max));
    newP->green_delta = round(Random::Spread(p->green_delta_min, p->green_delta_max));
    //newP->layer = Params::paralax_currentlayer;

    //newP->paralax = Params::paralax;
    newP->filter = filter;



    newP->destroy = false;


    return newP;
}


Particle::Particle(const std::string & n)  : Asset(n) {
        Image_name = "UNDEFINED";



        duration_max = 90;
        alpha_max = 255;
        xScale_max = 1;
        yScale_max = 1;
        multiScale_max = 1;
        rotation_max = 360;
        direction_max = 1;
        speed_max = 2;

        red_max = 255;
        blue_max = 255;
        green_max = 255;

        duration_min = 1;
        alpha_min = 0;
        xScale_min = 0;
        yScale_min = 0;
        multiScale_min = 1;
        rotation_min = 0;
        direction_min = 0;
        speed_min = 0;

        red_min = 0;
        blue_min = 0;
        green_min = 0;

        alpha_delta_min = -1;
        xScale_delta_min = 0;
        yScale_delta_min = 0;
        multiScale_delta_min = 0;
        rotation_delta_min = .01;
        direction_delta_min = 1;
        speed_delta_min = -.01;

        red_delta_min = 0;
        blue_delta_min = 0;
        green_delta_min = 0;

        alpha_delta_max = -1;
        xScale_delta_max = 0;
        yScale_delta_max = 0;
        multiScale_delta_max = 0;
        rotation_delta_max = .01;
        direction_delta_max = 1;
        speed_delta_max = -.01;

        red_delta_max = 0;
        blue_delta_max = 0;
        green_delta_max = 0;

}
