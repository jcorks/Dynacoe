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


#ifndef H_TURRET_INCLUDED
#define H_TURRET_INCLUDED

/* Shoots bullets in bursts. Bursts are slightly more exciting than
   single-round firing. Slightly. */

#include "bullet.h"
#include <Dynacoe/Library.h>

class Turret : public Dynacoe::Entity {
  public:
    Turret()  {
        AddComponent(&barrel);

        refireRate = -1;
        burstDuration = -1;
        burstRate = -1;
        firing = false;
        level = 0;

        barrel.FormLines({
            {0, 0},
            {8, 0}
        });
    }


    // gives needed attributes to the turret
    void Setup(
        float refireRateSeconds,
        float burstDurationSeconds,
        float burstRateSeconds
    ) {


        refireRate    = refireRateSeconds*1000;
        burstDuration = burstDurationSeconds*1000;
        burstRate     = burstRateSeconds*1000;


        refireTimer.Set(1);

    }

    // Makes the turret better in some way
    void LevelUp() {
        switch(Dynacoe::Random::Integer(3)) {
            case 0: refireRate*=.7;
                break;

            case 1: burstDuration *= 1.4;
                break;

            case 2: burstRate *= .7;
                break;

            default:;
        };
        level++;
    }

    int Level() {
        return level;
    }


    void AimAt(const Dynacoe::Point & p) {
        target = p;
        barrel.node.local.rotation.z = -node.global.position.RotationZDiffRelative(p);
    }


    // requests firing of the turret.
    // the bullets will be given the same faction
    // as the host ( to prevent friendly fire)
    void Fire(Alignment::ID f) {
        // although we cant issue another burst if
        // one is already active,
        // we can update the target to shoot at
        // if a burst is already occuring
        align  = f;

        if (refireTimer.IsExpired() && !firing) {
            burstTimer.      Set(burstDuration);
            burstRefireTimer.Set(burstRate);
            firing = true;
        }



    }

    // percentage of time left before refire.
    float PercentTimeLeft() {
        return refireTimer.GetTimeLeft() / refireRate;
    }

    void OnStep() {
        if (!HasParent()) {Remove(); return;}
        if (!firing) return;


        // fire the rounds of the burst
        if (burstRefireTimer.IsExpired()) {
            Bullet * b = Dynacoe::Engine::Root().Identify()->CreateChild<Bullet>();
            b->Go(
                align,
                node.global.position,
                target,
                GetParent().GetID()
            );
            burstRefireTimer.Set(burstRate);

        }


        // when the burst is over, set the cool down for firing
        if (burstTimer.IsExpired()) {
            refireTimer.Set(refireRate);
            firing = false;
        }

    }

  private:

    Dynacoe::Clock refireTimer;
    Dynacoe::Clock burstTimer;
    Dynacoe::Clock burstRefireTimer;

    float refireRate;
    float burstDuration;
    float burstRate;

    bool firing;
    Alignment::ID align;
    Dynacoe::Point target;
    Dynacoe::Shape2D barrel;
    int level;

};

#endif
