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
    input from a connected controller


    2019, Johnathan Corkery
*/


// TODO: needs cleanup. This is just a crude sample.


#include <Dynacoe/Library.h>
#include <cmath>

using namespace Dynacoe;


class InputExample;
class MapInput;
static MapInput * active = nullptr;

class MapInput : public Dynacoe::Entity {
  public:


    
    static DynacoeEvent(onClick) {

        // reset others
        if (active && active->control->GetCurrentState() == "select") {
            active->control->Execute("idle");
        }

        // enter the input select state
        MapInput * m = self.IdentifyAs<MapInput>();
        m->control->Execute("select");
        active = m;
        m->button = 0;
        return true;
    };

    static DynacoeEvent(idle_start) {
        MapInput * m = self.IdentifyAs<MapInput>();
        m->bgButton->color = "blue";
        if (active == m) active = nullptr;
        return true;
    }
    static DynacoeEvent(idle_step) {
        return true;
    };


    static DynacoeEvent(select_start) {
        MapInput * m = self.IdentifyAs<MapInput>();
        m->bgButton->color = "red";
        return true;
    }

    
    static DynacoeEvent(select_step) {
        return true;
        
    };


    MapInput() {
        button = 0;

        gui = AddComponent<GUI>();
        bgButton = AddComponent<Shape2D>();
        key = AddComponent<Text2D>();
        key->text = "press me!";

        inputDisplay = AddComponent<Shape2D>();
        control = AddComponent<StateControl>();

        control->CreateState("idle", {idle_step, idle_start});
        control->CreateState("select", {select_step, select_start});

        gui->DefineRegion(70, 70);
        bgButton->FormRectangle(70, 70);
        bgButton->color = "blue";




        gui->InstallHook("on-click", onClick);
        
    }

    void SetLastInput(int b) {
        if (active == this) {
            button = b;
            key->text = Chain() << buttonName << ":" << Dynacoe::InputManager::IDToString(button);
            Input::MapInput(buttonName, 0, button);
            active->control->Execute("idle");
        }
    }

    void SetButtonName(const std::string & str) {
        buttonName = str;
        key->text = Chain() << str << ": press me!";
    }




  private:
    
    int button;
    std::string buttonName;
    StateControl * control;
    GUI * gui;
    Shape2D * bgButton;
    Shape2D * inputDisplay;
    Text2D * key;


};

class ControllerDetector : public Dynacoe::InputListener {
  public:


    void OnActive(int button, float val) {
        
        if (active && fabs(val) > .5) { // for sure activated!!
            active->SetLastInput(button);
        }            
    }
};



class ControllerExample : public Entity {
  public:

    class MappedListenerUp : public Dynacoe::InputListener {
      public:
        MappedListenerUp(ControllerExample * ex) {
            ent = ex;
        }
        void OnActive(int, float val) {
            ent->Node().Position().y -= val*5;
        }

        ControllerExample * ent;
    };

    class MappedListenerDown : public Dynacoe::InputListener {
      public:
        MappedListenerDown(ControllerExample * ex) {
            ent = ex;
        }
        void OnActive(int, float val) {
            ent->Node().Position().y += val*5;
        }

        ControllerExample * ent;
    };
    class MappedListenerLeft : public Dynacoe::InputListener {
      public:
        MappedListenerLeft(ControllerExample * ex) {
            ent = ex;
        }
        void OnActive(int, float val) {
            ent->Node().Position().x -= val*5;
        }

        ControllerExample * ent;
    };

    class MappedListenerRight : public Dynacoe::InputListener {
      public:
        MappedListenerRight(ControllerExample * ex) {
            ent = ex;
        }
        void OnActive(int, float val) {
            ent->Node().Position().x += val*5;
        }

        ControllerExample * ent;
    };


    ControllerExample() {
        SetName("ControllerExample");
        mouseSquare = AddComponent<Shape2D>();


        // Let's center our Entity.
        Node().Position().x = ViewManager::GetViewWidth() / 2;
        Node().Position().y = ViewManager::GetViewHeight() / 2;


        mouseSquare->FormRectangle(4, 4);
        mouseSquare->color = "yellow";
    
        Input::AddMappedListener(new MappedListenerUp(this), "up");
        Input::AddMappedListener(new MappedListenerDown(this), "down");
        Input::AddMappedListener(new MappedListenerLeft(this), "left");
        Input::AddMappedListener(new MappedListenerRight(this), "right");

    }

    int lastInput;  
  private:
    // A square to show where our little entity is
    Shape2D * mouseSquare;

};





int main() {
    Engine::Startup();    
    ViewManager::NewMain("Input Example");


    Engine::Root() = Entity::Create();

    MapInput * up = Engine::Root().Identify()->CreateChild<MapInput>();
    MapInput * down = Engine::Root().Identify()->CreateChild<MapInput>();
    MapInput * left = Engine::Root().Identify()->CreateChild<MapInput>();
    MapInput * right = Engine::Root().Identify()->CreateChild<MapInput>();


    up->SetButtonName("up");
    up->Node().Position() = {80, 0};

    down->SetButtonName("down");
    down->Node().Position() = {80, 160};

    left->SetButtonName("left");
    left->Node().Position() = {0, 80};

    right->SetButtonName("right");
    right->Node().Position() = {170, 80};


    Engine::Root().Identify()->CreateChild<ControllerExample>();

    Input::AddPadListener(new ControllerDetector, 0);

    Engine::Run(); 
    
    return 0;
}

