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

/*  A Basic example that shows how to draw simple 2D shapes
    2014, Johnathan Corkery
 */





#include <Dynacoe/Library.h>

using namespace Dynacoe;



// When created, this entity will draw and manipulate a 2D square.
// Note that 2D objects, while only positioned in 2D space are able to 
// rotate in 3D space.
//
// Renderings with 3 rotation do not use any perspective.
class ExampleRectangles : public Entity { // enherits from the base Entity
  public:


    // Setting a name helps identify our instance during runtime
    ExampleRectangles() : Entity("ExRectangles") { 


        // Attachs and creates the rectangle to the entity.
        // Components are attachments to entitys that allow them to do 
        // additional behavior. Shape2D is the basic component that 
        // renders all 2D shapes.
        shape = AddComponent<Shape2D>();

        // Since we aren't going to change the colors,
        // lets set it now.
        // Color objects can be created from RGB integers, or via
        // a string. Hexadecimal strings are also accepted.
        shape->color = "cyan";


        // It needs to have length, so lets make the square be
        // one half of the render resolution height. Keep in mind, link in most 
        // multimedia visual setups, the render resolution is not necessarily the window 
        // width and height.
        float length = Graphics::GetRenderCamera().Height()  / 2.0;


        // Actually changes the Shape2D component to be the square shape.
        shape->FormRectangle(length, length);


        // Sets the position of the whole entity itself. Since Shape2D is beign held 
        // by the Entity, this affects the visual position as well. 
        // The position is one half of width and height of the rendering resolution.
        Node().Position() = {
            Graphics::GetRenderCamera().Width()   / 2.f,
            Graphics::GetRenderCamera().Height()  / 2.f
        };


        // Sets the center of the rectangle to be its center rather than 
        // the default topleft.
        shape->Node().Position() = {-length/2.f, -length/2.f};


    }


    // OnStep is called every frame to update the Entity.
    void OnStep() {


        // Here the Entity's rotation is updated to change in all3 axis
        Node().Rotation().z += .4;
        Node().Rotation().x += .5;
        Node().Rotation().y += .3;



    }

  private:
    // A reference to the shape
    Shape2D * shape;


};





// For most of Dynacoe's examples, we will place the implementation within 
// a single file. The main contains just the driver code.
//
// Go to rectangles.h to see the functionality code.
int main() {

    // Initializes the engine. Should be called before 
    // using any Dynacoe feature.
    Engine::Startup();

    
    // Creates a new display/window with the given name "Rectangles"
    ViewManager::NewMain("Rectangles");

    // Creates a root entity that the game will update automatically
    Engine::Root() = Entity::Create<ExampleRectangles>();


    // Begin the main update loop.
    Engine::Run();

    return 0;
}
