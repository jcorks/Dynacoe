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

class CollideObject : public Dynacoe::Entity {
  public:
    CollideObject() {
        obj = AddComponent<Dynacoe::Object2D>();
        shape = AddComponent<Dynacoe::Shape2D>();

        shape->color = "red";
        obj->InstallHook("on-collide", OnCollide);
    }

    void Set(float w, float h) {
        obj->collider = 
            Dynacoe::Object2D::Collider({
                Dynacoe::Vector{0, 0},
                Dynacoe::Vector{w, 0},
                Dynacoe::Vector{w, h},
                Dynacoe::Vector{0, h}
            });
        ;

        shape->FormRectangle(w, h);

    }


    void OnDraw() {
        if (obj->collider.lastCollided.Valid()) {
            shape->color = "green";
            obj->collider.lastCollided = Dynacoe::Entity::ID();
        } else {
            shape->color = "grey";
        }
    }

    static DynacoeEvent(OnCollide) {
        CollideObject * e = self.IdentifyAs<CollideObject>();
        //e->shape.color.g += .1f;
        return true;
    }
    
    Dynacoe::Object2D * obj;
    Dynacoe::Shape2D * shape;
};

class Dumb : public Dynacoe::Entity {
  public:
    Dumb() {
        o2 = Dynacoe::Entity::CreateChild<CollideObject>();
        o2->Set(Dynacoe::Random::Value()*10 + 5, Dynacoe::Random::Value()*10 + 5);
        Node().Position() = {Dynacoe::Random::Value()*500, Dynacoe::Random::Value()*500};

    }

    void OnStep() {

        /*if (o2->Node().global.position.x < 0)
            o2->obj.AddVelocity(4, 0);
        else if (o2->Node().global.position.x > 200) 
            o2->obj.AddVelocity(-4, 0);*/
    }

    CollideObject * o2;
    
};

class InputExample : public Dynacoe::Entity {
  public:

    // A square to show where our little entity is
    Dynacoe::Shape2D * mouseSquare;


    InputExample() {
        SetName("InputEx");
        mouseSquare = AddComponent<Dynacoe::Shape2D>();


        // Let's center our Entity.
        Node().Position().x = Dynacoe::ViewManager::GetViewWidth() / 2;
        Node().Position().y = Dynacoe::ViewManager::GetViewHeight() / 2;


        mouseSquare->FormRectangle(4, 4);
        mouseSquare->Node().Position() = {-2, -2};
        mouseSquare->color = "yellow";

        CollideObject * o = Dynacoe::Entity::CreateChild<CollideObject>();
        o->Set(4, 4);
    }

    void OnStep() {
        // We're going to have our Entity move in response
        // to the keys W, A, S, and D

        // GetState() will return true if the button is down.
        // Using seperate if-statements allows us to have directional
        // movement

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_w)) {
            Node().Position().y -= 2;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_a)) {
            Node().Position().x -= 2;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_s)) {
            Node().Position().y += 2;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_d)) {
            Node().Position().x += 2;
        }


        // Lets have the Entity teleport to where the mouse clicks.
        // IsPressed() will return true only when the press is registered.
        if (Dynacoe::Input::IsPressed(Dynacoe::MouseButtons::Left)) {
            Node().Position() = {
                Dynacoe::Input::MouseX(),
                Dynacoe::Input::MouseY()
            };
        }
    }




};

#endif // H_INPUT_INCLUDED
