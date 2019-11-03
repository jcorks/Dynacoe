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
#include <cmath>

using namespace Dynacoe;


class InputExample;




// This entity responds to mouse input and pushing the 
// WASD keys for movement.
class InputExample : public Entity {
  public:

    class InputDetector : public Dynacoe::InputListener {
      public:
        InputDetector(InputExample * ex) {
            object = ex;
        }


        void OnChange(int button, float val) {
            if (fabs(val) > .8) {
                object->lastInput = button;
                
                Console::Info() << "detected input" << button << "\n";
            }
        }
        InputExample * object;
    };


    class ControllerListener : public Dynacoe::InputListener {
      public:
        ControllerListener(InputExample * ex) {
            object = ex;
        }


        void OnPress(int button) {
        }

        void OnActive(int button, float val) {
            switch(button) {

                case UserInput::Pad_y:
                    object->Node().Position().y -= 5;
                    break;

                case UserInput::Pad_x:
                    object->Node().Position().x -= 5;
                    break;

                case UserInput::Pad_a:
                    object->Node().Position().y += 5;
                    break;

                case UserInput::Pad_b:
                    object->Node().Position().x += 5;
                    break;




                case UserInput::Pad_l:
                    object->Node().Position().x -= 4;
                    break;

                case UserInput::Pad_r:
                    object->Node().Position().x += 4;
                    break;

                case UserInput::Pad_l2:
                    object->Node().Position().x -= 4;
                    break;

                case UserInput::Pad_r2:
                    object->Node().Position().x += 4;
                    break;

                
            }
        }


        InputExample * object;

    };

    InputExample() {
        SetName("InputEx");
        mouseSquare = AddComponent<Shape2D>();


        // Let's center our Entity.
        Node().Position().x = ViewManager::GetViewWidth() / 2;
        Node().Position().y = ViewManager::GetViewHeight() / 2;


        mouseSquare->FormRectangle(4, 4);
        mouseSquare->color = "yellow";

        Input::AddPadListener(new ControllerListener(this), 0);
        Input::AddPadListener(new InputDetector(this), 0);

    }

    void OnStep() {
        // We're going to have our Entity move in response
        // to the keys W, A, S, and D

        // GetState() will return true if the button is down.
        // Using seperate if-statements allows us to have directional
        // movement


        Node().Position().x += Input::GetState(0, UserInput::Pad_axisX);
        Node().Position().y += Input::GetState(0, UserInput::Pad_axisY);



        float triggerL = Input::GetState(0, UserInput::Pad_axisL)+1;
        float triggerR = Input::GetState(0, UserInput::Pad_axisR)+1;

        if (triggerL > 0) Node().Position().x += triggerL*3;
        if (triggerR > 0) Node().Position().x -= triggerR*3;


        float leftX = Input::GetState(0, UserInput::Pad_axisX2);
        float leftY = Input::GetState(0, UserInput::Pad_axisY2);



        Node().Position().x += leftX*10;
        Node().Position().y += leftY*10;

        leftX = Input::GetState(0, UserInput::Pad_axisX3);
        leftY = Input::GetState(0, UserInput::Pad_axisY3);

        Node().Position().x += leftX*10;
        Node().Position().y += leftY*10;

        // Lets have the Entity teleport to where the mouse clicks.
        // IsPressed() will return true only when the press is registered.
        if (Input::GetState(UserInput::Pointer_0)) {
            Node().Position() = {
                Input::MouseX(),
                Input::MouseY()
            };

            Console::Info() << "I've moved to: " << Input::MouseX() << ", " << Input::MouseY() << "\n";
        }
    }

    int lastInput;
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

