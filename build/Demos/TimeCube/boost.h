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


#ifndef H_BOOST_INLCUDED
#define H_BOOST_INCLUDED

#include <Dynacoe/Entity.h>
#include <Dynacoe/Library.h>
// This would be most appropriate for a particle,
// but this demo is meant to not have any assets



class BoostEffect : public Dynacoe::Entity {
  public:
    const float max_size = 30.f;
    const float min_size = 20.f;
    const float duration = 1.f;



    BoostEffect() {
        AddComponent(&shape);
        sizeShift.Clear(
            Dynacoe::Random::Spread(
                min_size,
                max_size

            )
        );

        sizeShift.NewMutation(
            duration,
            0,
            Dynacoe::Mutator::Function::Logarithmic
        );


        blueShift.Clear(1.f);

        blueShift.NewMutation(
            duration,
            0.f,
            Dynacoe::Mutator::Function::Linear
        );

        shape.mode = Dynacoe::Shape2D::RenderMode::Translucent;
        blueShift.Start();
        sizeShift.Start();

    }

    void OnStep() {
        if (blueShift.Expired()) Remove();
    }

    void OnDraw() {
        float size = sizeShift.Value();
        shape.FormRectangle(size, size);
        shape.node.local.position = {size/2, size/2};

        shape.color = {
            blueShift.Value(),
            1.f, //blueShift.Value(),
            1.f,
            blueShift.Value()*.6f
        };
        node.local.rotation.z = Dynacoe::Random::Value()*360;

    }

  private:
    Dynacoe::Shape2D shape;
    Dynacoe::Mutator sizeShift;
    Dynacoe::Mutator blueShift;
};


#endif
