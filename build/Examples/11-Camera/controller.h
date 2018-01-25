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

#include <Dynacoe/Library.h>

// Like in the previous example, controls the logic. here 
// it has been separated as its own class
class Controller : public Dynacoe::Entity {
  public:
    Dynacoe::Vector target;
    void OnStep() {


        Dynacoe::Camera & cam = Dynacoe::Graphics::GetCamera3D();

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_w)) {
            target.y -= .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_s)) {
            target.y += .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_a)) {
            target.x += .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_d)) {
            target.x -= .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_q)) {
            target.z -= .1f;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_e)) {
            target.z += .1f;
        }


        // ease towards position
        cam.node.local.position.x = Dynacoe::Mutator::StepTowards(cam.node.local.position.x, target.x, .1);
        cam.node.local.position.y = Dynacoe::Mutator::StepTowards(cam.node.local.position.y, target.y, .1);
        cam.node.local.position.z = Dynacoe::Mutator::StepTowards(cam.node.local.position.z, target.z, .1);

    }        
    
};