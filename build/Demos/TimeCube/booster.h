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


#ifndef H_BOOSTER_INCLUDED
#define H_BOOSTER_INCLUDED

#include <Dynacoe/Component.h>
#include <Dynacoe/Components/Mutator.h>

/* Grants the ability to boost.
   "It's better! MUCH better!" */

class Booster : public Dynacoe::Component {
  public:

    const float acceleration = .1;

    /* interface */

    // Sets the attributes of the boosterrrr
    void Setup(
        float fuelExpenditure_, // percent per frame
        float fuelRecharge_,   // percent per frame
        float minimumToBoost_,  // percent
        float boostAmount_,      // pixels per frame of increase
        float normalSpeed_
    ) {
        fuel = 1.f;
        fuelExpenditure = fuelExpenditure_;
        fuelRecharge = fuelRecharge_;
        minimumToBoost = minimumToBoost_;
        boostAmount = boostAmount_;
        normalSpeed = normalSpeed_;

        targetSpeed = normalSpeed;
        currentSpeed = normalSpeed;

    }

    // improves the booster in some way
    void LevelUp() {
        switch(Dynacoe::Random::Integer(4)) {
          case 0:
            fuelExpenditure *= .6;
            break;
          case 1:
            fuelRecharge *= 1.5;
            break;
          case 2:
            minimumToBoost *= .6;
            break;
          case 3:
            boostAmount *= 1.7;
            break;
          default:;
        }
        level++;
    };

    int Level() {
        return level;
    }

    // Turn the thing on or off
    void Activate(bool doIt) {
        if (doIt == false) {
            isOn = false;
            return;
        }

        if (!GetHost())  return;
        if (fuel <= minimumToBoost) return;

        isOn = true;
    }

    // Returns whether or not the booster is working.
    // A booster will not work
    bool IsActive() {
        return isOn;
    }

    // Returns amount of fuel as a percent.
    float FuelLeft() {
        return fuel;
    }


    float GetSpeed() {
        return currentSpeed;
    }


  public:

    Booster() : Dynacoe::Component("Booster"){}

    void OnAttach() {
        isOn = false;
        level = 0;
        fuel = 0;
        fuelRecharge = 0;
        fuelExpenditure = 0;
        minimumToBoost = 0;

        boostAmount = 0;
        normalSpeed = 0;
        currentSpeed = 0;
        targetSpeed = 0;

    }

    void OnStep() {
        if (isOn) {
            fuel -= fuelExpenditure;
            if (fuel <= 0.f) {
                fuel = 0.f;
                isOn = false;
            } else {
                targetSpeed = normalSpeed + boostAmount;
            }
        } else {
            if (fuel < 1.f) {
                fuel += fuelRecharge;
                if (fuel >= 1.f) fuel = 1.f;
            }
            targetSpeed = normalSpeed;
        }
        currentSpeed = Dynacoe::Mutator::StepTowards(
            currentSpeed,
            targetSpeed,
            acceleration
        );
    }

  private:

    bool  isOn;
    int   level;
    float fuel;
    float fuelExpenditure;
    float fuelRecharge;
    float minimumToBoost;

    float boostAmount;
    float normalSpeed;
    float currentSpeed;
    float targetSpeed;

};


#endif
