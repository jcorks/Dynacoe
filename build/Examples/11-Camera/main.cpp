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


#include <Dynacoe/Dynacoe.h>
#include "screen.h"
#include "shadercube.h"
#include "controller.h"

/* An extension of the shader example that shows how to use 
   multiple cameras and framebuffer rendering techniques.

   2016, Johnathan Corkery

*/



using namespace Dynacoe;

int main() {
    Engine::Startup();

    ViewManager::NewMain("Camera/Framebuffer Example");

    // Sets the new root 
    Engine::Root() = Entity::Create<Entity>();
    Entity * demo = Engine::Root().Identify();


    // First creates a controller, which allows the user to move the camer using WASDQE
    demo->CreateChild<Controller>();
    
    // Creates a "screen" that shows a rendered visual as a texture
    Screen * s = demo->CreateChild<Screen>();
    s->node.Position() = {0, 0, -5};    


    Engine::Run();
    return 0;
}

