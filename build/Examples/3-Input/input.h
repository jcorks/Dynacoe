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


#ifndef H_INPUT_INCLUDED
#define H_INPUT_INCLUDED

#include <Dynacoe/Library.h>

class InputExample : public Dynacoe::Entity {
  public:

    // A square to show where our little entity is
    Dynacoe::Shape2D mouseSquare;



    InputExample() {
        SetName("InputEx");


        // Let's center our Entity.
        node.Position().x = Dynacoe::ViewManager::GetViewWidth() / 2;
        node.Position().y = Dynacoe::ViewManager::GetViewHeight() / 2;


        mouseSquare.FormRectangle(4, 4);
        mouseSquare.node.Position() = {-2, -2};
        mouseSquare.color = "yellow";
        AddComponent(&mouseSquare);
    }

    void OnStep() {
        // We're going to have our Entity move in response
        // to the keys W, A, S, and D

        // GetState() will return true if the button is down.
        // Using seperate if-statements allows us to have directional
        // movement

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_w)) {
            node.Position().y -= 2;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_a)) {
            node.Position().x -= 2;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_s)) {
            node.Position().y += 2;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_d)) {
            node.Position().x += 2;
        }


        // Lets have the Entity teleport to where the mouse clicks.
        // IsPressed() will return true only when the press is registered.
        if (Dynacoe::Input::IsPressed(Dynacoe::MouseButtons::Left)) {
            node.Position() = {
                Dynacoe::Input::MouseX(),
                Dynacoe::Input::MouseY()
            };
        }
    }




};

#endif // H_INPUT_INCLUDED
