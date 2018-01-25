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


#ifndef H_UNITAI_INCLUDED
#define H_UNITAI_INCLUDED


#include "unit.h"
#include "targeter.h"


/* Shoots things. Powered by a computer brain */




class UnitAI : public Unit {
  private:

    const float dist_away_from_base_max    = 1000;
    const float dist_away_from_base_return = 4000;
    const float target_min_fire_dist       = 130;
    const float target_max_fire_dist       = 170;


    // the normal state:
    //      - Look for things to shoot
    //      - go towards the thing to shoot
    //      - shoot it
    static DynacoeEvent(State_Hunt) {

        UnitAI & unit = *self.IdentifyAs<UnitAI>();
        if (unit.IsDead()) {
            unit.control.Execute("Null");
            return true;
        }


        if (unit.updateEnemy.IsExpired()) {
            unit.updateEnemy.Reset(10000);
            unit.targeter.FindNewTarget();
        }



        Entity::ID targetID = unit.targeter.GetCurrentTarget();

        // If our target is still a target, go towards it and shoot it
        // else, get a new target
        if (targetID.Valid() && IsTarget(*ent.Identify(), *targetID.Identify())) {
            Entity & curTarget = *targetID.Identify();

            // go to target
            unit.GoTowards(curTarget.Position);


            // sight always catches up to the real target position
            unit.sight.x = Dynacoe::Mutator::StepTowards(unit.sight.x, curTarget.Position.x, .04);
            unit.sight.y = Dynacoe::Mutator::StepTowards(unit.sight.y, curTarget.Position.y, .04);
            unit.Aim(unit.sight);

            // the element of surprise
            if (Dynacoe::Random::Value() > .998)
                unit.BoostOn();

            if (unit.Position.Distance(curTarget.Position) < unit.fireDistanceTolerance) {
                unit.Fire();
                unit.BoostOff();
            }

        } else {

            unit.targeter.FindNewTarget();
        }


        // if we travel too far from the base
        // change to state to retreat
        Entity * baseEnt = unit.GetBase().Identify();
        if (baseEnt && baseEnt->Position.Distance(unit.Position)
                     > unit.dist_away_from_base_max) {
            unit.control.Execute("ReturnToBase");
            return true;
        }
        return true;
    }








    // the "retreat" state.
    // if a target deviates too far, go back to base
    static Dynacoe::Event(State_ReturnToBase) {
        UnitAI & unit = *self.IdentifyAs<UnitAI>();
        if (unit.IsDead()) {
            unit.control.Execute("Null");
            return true;
        }


        // if the base blew up, nothing to return to
        Entity * baseEnt = unit.GetBase().Identify();
        if (!baseEnt) {
            unit.control.Execute("Hunt");
            return true;
        }



        // close enough to base. resume hunting
        unit.GoTowards(baseEnt->Position);
        if (unit.Position.Distance(baseEnt->Position)
            < unit.dist_away_from_base_return) {
            unit.targeter.ClearTarget();
            unit.control.Execute("Hunt");
            return true;
        }



        return true;
    }




    // returns whether or not e is a target of host
    // (This is the criterion for the Targeter)
    static bool IsTarget(Entity & host, Entity & e) {

        assert(&e && &host);
        return
               &e != &host &&
               e.QueryComponent<Alignment>() &&
                   e   .QueryComponent<Alignment>()->GetID() !=
                   host.QueryComponent<Alignment>()->GetID() &&
               e.QueryComponent<Health>();
    }





  public:
    void Init() {
        Unit::Init();
        updateEnemy.Reset(10000);
        BoostOff();


        SetName("UnitAI");

        // how comfortable a pilot is with firing
        // at a distance depends on their experience
        fireDistanceTolerance = Dynacoe::Random::Spread(
            target_min_fire_dist,
            target_max_fire_dist
        );


        Attach(&control);
        Attach(&targeter);

        targeter.SetTargetCriterion(IsTarget);
        targeter.FindNewTarget();

        control.CreateState("Hunt",         Dynacoe::StateControl::StateLoop(State_Hunt));
        control.CreateState("ReturnToBase", Dynacoe::StateControl::StateLoop(State_ReturnToBase));
        control.Execute("Hunt");


    }



  private:
    Dynacoe::StateControl control;
    Targeter targeter;
    Dynacoe::Point sight;
    float fireDistanceTolerance;
    Dynacoe::Timer updateEnemy;
};

#endif
