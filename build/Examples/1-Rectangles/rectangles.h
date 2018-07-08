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


#ifndef RECTANGLES_H_INCLUDED
#define RECTANGLED_H_INCLUDED

#include <Dynacoe/Library.h>



// For convenience, the class has been implemented in the header.
// When created, this entity will draw 4 rectangles every frame.


class ExampleRectangles : public Dynacoe::Entity { // enherits from the base Entity
  public:
    Dynacoe::Shape2D shape;

    // Setting a name helps identify our instance during runtime
    ExampleRectangles() : Entity("ExRectangles") {
        AddComponent(&shape);

        // Since we aren't going to change the colors,
        // lets set their colors now.

        // Color objects can be created from RGB integers, or via
        // a string.
        shape.color = "cyan";


        float length = Dynacoe::Graphics::GetRenderCamera().Height()  / 2.0;

        shape.FormRectangle(length, length);

        node.Position() = {
            Dynacoe::Graphics::GetRenderCamera().Width()   / 2.f,
            Dynacoe::Graphics::GetRenderCamera().Height()  / 2.f
        };

        shape.node.Position() = {-length/2.f, -length/2.f};
        node.Rotation().z = 5;



    }



    void OnStep() {
        node.Rotation().z += .4;
        node.Rotation().x += .5;
        node.Rotation().y += .3;



    }



};

#endif // RECTANGLES_H_INCLUDED
