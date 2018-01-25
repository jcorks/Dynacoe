/*

Copyright (c) 2016, Johnathan Corkery. (jcorkery@umich.edu)
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


#ifndef H_EXP_INCLUDED
#define H_EXP_INCLUDED

#include "listener.h"
#include <Dynacoe/Library.h>


/* boom boom */



// big explosion
class Explosion : public Dynacoe::ParticleEmitter2D {
  public:

    const float exp_min_init_size = 10.f/3;
    const float exp_max_init_size = 20.f/3;
    const int   exp_num_shards = 30;
    const float exp_end_size = 50.f/3;
    const float exp_duration = 1.f;

    Dynacoe::AssetID particleSrc;

    Explosion() {
	emitted = false;
	AddComponent(&base);
        particleSrc = Dynacoe::Assets::Query(
            Dynacoe::Assets::Type::Particle,
            "exp.part"
        );

        // Our particle isn't loaded yet, so lets get it
        if (!particleSrc.Valid()) {
            particleSrc = Dynacoe::Assets::Load("part", "exp.part");
        }




        SetName("Explosion");

        // base of the explosion is just a centered rectangle
        base.FormRectangle(4, 4);
        base.node.local.position = -(Dynacoe::Point(2, 2));

        // Make the explosion start to grow fast, then slow down
        mutateSize.Clear(Dynacoe::Random::Spread(exp_min_init_size, exp_max_init_size));
        mutateSize.NewMutation(
            exp_duration,
            exp_end_size,
            Dynacoe::Mutator::Function::Logarithmic
        );
        mutateSize.Start();

        // Decrease alpha slowly then ramp it up.
        // should be inverse of the size mutation transform
        mutateAlpha.Clear(1.f);
        mutateAlpha.NewMutation(
            1.f,
            0.f,
            Dynacoe::Mutator::Function::Quadratic
        );
        mutateAlpha.Start();
        PlaySound();
    }

    void OnStep() {
        if (!emitted) {
            emitted = true;
            ParticleEmitter2D::EmitParticle(particleSrc, exp_num_shards);
        }
        ParticleEmitter2D::OnStep();
        // If invisible, thats a good time to excuse ourselves
        if (mutateAlpha.Expired()) {
            Remove();
        }
    }


    void OnDraw() {
        ParticleEmitter2D::OnDraw();
        float scale = mutateSize.Value();

        node.local.scale = {scale, scale};
        base.color = (
            Dynacoe::Color(
                1.f,
                Dynacoe::Random::Value(),
                0.f,
                mutateAlpha.Value()
            )
        );
        Dynacoe::Graphics::Draw(base);
    }


  private:
    void PlaySound() {
        static Dynacoe::AssetID asset;
        if (!asset.Valid()) {
            asset = Dynacoe::Assets::Load("wav", "exp.wav");
        }

        Listener::PlaySoundProximity(node.global.position, asset);
    }
    bool emitted;
    Dynacoe::Shape2D base;
    Dynacoe::Mutator mutateSize;
    Dynacoe::Mutator mutateAlpha;
};




#endif
