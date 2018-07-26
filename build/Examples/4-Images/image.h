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

#ifndef IMAGE_EXAMPLE_H_INCLUDED
#define IMAGE_EXAMPLE_H_INCLUDED

#include <Dynacoe/Library.h>
#include <Dynacoe/Components/Mutator.h>

class ImageExample : public Dynacoe::Entity {
  public:

    Dynacoe::Shape2D * image;

    // The aspect member holds the image we are going to draw.
    Dynacoe::AssetID id;

    ImageExample() {
        SetName("ImageExample");

        image = AddComponent<Dynacoe::Shape2D>();
        // First we need to load the the image into the program.
        // To do that, we use Assets::Load. The first argument specifies
        // what kind of file we are attempting to load, while the second should be the name of
        // the file to load. Each kind of Asset type has a number of file types that
        // are recognized. See Assets in the documentation for more information.

        // Immediately after loading the image, we call FormImage to transform
        // the imageAspect into a drawable rectangle showing the loaded image.


        id = Dynacoe::Assets::Load("png", "image.png");
        image->FormImage(id);

        Dynacoe::Image & imageReference = Dynacoe::Assets::Get<Dynacoe::Image>(id);

        image->node.Position() = {imageReference.CurrentFrame().Width(), imageReference.CurrentFrame().Height()};
        image->node.Position().x *= -.5;
        image->node.Position().y *= -.5;

    }

    void OnStep() {

        // We want to have the Image follow the mouse and appear to naturally drag behind its position.
        // To accomplish this effect, we will use the "setpTowards" function
        // found in the Mutator component. stepTowards slowly interpolates the
        // entity's position into the mouse position every step.
        //
        // Every Entity has a Position attribute that can be edited. It's often useful to use this
        // as the Entity's baseline position to draw from.

        node.Position().x = Dynacoe::Mutator::StepTowards(node.GetPosition().x, Dynacoe::Input::MouseX(), .1f);
        node.Position().y = Dynacoe::Mutator::StepTowards(node.GetPosition().y, Dynacoe::Input::MouseY(), .1f);

        // To make it more interesting, we will secretly rotate the image based on the
        // difference in position of the Mouse pointer's position.
        node.Rotation().z += Dynacoe::Input::MouseYDelta();


    }





};




#endif // IMAGE_EXAMPLE_H_INCLUDED
