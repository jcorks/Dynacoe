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


#ifndef H_BACKGROUND_INCLUDED
#define H_BACKGROUND_INCLUDED

#include <Dynacoe/Entity.h>
#include <Dynacoe/Parallaxizer.h>

/* the final frontier */
class Space : public Dynacoe::Entity {
  public:
    const float max_parallax       = 10.f;
    const Dynacoe::Point min_range = Dynacoe::Point (-1000, -1000, 3.f);
    const Dynacoe::Point max_range = Dynacoe::Point ( 1000,  1000, 7.f);

    const int   num_stars       = 200.f;

    // where 1.0 is full white
    const float min_star_brightness = .1;
    const float max_star_brightness = .3;



    void Init() {
        Dynacoe::Parallaxizer::SetRange(max_parallax);

        stars.  resize(num_stars);
        origPos.resize(num_stars);

        float brightness;
        for(int i = 0; i < num_stars; ++i) {
            origPos[i] = Dynacoe::Point(
                Dynacoe::Random::Spread(min_range.x, max_range.x),
                Dynacoe::Random::Spread(min_range.y, max_range.y),
                Dynacoe::Random::Spread(min_range.z, max_range.z)
            );

            brightness = Dynacoe::Random::Spread(
                min_star_brightness,
                max_star_brightness
            );

            stars[i].FormRectangle(2, 2);
            
            stars[i].SetColor(Dynacoe::Color( 
                255*brightness,
                255*brightness,
                255*brightness,
                255
            ));
        }

    }


    void Draw() {


        for(int i = 0; i < num_stars; ++i) {
            
            stars[i].SetPosition(
                Dynacoe::Parallaxizer::Shift(origPos[i])
            );
            Dynacoe::Graphics::Draw(stars[i]);
            

        }
    }


  private:
    std::vector<Dynacoe::AspectShape> stars;
    std::vector<Dynacoe::Point> origPos;
};


#endif
