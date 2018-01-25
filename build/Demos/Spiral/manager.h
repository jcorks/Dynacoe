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


#ifndef H_MANAGER_INCLUDED
#define H_MANAGER_INCLUDED 

#include <Dynacoe/Library.h>
#include <Dynacoe/Components/Mutator.h>
#include "player.h"

// Manages the demo.
//
// It really just manages the input, 
// the camera, and creates the player. 


class Manager : public Dynacoe::Entity {
  public:

    Manager() {



        // Lets set up input mappings. Input mappings introduce a mechanism
        // to easily specify alternate controls without changing any code.

        Dynacoe::Input::MapInput("playerUp"      , Dynacoe::Keyboard::Key_w);
        Dynacoe::Input::MapInput("playerDown"    , Dynacoe::Keyboard::Key_s);
        Dynacoe::Input::MapInput("playerLeft"    , Dynacoe::Keyboard::Key_a);
        Dynacoe::Input::MapInput("playerRight"   , Dynacoe::Keyboard::Key_d);


        Dynacoe::Input::MapInput("playerFaster"  , Dynacoe::Keyboard::Key_up);
        Dynacoe::Input::MapInput("playerSlower"  , Dynacoe::Keyboard::Key_down);
        Dynacoe::Input::MapInput("playerFastest" , Dynacoe::Keyboard::Key_right);
        Dynacoe::Input::MapInput("playerSlowest" , Dynacoe::Keyboard::Key_left);

        Dynacoe::Input::MapInput("playerMakeBit" , Dynacoe::Keyboard::Key_space);

    }



	void OnStep()  {
        // Excercise: can you find a better / more efficient way 
        // to access the player entity?
        auto results = Dynacoe::Engine::Root().Identify()->FindChildByName("Player");
        if (!results.size()) return;
        Entity * player = results[0].Identify();
        if (!player) return;
        


        cameraPos.x = Dynacoe::Mutator::StepTowards(
            cameraPos.x, 
            player->node.local.position.x,
            .2f, 
            Dynacoe::Mutator::Function::Quadratic
        );


        cameraPos.y = Dynacoe::Mutator::StepTowards(
            cameraPos.y, 
            player->node.local.position.y,
            .2f, 
            Dynacoe::Mutator::Function::Quadratic
        );

        Dynacoe::Graphics::GetCamera2D().node.local.position = cameraPos - Dynacoe::Point(
                    Dynacoe::Graphics::GetRenderCamera().Width() / 2, 
                    Dynacoe::Graphics::GetRenderCamera().Height() / 2
        );
    }





  private:

    Dynacoe::Point cameraPos;

    

};

#endif

