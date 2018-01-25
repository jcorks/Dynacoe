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


#ifndef H_HUD_INCLUDED
#define H_HUD_INCLUDED

/* displays the player's stats 
   without text */

// The idea is to have the HUD items react explicitly to certain events
// in a way that indicates its use, i.e. shaking the hp bar when hit,
// flashing the scoreboard with getting a kill, etc.

#include <Dynacoe/Entity.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/BuiltIn/FillBar.h>
#include <cmath>


class PowerBox : public Dynacoe::Entity {
  public:
    const float base_size = 10;

    PowerBox() {
        power = 0;
        speed = 0;
    }

    void Init() {
        std::vector<Dynacoe::Point> pts;
        pts.push_back(Dynacoe::Point(sqrt(2)*base_size, sqrt(2)*base_size));
        pts.push_back(Dynacoe::Point(-sqrt(2)*base_size, sqrt(2)*base_size));
        pts.push_back(Dynacoe::Point(0, -base_size));

        powerTri.FormTriangles(
            pts
        );
        powerTri.SetColor(Dynacoe::Color("red"));

        speedTri.FormTriangles(
            pts
        );
        speedTri.SetColor(Dynacoe::Color("blue"));

    
    }

    void PowerRatingUp() {
        power ++;
    }

    void SpeedRatingUp() {
        speed ++;
    }    


    void Run() {
    }

    void Draw() {
        powerTri.Rotate((power)*2);
        speedTri.Rotate((speed)*2);
        powerTri.SetPosition(Position+Dynacoe::Point(10, 0 ));
        speedTri.SetPosition(Position+Dynacoe::Point(10, 30));
        
        Dynacoe::Chain str;
        str << "POWER Lv" << 1+power;
        Dynacoe::Graphics::DrawString(str.c_str(), Position + Dynacoe::Point(40, 0), Dynacoe::Color("white"));
    
        str = "";
        str << "SPEED Lv" << 1+speed;
        Dynacoe::Graphics::DrawString(str.c_str(), Position + Dynacoe::Point(40, 30), Dynacoe::Color("white"));
    
        Dynacoe::Graphics::Draw(powerTri);
        Dynacoe::Graphics::Draw(speedTri);

    }

  private:
    int power;
    int speed;
    Dynacoe::AspectShape powerTri;
    Dynacoe::AspectShape speedTri;
    
};

class HUD : public Dynacoe::World {
  public:





    const float          bg_width  = 100;
    const float          bg_height = 40;


    const Dynacoe::Color bg_color    = Dynacoe::Color(32, 32, 32, 255);

    // flashes the console a certain color.
    // the console fades back into its default color afterward
    void Flash(const Dynacoe::Color & c) {
        color = c;
    }
    

    void PowerLevelUp() {
        PowerBox * p = powerBoxID.IdentifyAs<PowerBox>();
        p->PowerRatingUp();

    }
    void SpeedLevelUp() {
        PowerBox * p = powerBoxID.IdentifyAs<PowerBox>();
        p->SpeedRatingUp();
    }


    

    void SetHealthPercent(float fraction) {
        Dynacoe::FillBar * healthBar = healthBarID.IdentifyAs<Dynacoe::FillBar>();
        healthBar->SetFill(fraction);
    }

    void SetBoostPercent(float fraction) {
        Dynacoe::FillBar * boostBar = boostBarID.IdentifyAs<Dynacoe::FillBar>();
        boostBar->SetFill(fraction);
    }


    void SetFirePercent(float fraction) {
        Dynacoe::FillBar * fireBar = fireBarID.IdentifyAs<Dynacoe::FillBar>();
        fireBar->SetFill(fraction);
    }

