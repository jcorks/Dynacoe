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


#ifndef H_WHITEOUT_INCLUDED
#define H_WHITEOUT_INCLUDED

#include <Dynacoe/Entity.h>

/* Small effect showing a huge flash of light.
   Probably exlusivley for the Base explosion */




class Whiteout : public Dynacoe::Entity {
  public:
    const float effect_duration = 3.f;
    const float effect_width = 9000.f;
    const float effect_height = 4000.f;

    
    void Init() {
        aspect.FormRectangle(effect_width, 1);

        aspect.SetCenter(Dynacoe::Point(
            effect_width/2,
            .5
        ));


        aspect.SetColor(Dynacoe::Color("white"));
        

        expandController.Clear(1.f);
        expandController.NewMutation(
            effect_duration/3, 
            effect_height,
            Dynacoe::Mutator::Function::Quadratic
        );


        //expandController.NewMutation(e

        alphaController.Clear(1.f);
        alphaController.NewMutation(
            effect_duration, 
            0.f,    
            Dynacoe::Mutator::Function::Linear
        );



        expandController.Start();
        alphaController.Start();

    }


    void Run() {
        if (alphaController.Expired()) {
            Remove();
        }
    }

    void Draw() {
        aspect.SetPosition(Position);
        aspect.SetColor(Dynacoe::Color(
            255, 255, 255,
            255*alphaController.Value()
        ));

        aspect.ScaleTo(
            Dynacoe::Point(1.f, expandController.Value())
        );





        Dynacoe::Graphics::Draw(aspect);
    }
    


  private:
    Dynacoe::Mutator alphaController;
    Dynacoe::Mutator expandController;

    Dynacoe::AspectShape aspect; 
};



#endif
