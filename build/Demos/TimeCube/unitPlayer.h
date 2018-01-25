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


#ifndef H_UNIT_PLAYER_INCLUDED
#define H_UNIT_PLAYER_INCLUDED

/* Shoots things. Powered by YOU! */

#include "unit.h"
#include "hud.h"
#include "view.h"
#include "health.h"
#include <Dynacoe/Library.h>
#include <Dynacoe/Util/Chain.h>


class UnitPlayer : public Unit {
  public:

    void Init() {
    
        Unit::Init();
        SetName("UnitPlayer");
        movePoint = Dynacoe::Point(0, 20);
        crosshair.FormLines(
            std::vector<Dynacoe::Point>{
                Dynacoe::Point(-5, 5),
                Dynacoe::Point( 5, 5),

                Dynacoe::Point( 5, 5),
                Dynacoe::Point( 5,-5),

                Dynacoe::Point( 5,-5),
                Dynacoe::Point(-5,-5),

                Dynacoe::Point(-5,-5),
                Dynacoe::Point(-5, 5),
            }
        );

        Entity & hud = *GetWorld().Create<HUD>();
        scoreID = hud.GetID();
        View::HUDify(hud);


        currentPower = 0;
        currentSpeed = 0;
        //moveThing.FormRectangle(4, 4);
        //moveThing.SetCenter(Dynacoe::Point(2, 2));
    }

    void Run() {
        Unit::Run();
        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_d)) {
            Dynacoe::Point ideal(20, 0);
        
            movePoint.x = Dynacoe::Mutator::StepTowards(movePoint.x, ideal.x, .04);
            movePoint.y = Dynacoe::Mutator::StepTowards(movePoint.y, ideal.y, .04);
            movePoint.SetToNormalize(); 
            movePoint.x *= 20;
            movePoint.y *= 20;
        
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_a)) {
            Dynacoe::Point ideal(-20, 0);
        
            movePoint.x = Dynacoe::Mutator::StepTowards(movePoint.x, ideal.x, .04);
            movePoint.y = Dynacoe::Mutator::StepTowards(movePoint.y, ideal.y, .04);
            movePoint.SetToNormalize(); 
            movePoint.x *= 20;
            movePoint.y *= 20;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_w)) {
            Dynacoe::Point ideal(0, -20);
        
            movePoint.x = Dynacoe::Mutator::StepTowards(movePoint.x, ideal.x, .04);
            movePoint.y = Dynacoe::Mutator::StepTowards(movePoint.y, ideal.y, .04);
            movePoint.SetToNormalize(); 
            movePoint.x *= 20;
            movePoint.y *= 20;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_s)) {
            Dynacoe::Point ideal(0, 20);
        
            movePoint.x = Dynacoe::Mutator::StepTowards(movePoint.x, ideal.x, .04);
            movePoint.y = Dynacoe::Mutator::StepTowards(movePoint.y, ideal.y, .04);
            movePoint.SetToNormalize(); 
            movePoint.x *= 20;
            movePoint.y *= 20;
        }


        Aim(Position + targetPoint);
        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_space) ||
            Dynacoe::Input::GetState(Dynacoe::MouseButtons::Left)) {
            Fire();
        }


        HUD * hud = scoreID.IdentifyAs<HUD>();
        hud->SetHealthPercent(
            QueryComponent<Health>()->GetHealthFraction()
        );

        hud->SetBoostPercent(
            QueryComponent<Booster>()->FuelLeft()
        );

        hud->SetFirePercent(
            QueryComponent<Turret>()->PercentTimeLeft()
        );
    

        /*    
        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_up)) {
            targetPoint.y -= 3;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_down)) {
            targetPoint.y += 3;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_left)) {
            targetPoint.x -= 3;
        }

        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_right)) {
            targetPoint.x += 3;
        }
        */


        if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_lshift)) 
            BoostOn();
        else
            BoostOff();



        targetPoint.x = Dynacoe::Input::MouseX() - Position.x;
        targetPoint.y = Dynacoe::Input::MouseY() - Position.y;



        GoTowards(Position + movePoint);



        
    }    



    void Draw() {
        Unit::Draw();

        crosshair.Rotate(Dynacoe::Input::GetState(Dynacoe::MouseButtons::Left) ? 3: 1);
        crosshair.ScaleTo(Dynacoe::Point(3, 3, 3));        
        //crosshair.SetPosition(Position + targetPoint);
        // Dynacoe::Graphics::Draw(crosshair);

        // target crosshair
        if (QueryComponent<Alignment>()) {
            auto ents = GetWorld().FindByName("UnitAI");
            for(int i = 0; i < ents.size(); ++i) {
                if (!ents[i].Identify()->QueryComponent<Alignment>()) continue;
                if (ents[i].Identify()->QueryComponent<Alignment>()->GetID() != 
                                         QueryComponent<Alignment>()->GetID()) {
                    crosshair.SetPosition(ents[i].Identify()->Position);
                    crosshair.SetColor(Dynacoe::Color("red"));
                    Dynacoe::Graphics::Draw(crosshair);
                }
            }
        }

        moveThing.SetColor(Dynacoe::Color(
            GetColor().r/2,
            GetColor().g/2,
            GetColor().b/2,
            255
        ));
        //moveThing.SetPosition(Position + movePoint);
        moveThing.FormLines({Position + movePoint/2, Position+movePoint});
        Dynacoe::Graphics::Draw(moveThing);
        


            


        // update score
        
        HUD & hud = *scoreID.IdentifyAs<HUD>();
        assert(&hud);
        if (currentPower != QueryComponent<Turret>()->Level()) {
            currentPower++;
            hud.Flash(Dynacoe::Color("red"));
            hud.PowerLevelUp();

            if (currentPower + currentSpeed > 4) {
                hud.Flash(Dynacoe::Color("yellow"));
            }
            if (currentPower + currentSpeed == 5) {
                PlayPowerSound();
            }

        }
        if (currentSpeed != QueryComponent<Booster>()->Level()) {
            currentSpeed++;
            hud.Flash(Dynacoe::Color("blue"));
            hud.SpeedLevelUp();
            if (currentPower + currentSpeed > 4) {
                hud.Flash(Dynacoe::Color("yellow"));
            }

            if (currentPower + currentSpeed == 5) {
                PlayPowerSound();
            }
        }


        // overdriver mode


        
    }





    void Destruct() {
        HUD & hud = *scoreID.IdentifyAs<HUD>();
        assert(&hud);
        hud.Remove();
    }

  private:

    void PlayPowerSound() {
        static Dynacoe::AssetID asset;
        if (!asset.Valid()) {
            asset = Dynacoe::Assets::Load("wav", "super.wav");
        }
        
        Listener::PlaySoundProximity(Position, asset);
    }


    Dynacoe::Point movePoint;
    Dynacoe::Point targetPoint;
    Dynacoe::AspectShape crosshair;
    Dynacoe::AspectShape moveThing;
    
    Dynacoe::Color  uiColor;
    Dynacoe::Entity::ID scoreID;

    int currentPower;
    int currentSpeed;
};

#endif