  public:
    void Init() {

        SetPriority(-100);


        Dynacoe::FillBar * healthBar = Create<Dynacoe::FillBar>();

        healthBar->SetBorder(0);
        healthBar->SetDims(
            Dynacoe::Graphics::GetRenderResolutionWidth()/4,
            10
        );
        healthBar->SetColorFill  (Dynacoe::Color(0, 150, 0, 255));
        healthBar->SetColorBorder(Dynacoe::Color("white"));
        healthBar->SetColorEmpty (Dynacoe::Color(50, 0, 0, 255));
        healthBar->SetFill(1.f);
        healthBar->SetPriority(GetPriority()-1);
        healthBarID = healthBar->GetID();


        Dynacoe::FillBar * fireBar = Create<Dynacoe::FillBar>();
        fireBar->SetBorder(3);
        fireBar->SetDims(
            Dynacoe::Graphics::GetRenderResolutionWidth()/4,
            10
        );
        fireBar->SetColorFill  (Dynacoe::Color("black"));
        fireBar->SetColorBorder(Dynacoe::Color(128, 128, 128, 255));
        fireBar->SetColorEmpty (Dynacoe::Color(128, 128, 128, 255));
        fireBar->SetFill(1.f);
        fireBar->SetFillDirection(
            Dynacoe::FillBar::FillDirection::Left
        );
        fireBar->SetPriority(GetPriority()-1);
        fireBarID = fireBar->GetID();
            
        

        Dynacoe::FillBar * boostBar = Create<Dynacoe::FillBar>();
        boostBar->SetDims(5, bg_height-3);
        boostBar->SetFillDirection(
            Dynacoe::FillBar::FillDirection::Up
        );
        boostBar->SetColorFill(Dynacoe::Color(100, 150, 255, 255));
        boostBar->SetColorEmpty(Dynacoe::Color( 40, 40, 40, 255));
        fireBar->SetPriority(GetPriority()-1);
        boostBarID = boostBar->GetID();

        
          
        powerBoxID = Create<PowerBox>()->GetID();
        


    }




    void PreDraw() {

        color.r = Dynacoe::Mutator::StepTowards(color.r, bg_color.r, .05);
        color.g = Dynacoe::Mutator::StepTowards(color.g, bg_color.g, .05);
        color.b = Dynacoe::Mutator::StepTowards(color.b, bg_color.b, .05);

        Position = Dynacoe::Graphics::GetFlatPosition();    
        bg.SetColor(color);
        bg.FormRectangle(
            Dynacoe::Graphics::GetRenderResolutionWidth(), 
            bg_height
        );

        bg.SetPosition(
            Position + Dynacoe::Point(0, Dynacoe::Graphics::GetRenderResolutionHeight() - bg_height)
        );

        healthBarID.Identify()->Position = bg.GetRealPosition() + Dynacoe::Point(Dynacoe::Point(
            Dynacoe::Graphics::GetRenderResolutionWidth()/2,
            20
        ));

        fireBarID.Identify()->Position = bg.GetRealPosition() + Dynacoe::Point(Dynacoe::Point(
            Dynacoe::Graphics::GetRenderResolutionWidth()/2,
            0
        ));


        boostBarID.Identify()->Position = bg.GetRealPosition() + Dynacoe::Point(Dynacoe::Point(
            Dynacoe::Graphics::GetRenderResolutionWidth()/2 - 10,
            0
        ));

        Dynacoe::Graphics::Draw(bg);
        
        powerBoxID.Identify()->Position = (bg.GetRealPosition());




    }

    void Destruct() {
        Entity * hBar = healthBarID.Identify();
        hBar->Remove();

        hBar = boostBarID.Identify();
        hBar->Remove();

        hBar = fireBarID.Identify();
        hBar->Remove();

        hBar = powerBoxID.Identify();
        hBar->Remove();
        
    }

  private:



    Dynacoe::Entity::ID powerBoxID;
    Dynacoe::Entity::ID healthBarID;
    Dynacoe::Entity::ID boostBarID;
    Dynacoe::Entity::ID fireBarID;
    float healthFraction;


    Dynacoe::Color color;
    Dynacoe::AspectShape bg;




};


#endif
