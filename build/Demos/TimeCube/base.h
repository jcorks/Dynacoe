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


#ifndef H_BASE_INCLUDED
#define H_BASE_INCLUDED

#include "alignment.h"
#include "health.h"
#include "unitAI.h"
#include "manager.h"
#include "whiteout.h"
#include "targeter.h"
#include <Dynacoe/Library.h>
#include <Dynacoe/Components/Object2D.h>


/* Creates units */

class Base : public Dynacoe::Entity {
  public:

    void SetDeathCallback(Dynacoe::Entity::Event evt) {
        deathCB = evt;
    }

  private:
    // states n' signals 

    Dynacoe::Point deathPosOld;
    static void OnDeath(Entity::ID id, Entity::ID killer) {
        Base & ent = *id.IdentifyAs<Base>();
        ent.RemoveComponent(&ent.health);
        ent.RemoveComponent(&ent.faction);
        ent.deathTime.Reset(3000);
        ent.deathPosOld = ent.Position;
        ent.states.Halt();
        ent.states.Execute("Death");

        if(ent.deathCB) ent.deathCB(id);
    }

    static void OnOuch(Entity::ID e) {
        Base & ent = *e.IdentifyAs<Base>();
        ent.aspect.SetColor(Dynacoe::Color("white"));
    }










    static std::string State_Normal(Dynacoe::Entity::ID id) {
        Base & base = *id.IdentifyAs<Base>();

        // spawn normal units
        if (base.spawnTime.IsExpired()) {
            UnitAI * newUnit = base.GetWorld().Create<UnitAI>(base.Position);
            newUnit->SetBase(base.GetID());
            base.spawnTime.Reset(base.spawnRate*1000 + Dynacoe::Random::Spread(100, 400));
        }


        // Control the mortar
        
        Entity * target = 
            base.targeter.GetCurrentTarget().Identify();
        
        if (target && IsTarget(base, *target) && !base.targetTimer.IsExpired()) {
            base.mortar.AimAt(target->Position);
            base.mortar.Fire(base.QueryComponent<Alignment>()->GetID());
        } else {
            base.targetTimer.Reset(400);
            base.targeter.FindNewTarget();
        }
        

        return "";
    }





    bool deathMadeWhiteout;
    static std::string State_Death(Dynacoe::Entity::ID e) {
        Base & ent = *e.IdentifyAs<Base>();
        
        // Shake it around the original position.
        ent.Position = ent.deathPosOld;
        ent.Position.x += Dynacoe::Random::Spread(-5, 5);
        ent.Position.y += Dynacoe::Random::Spread(-5, 5);


        if (Dynacoe::Random::Value() > .9) {
            ent.GetWorld().Create<Explosion>(
                ent.Position + Dynacoe::Point(
                    Dynacoe::Random::Spread(-50, 50),
                    Dynacoe::Random::Spread(-50, 50)
                )
            );
            ent.aspect.SetColor(Dynacoe::Color("white"));
        }

        if ((ent.deathTime.GetTimeLeft() < 1000) && !ent.deathMadeWhiteout) {
            ent.GetWorld().Create<Whiteout>(ent.Position);
            ent.deathMadeWhiteout = true;
        }


        if (ent.deathTime.IsExpired()) {
            ent.Remove();
            return "null";
        }
        return "";
    }


    // "oddly familiar" criterion for enemies of the base.
    static bool IsTarget(Entity & host, Entity & e) {
        assert(&host && &e);
        return  
               &e != &host &&
               e.Position.Distance(host.Position) < 300 &&
               e.QueryComponent<Alignment>() && 
                   e   .QueryComponent<Alignment>()->GetID() != 
                   host.QueryComponent<Alignment>()->GetID() &&
               e.QueryComponent<Health>();
    }

  public:
    
    // core logic

    void Init() {
        SetName("Base");
        Attach(&faction);
        Attach(&health);
        Attach(&obj);
        Attach(&states);
        Attach(&mortar);
        Attach(&targeter);

        SetPriority(-10);

        deathMadeWhiteout = false;
        mortar.Setup(4, 3, .1);
        targeter.SetTargetCriterion(IsTarget); 
        targetTimer.Reset(1000);

        

        health.SetHealth(10000);
        health.SetDamageCallback(OnOuch);
        health.SetDeathCallback(OnDeath);



        aspect.FormRectangle(20, 60);
        aspect.SetCenter(Dynacoe::Point(10, 30));
        aspect.SetColor(faction.Color() * .5);
        color = aspect.GetColor();        

        obj.AddContactBox(Dynacoe::Point(-10, -30), 20, 60);
        obj.SeekCollisions(false);

        spawnRate = 2.2f;
        spawnTime.Reset(spawnRate*1000);

        states.CreateState("Normal", Dynacoe::StateControl::StateLoop(State_Normal));
        states.CreateState("Death",  Dynacoe::StateControl::StateLoop(State_Death));
        states.Execute("Normal");
        
    }


    void Run() {

        // Ease the displayed color to the target color.
        // Mostly applicable only fading back from the hit color
        Dynacoe::Color cur = aspect.GetColor();
        cur.r = Dynacoe::Mutator::StepTowards(cur.r, color.r, .3f); 
        cur.g = Dynacoe::Mutator::StepTowards(cur.g, color.g, .3f); 
        cur.b = Dynacoe::Mutator::StepTowards(cur.b, color.b, .3f); 
        aspect.SetColor(cur);
    }

    void Draw() {
        aspect.SetPosition(Position);
        Dynacoe::Graphics::Draw(aspect);
    }


  private:

    Alignment faction;
    Health    health;
    
    Turret   mortar;
    Targeter targeter;


    Dynacoe::Timer  deathTime;
    Dynacoe::Point  origPos;
    Dynacoe::Color  color;
    Dynacoe::StateControl states;
    Dynacoe::Object2D obj;
    Dynacoe::AspectShape aspect;
    Dynacoe::Timer    spawnTime;
    Dynacoe::Timer    targetTimer;
    
    Dynacoe::Entity::Event deathCB;
    float spawnRate;


};


#endif
