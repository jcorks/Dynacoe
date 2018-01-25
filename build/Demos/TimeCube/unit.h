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


#ifndef H_UNIT_INCLUDED
#define H_UNIT_INCLUDED

#include "upgrade.h"
#include "bullet.h"
#include "health.h"
#include "exp.h"
#include "turret.h"
#include "booster.h"
#include "boost.h"

#include <Dynacoe/Library.h>

/* shoots things */






class Unit : public Dynacoe::Entity {

  public:
    // Basic control set overview

    // Registers the base and sets the alignment
    void SetBase(Dynacoe::Entity::ID b) {
        baseID = b;
        faction.Change(
            b.Identify()->QueryComponent<Alignment>()->GetID()
        );
        shape.color = faction.Color();
        shape.FormLines(faction.GetShape());
        state.Execute("Normal");
    }

    // Move towards a point
    void GoTowards(const Dynacoe::Point & targetPos) {
        float speed = obj.GetSpeed();
        obj.AddVelocity(turnAmount, Position.RotationZDiffRelative(targetPos));
        obj.SetSpeed(speed);
    }


    void Aim(const Dynacoe::Point & tPos) {
        turret.AimAt(tPos);
    }
    // Fire the weapon (see Turret.h for the mechanics)
    void Fire() {
        turret.Fire(faction.GetID());
    }


    // Attempts to make the unit go faster
    // in the direction that it is already going.
    void BoostOn() {
        booster.Activate(true);
    }

    void BoostOff() {
        booster.Activate(false);
    }

    // returns the set faction
    Alignment::ID GetFaction() {
        return faction.GetID();
    }

    // returns the color;
    Dynacoe::Color GetColor() {
        return shape.color;
    }


    // returns the baseID
    Dynacoe::Entity::ID GetBase() {
        return baseID;
    }


    // Returns whether or not the unit died
    bool IsDead() {
        return died;
    }














  private:
    // States of the unit



    const float min_speed = 2.0f;
    const float max_speed = 2.4f;
   // const float unit_turn_amount = .13f;
    const float unit_turn_amount = .11f;

    const float min_refire_rate = .6f;
    const float max_refire_rate = .9f;

    const float min_burst_rate = .1f;
    const float max_burst_rate = .15f;

    const float min_burst_duration = .5f;
    const float max_burst_duration = .8f;

    const float unit_size = 10.f;
    const float death_duration_ms = 1000;










    // Death animation.
    static DynacoeEvent(State_Dying) {
        Unit & unit = *self.IdentifyAs<Unit>();

        // Flashing and jarring movement are akin to an spacecraft losing control
        unit.local.position.x += Dynacoe::Random::Spread(-4, 4);
        unit.local.position.y += Dynacoe::Random::Spread(-4, 4);

        unit.shape.color = {
            Dynacoe::Random::Value(),
            Dynacoe::Random::Value(),
            Dynacoe::Random::Value(),
            1.f
        };


        // RIP
        if (unit.dead.IsExpired()) {
            Dynacoe::Engine::Root().Identify()->Create<Explosion>();

            Upgrade * upgrade = unit.GetWorld().Create<Upgrade>();
            upgrade->MoveTo(unit.Position);

            // if a unit has a total of level 4 or higher in upgrades,
            // the dropped upgrade carries over the stats
            // HEAVEN OR HELL: RETS ROCK
            if (unit.turret.Level() + unit.booster.Level() > 4) {
                upgrade->MakeSpecial(
                    unit.turret.Level(),
                    unit.booster.Level()
                );
            }
            upgrade->GiveTo(unit.killerID);


            unit.Remove();

            unit.state.Execute("Null");
        }
        return true;
    }





    // Deactivates the unit and begins the dying animation
    static DynacoeEvent(Event_OutOfHealth) {
        Unit & unit = *id.IdentifyAs<Unit>();
        unit.dead.Reset(unit.death_duration_ms);
        unit.RemoveComponent(unit.QueryComponent<Alignment>());
        // I think it's fair to let them still fire while dying
        //unit.RemoveComponent(unit.QueryComponent<Turret>());
        unit.killerID = source;
        unit.died = true;
        unit.state.Execute("Dying");
    }







  public:
    // base logic

    void UnitAI() {

        dead = false;

        Attach(&faction);
        Attach(&health);
        Attach(&obj);
        Attach(&state);
        Attach(&turret);
        Attach(&booster);

        Attach(&aurashape);
        Attach(&shape);

        health.SetDeathCallback(Event_OutOfHealth);
        state. CreateState("Dying", Dynacoe::StateControl::StateLoop(State_Dying));





        // no gun is the same on a unit
        float shootTime = Dynacoe::Random::Spread(
            min_refire_rate,
            max_refire_rate
        );

        float burstDuration = Dynacoe::Random::Spread(
            min_burst_duration,
            max_burst_duration
        );

        float burstRefireRate = Dynacoe::Random::Spread(
            min_burst_rate,
            max_burst_rate
        );

        turret.Setup(
            shootTime,
            burstDuration,
            burstRefireRate
        );



        aurashape.FormRectangle(23, 23);
        aurashape.node.local.postion = {11, 11};
        aurashape.color = {255, 255, 0, 64};
        aurashape.draw = false;


        obj.AddContactBox(
            Dynacoe::Point(-unit_size, -unit_size),
            unit_size*2, unit_size*2
        );




        // always in motion; never stopping
        obj.SeekCollisions(false);
        obj.SetVelocity(
            Dynacoe::Random::Spread(
                min_speed,
                max_speed
            ),
            Dynacoe::Random::Spread(0, 360)
        );


        booster.Setup(
            .0095,
            .003,
            .2,
            2, //2
            obj.GetSpeed()
        );

        health.SetHealth(300);
        turnAmount = unit_turn_amount;
    }




    void OnStep() {
        // Yay for StateControls!
        obj.SetSpeed(booster.GetSpeed());
        if (booster.IsActive()) {
            GetWorld().Create<BoostEffect>(Position);
        }
    }


    void OnDraw() {
        shape.node.local.rotation.z = -obj.GetDirection();
        if (turret.Level() + booster.Level() > 4) {
            aurashape.draw = true;
            aurashape.node.local.rotation.z += 1;
            aurashape.node.local.scale = {
                Dynacoe::Random::Spread(-1, 1),
                Dynacoe::Random::Spread(-1, 1)
            });
        }
    }




  private:



    Health health;
    Alignment faction;
    Turret turret;
    Booster booster;

    Dynacoe::Object2D obj;
    Dynacoe::Shape2D shape;
    Dynacoe::Shape2D aurashape;
    Dynacoe::StateControl state;
    Dynacoe::Clock dead;

    Dynacoe::Entity::ID baseID;


    Dynacoe::Entity::ID targetID;
    Dynacoe::Entity::ID killerID;
    bool hasTarget;
    bool died;
    bool isBoosting;
    float turnAmount;
    Dynacoe::Point targetPos;

};

#endif
