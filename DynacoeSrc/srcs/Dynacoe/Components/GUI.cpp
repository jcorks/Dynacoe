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


#include <Dynacoe/Components/GUI.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Modules/Input.h>
#include <Dynacoe/Components/Text2D.h>
#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Modules/Input.h>
#include <Dynacoe/Dynacoe.h>

using namespace Dynacoe;
GUI *       GUI::focused = nullptr;
GUI *       GUI::grabbed = nullptr;
static GUI * clicked     = nullptr;

static Entity::ID tooltipManager;


class TooltipManager : public Entity {
  public:
    TooltipManager() {


        shape = AddComponent<Shape2D>();
        text = AddComponent<Text2D>();

        draw = false;
        step = false;
        text->Node().Position() = {5, 5};

        shape->color = {0.f, 0.f, 0.f, 1.f};
        source = nullptr;
    }


    void OnStep() {
        Node().Position() = Graphics::GetCamera2D().Node().GetPosition() +
            Vector(Input::MouseX(), Input::MouseY()) +
            Vector(10, 10);

    }

    void SetText(const std::string & str) {
        text->text = str;
        shape->FormRectangle(text->GetDimensions().x+10, text->GetDimensions().y+10);


    }


    void SetOn(GUI * o) {
        source = o;
        draw = true;
        step = true;
    }

    void SetOff(GUI * o) {
        if (source != o) return;
        draw = false;
        step = false;

    }
  private:
      GUI * source;
      Text2D * text;
      Shape2D *  shape;

};

void GUI::Initialize() {
    w = 0;
    h = 0;
    hovered = false;

    InstallEvent("on-click");
    InstallEvent("on-enter");
    InstallEvent("on-leave");
    InstallEvent("on-hover");
    InstallEvent("on-drag");
    InstallEvent("on-focus");
    InstallEvent("on-unfocus");
    if (!tooltipManager.Valid()) {
        tooltipManager = Entity::Create<TooltipManager>();


        Engine::AttachManager(tooltipManager);
    }
}


GUI::GUI() : Component() {
    SetTag("GUI");
    Initialize();
}

GUI::~GUI() {
    if (IsFocused()) focused = nullptr;
}

void GUI::DefineRegion(int w_, int h_) {
    w = w_;
    h = h_;

}





bool GUI::IsFocused() {
    return focused == this;
}

bool GUI::IsHovered() {
    return hovered;
}



bool GUI::IsBeingDragged() {
    return IsHovered() && Input::IsHeld(MouseButtons::Left);
}



bool GUI::IsFocusedHovered() {
    if (!focused) return false;
    return focused->IsHovered();
}


void GUI::GrabInput() {
    grabbed = this;
}

void GUI::UngrabInput() {
    grabbed = nullptr;
}


void GUI::SetFocus() {
    if (focused == this) return;
    if (focused) {
        focused->Unfocus();
    }
    focused = this;
    EmitEvent("on-focus");
}

void GUI::Unfocus() {
    if (!focused) return;
    GUI * oldF = focused;
    focused = nullptr;
    oldF->EmitEvent("on-unfocus");
}

int GUI::GetRegionW() {
    return w;
}

int GUI::GetRegionH() {
    return h;
}









void GUI::OnStep() {
    if (grabbed && grabbed != this) return;

    Vector pt = {0, 0};
    pt = GetGlobalTransform().Transform(pt);
    bool newHovered = Math::IsVectorWithinBounds(
        Vector(Input::MouseX(), Input::MouseY()),
        pt, w-1, h-1
    );

    if (!hovered && newHovered) {
        EmitEvent("on-enter");
        if (tooltipText.size()) {
            tooltipManager.IdentifyAs<TooltipManager>()->SetText(tooltipText);
            tooltipManager.IdentifyAs<TooltipManager>()->SetOn(this);
        }
    }

    if (hovered && !newHovered) {
        EmitEvent("on-leave");
        tooltipManager.IdentifyAs<TooltipManager>()->SetOff(this);
    }

    hovered = newHovered;
    if (hovered) {
        EmitEvent("on-hover");
        if (Input::IsPressed(MouseButtons::Left)) {
            clicked = this;
        }
    
        if (Input::IsReleased(MouseButtons::Left)) {
            if (clicked == this) {
                EmitEvent("on-click");
            }
        }

        if (IsBeingDragged()) {
            EmitEvent("on-drag");
        }
    }
}


std::string GUI::GetInfo() {
    return (Chain() << "@" << Node().GetPosition() << " w/h:" << w << ", " << h << "\n"
                    << "Hovered? " << (IsHovered() ? "Yes" : "Nope") << "\n"
                    << "Focused? " << (IsFocused() ? "Yes" : "Nope") << "\n");
}

void GUI::SetTooltipText(const std::string & t) {
    tooltipText = t;
}

const std::string & GUI::GetTooltipText() const {
    return tooltipText;
}
