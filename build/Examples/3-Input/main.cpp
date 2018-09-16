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





/*  An example that demonstrates
    input from the keyboard and mouse.


    2014, Johnathan Corkery
*/



#include <Dynacoe/Library.h>


using namespace Dynacoe;



// This entity responds to mouse input and pushing the 
// WASD keys for movement.
class InputExample : public Entity {
  public:

    InputExample() {
        SetName("InputEx");
        mouseSquare = AddComponent<Shape2D>();


        // Let's center our Entity.
        Node().Position().x = ViewManager::GetViewWidth() / 2;
        Node().Position().y = ViewManager::GetViewHeight() / 2;


        mouseSquare->FormRectangle(4, 4);
        mouseSquare->color = "yellow";
    }

    void OnStep() {
        // We're going to have our Entity move in response
        // to the keys W, A, S, and D

        // GetState() will return true if the button is down.
        // Using seperate if-statements allows us to have directional
        // movement

        if (Input::GetState(Keyboard::Key_w)) {
            Node().Position().y -= 2;
        }

        if (Input::GetState(Keyboard::Key_a)) {
            Node().Position().x -= 2;
        }

        if (Input::GetState(Keyboard::Key_s)) {
            Node().Position().y += 2;
        }

        if (Input::GetState(Keyboard::Key_d)) {
            Node().Position().x += 2;
        }


        // Lets have the Entity teleport to where the mouse clicks.
        // IsPressed() will return true only when the press is registered.
        if (Input::IsPressed(MouseButtons::Left)) {
            Node().Position() = {
                Input::MouseX(),
                Input::MouseY()
            };

            Console::Info() << "I've moved to: " << Input::MouseX() << ", " << Input::MouseY() << "\n";
        }
    }


  private:
    // A square to show where our little entity is
    Shape2D * mouseSquare;

};




// As in the first 2 examples, this is just the driver code.
int main() {
    Engine::Startup();    
    ViewManager::NewMain("Input Example");


    Engine::Root() = Entity::Create<InputExample>();
    Engine::Run(); 
    
    return 0;
}

