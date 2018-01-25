/*

Copyright (c) 2016, Johnathan Corkery. (jcorkery@umich.edu)
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

#ifndef H_MENU_INCLUDED
#define H_MENU_INCLUDED

#include <Dynacoe/Library.h>
#include <Dynacoe/Components/GUI.h>
#include <Dynacoe/Components/Scheduler.h>



typedef void (*OptionCallback)();

class MenuButton : public Dynacoe::Entity {

    const Dynacoe::Color hoverBG;
    const Dynacoe::Color hoverText;


    const Dynacoe::Color normalBG;
    const Dynacoe::Color normalText;

    const int width   = 260;
    const int height  = 50;

  public:

    MenuButton() : 
        hoverBG   (240, 240, 240, 255),
        hoverText (20, 20, 20, 255),

        normalBG  (20, 20, 20, 255),
        normalText(240, 240, 240, 255)
        
        {}



    void Setup(const std::string & str, OptionCallback cb) {
        text.SetFont(Dynacoe::Assets::Load("ttf", "uni.ttf"));
        text.SetFontSize(20);
        text.SetText(str.c_str());
        text.SetCenter(text.GetDimensions()/2.f);
        optCB = cb;
    }


    void MoveTo(const Dynacoe::Point & p) {
        target = p;
    }

    void RemoveLater() {
        Attach(&schedule);
        schedule.StartTask("remove-self", 1000, Task_RemoveSelf);
    }

    void Init() {
        Attach(&gui);
        
        bg.FormRectangle(width, height);
        bg.SetColor(normalBG);
        bg.SetCenter(Dynacoe::Point(width, height)/2.f);
        gui.DefineRegion(-Dynacoe::Point(width, height)/2.f, width, height);

        SetPriority(-1);
    }

    void Run() {

        Position = Dynacoe::Mutator::StepTowards(Position, target, .2);

        if (gui.IsHovered()) {
            textColor.r = Dynacoe::Mutator::StepTowards(textColor.r, hoverText.r, .2);
            textColor.g = Dynacoe::Mutator::StepTowards(textColor.g, hoverText.g, .2);
            textColor.b = Dynacoe::Mutator::StepTowards(textColor.b, hoverText.b, .2);

            bgColor.r = Dynacoe::Mutator::StepTowards(bgColor.r, hoverBG.r, .2);
            bgColor.g = Dynacoe::Mutator::StepTowards(bgColor.g, hoverBG.g, .2);
            bgColor.b = Dynacoe::Mutator::StepTowards(bgColor.b, hoverBG.b, .2);
        } else {
            textColor.r = Dynacoe::Mutator::StepTowards(textColor.r, normalText.r, .2);
            textColor.g = Dynacoe::Mutator::StepTowards(textColor.g, normalText.g, .2);
            textColor.b = Dynacoe::Mutator::StepTowards(textColor.b, normalText.b, .2);

            bgColor.r = Dynacoe::Mutator::StepTowards(bgColor.r, normalBG.r, .2);
            bgColor.g = Dynacoe::Mutator::StepTowards(bgColor.g, normalBG.g, .2);
            bgColor.b = Dynacoe::Mutator::StepTowards(bgColor.b, normalBG.b, .2);
        }

        if (gui.IsClicked() && optCB) {
            optCB();
        }

        bg.SetPosition(Position);
        bg.SetColor(bgColor);
        gui.SetPosition(Position);
        text.SetTextColor(0, 255, textColor);
        text.SetPosition(Position);
    }

    void Draw() {
        Dynacoe::Graphics::Draw(bg);
        Dynacoe::Graphics::Draw(text);
    }


    static bool Task_RemoveSelf(Entity::ID id) {
        id.Identify()->Remove();
        return false;
    }


  private:


    Dynacoe::AspectText text;
    Dynacoe::AspectShape bg;
    Dynacoe::GUI gui;

    Dynacoe::Color textColor;
    Dynacoe::Color bgColor;
    Dynacoe::Point target;

    Dynacoe::Scheduler schedule;
    OptionCallback optCB;
};


class Menu : public Dynacoe::Entity {
  public:


    void SetTitle(const std::string & str) {
        titleText.SetFont(font);
        titleText.SetFontSize(40);


    }

    
    void SetOptions(std::vector<std::pair<std::string, OptionCallback>> opts) {
        for(int i = 0; i < buttons.size(); ++i) {
            if (buttons[i].Valid()) {
                buttons[i].IdentifyAs<MenuButton>()->RemoveLater();
                buttons[i].IdentifyAs<MenuButton>()->MoveTo(Position+
                    Dynacoe::Point(
                        2*Dynacoe::Graphics::GetRenderResolutionWidth(),
                          Dynacoe::Graphics::GetRenderResolutionHeight()/2                    
                    )
                );
            }
        }
        buttons.clear();

        MenuButton * button;

        Dynacoe::Point origin = 
            Dynacoe::Point(
                Dynacoe::Graphics::GetRenderResolutionWidth(),
                0
            )/2 + Position;
        float offset = Dynacoe::Graphics::GetRenderResolutionHeight()/(opts.size()+1); 

        for(int i = 0; i < opts.size(); ++i) {
            button = GetWorld().Create<MenuButton>();

            button->Position = Position + Dynacoe::Point( 
                -Dynacoe::Graphics::GetRenderResolutionWidth(),
                 Dynacoe::Graphics::GetRenderResolutionHeight()/2
            );

            button->Setup(opts[i].first, opts[i].second);
            button->MoveTo(origin + Dynacoe::Point(0, offset*(i+1)));
            buttons.push_back(button->GetID());
        }
    }


   

  
    void Draw() {
        //titleText.SetPosition(Dynacoe
        //Dynacoe::Graphics::Draw(titleText);
        
    }



    void Init() {
        font = Dynacoe::Assets::Load("ttf", "uni.ttf"); 
        SetTitle("~Time Cube~");
    }
  private:

    Dynacoe::AssetID font;
    Dynacoe::AspectText titleText;
    

    std::vector<Entity::ID> buttons;
    

};

#endif
