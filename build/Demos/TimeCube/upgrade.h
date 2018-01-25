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


#ifndef H_UUPGRADE_INCLUDED
#define H_UUPGRADE_INCLUDED

#include "turret.h"
#include "health.h"
#include "booster.h"

#include <Dynacoe/Library.h>
#include <cmath>



// effect showing the unit just get way better
class Upgrade : public Dynacoe::Entity {
  public:
    const float terminalDist = 20.f;
    const float base_size = 7;


    enum class Type {
        Heal,
        Power,
        Speed,
        Special
    };



    Upgrade() {
        SetName("Upgrade");

        AddComponent(&obj);
        AddComponent(&state);
        AddComponent(&tri);

        state.CreateState("Created",  Dynacoe::StateControl::StateLoop(State_Created));
        state.CreateState("Idle",     Dynacoe::StateControl::StateLoop(State_Idle));
        state.CreateState("Collected",Dynacoe::StateControl::StateLoop(State_Collected));
        state.CreateState("FadeAway", Dynacoe::StateControl::StateLoop(State_FadeAway));

        state.Execute("Created");
        obj.AddContactBox(
            Dynacoe::Point(-base_size*6, -base_size*6),
            base_size*12,
            base_size*12
        );
        obj.InstallHandler("on-collide", collect);

        std::vector<Dynacoe::Point> pts;
        pts.push_back(Dynacoe::Point(sqrt(2)*base_size, sqrt(2)*base_size));
        pts.push_back(Dynacoe::Point(-sqrt(2)*base_size, sqrt(2)*base_size));
        pts.push_back(Dynacoe::Point(0, -base_size));
        tri.FormTriangles(
            pts
        );
        uniformSize = 0.f;
        terminalRate = .1f;
        rotationRate = 0.f;
        alpha = 1.f;

        speed = 0;
        power = 0;
        switch(Dynacoe::Random::Integer(2)) {
          case 0: type = Type::Heal;  break;
          case 1: type = Type::Power; power++; break;
          case 2: type = Type::Speed; speed++; break;
        }

        obj.SetFrictionX(.01);
        obj.SetFrictionY(.01);

    }

    void MoveTo(const Dynacoe::Point & p) {
        targetPoint = p;
        node.local.position = p;
    }

    void MakeSpecial(int pow, int spd) {
        power = pow;
        speed = spd;
        type = Type::Special;
    }


    int Speed() { return speed; }
    int Power() { return power; }

    void OnStep() {
        node.local.scale = (Dynacoe::Point(
            uniformSize,
            uniformSize
        ));
        node.local.rotation.z = rotationRate;

        if (type != Type::Special) {
            tri.color = {
                    (type==Type::Power? Dynacoe::Random::Spread(.5f, 1.f) : 0),
                    (type==Type::Heal?  Dynacoe::Random::Spread(.5f, 1.f) : 0),
                    (type==Type::Speed? Dynacoe::Random::Spread(.5f, 1.f) : 0),
                    alpha
            };
        } else {
            tri.color = {
                1.f,
                1.f,
                0.f,
                alpha
            };
        }
        if (obj.GetSpeed() > 14) obj.SetSpeed(14);

    }












    void GiveTo(Entity::ID other) {
        if (other.Valid() && !target.Valid() &&
            (other.Identify()->GetName() == "UnitAI" ||
             other.Identify()->GetName() == "UnitPlayer")) {
            target = other;
        }
    }



    /* simple states for effect */

    static DynacoeEvent(State_Created) {
        Upgrade & upgrade = *self.IdentifyAs<Upgrade>();
        upgrade.uniformSize = Dynacoe::Mutator::StepTowards(upgrade.uniformSize, 1.f, .1);
        if (upgrade.uniformSize > .90) {
            upgrade.state.Execute("Idle");
        }
        return true;
    }


    // locked in this state until a collision triggers the collected state
    static DynacoeEvent(State_Idle) {
        Upgrade & upgrade = *self.IdentifyAs<Upgrade>();
        upgrade.rotationRate = Dynacoe::Mutator::StepTowards(upgrade.rotationRate, 1.f, 0.05);

        if (upgrade.target.Valid())
            upgrade.state.Execute("Collected");

        return true;
    }




    static DynacoeEvent(State_Collected) {
        Upgrade & upgrade = *self.IdentifyAs<Upgrade>();
        upgrade.rotationRate = Dynacoe::Mutator::StepTowards(upgrade.rotationRate, 3.f, 0.2);


        if (!upgrade.target.Valid()) {
            upgrade.state.Execute("Idle");
            return true;
        }


        upgrade.terminalRate *= 1.01;



        Entity * ent = upgrade.target.Identify();
        upgrade.targetPoint = ent->node.global.position;
        upgrade.obj.AddVelocityTowards(.8, upgrade.node.global.position);



        if (upgrade.node.global.position.Distance(ent->node.global.position) < upgrade.terminalDist) {
            /*
            switch(upgrade.type) {
              case Type::Power: ent->QueryComponent<Turret> ()->LevelUp(); break;
              case Type::Heal:  ent->QueryComponent<Health> ()->HealUp();  break;
              case Type::Speed: ent->QueryComponent<Booster>()->LevelUp(); break;
            }
            */
            for(int i = 0; i < upgrade.power; ++i) ent->QueryComponent<Turret> ()->LevelUp();
            for(int i = 0; i < upgrade.speed; ++i) ent->QueryComponent<Booster> ()->LevelUp();
            if (!upgrade.power && !upgrade.speed) ent->QueryComponent<Health> ()->HealUp();
            upgrade.PlaySound();
            upgrade.state.Execute("FadeAway");
            return true;

        }

        return true;
    }

    static DynacoeEvent(State_FadeAway) {
        Upgrade & upgrade = *self.IdentifyAs<Upgrade>();

        upgrade.rotationRate = Dynacoe::Mutator::StepTowards(upgrade.rotationRate, 5.f, 0.05);
        upgrade.alpha = Dynacoe::Mutator::StepTowards(upgrade.alpha, 0.f, .08);
        upgrade.uniformSize+=.06;

        if (upgrade.target.Valid()) {
            // TODO: absolute position?
            upgrade.node.local.position = upgrade.target.Identify()->node.global.position;
        }
        if (upgrade.alpha < .02) {
            upgrade.Remove();
        }

        return true;
    }


  private:

    void PlaySound() {
        static Dynacoe::AssetID asset;
        if (!asset.Valid()) {
            asset = Dynacoe::Assets::Load("wav", "upgrade.wav");
        }

        Listener::PlaySoundProximity(node.global.position, asset);
    }

    static DynacoeEvent(collect) {
        Upgrade & u    = *self.IdentifyAs<Upgrade>();
        Entity & other = *source.Identify();
        if (u.target.Valid()) return true; // already collected

        if (other.GetName() == "UnitAI" ||
            other.GetName() == "UnitPlayer") {
            u.target = other.GetID();
            u.terminalRate = .1;
        }
    }
    float alpha;
    float uniformSize;
    float rotationRate;
    Dynacoe::Object2D obj;
    Dynacoe::StateControl state;
    Dynacoe::Entity::ID target;
    Dynacoe::Shape2D   tri;
    Dynacoe::Point      targetPoint;
    float terminalRate;
    Type type;
    int power;
    int speed;
};



#endif
