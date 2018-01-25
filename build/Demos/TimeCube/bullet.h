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


#ifndef H_BULLET_INCLUDED
#define H_BULLET_INCLUDED

#include "health.h"
#include "alignment.h"
#include "exp.h"
#include <Dynacoe/Library.h>

/* A fast moving thing that hurts. Ouchie. */







class Bullet : public Dynacoe::Entity {
  public:
    const float life_duration = 1.f;
    const float hitbox_size   = 10.f;


    const float speed_min = 8.f;
    const float speed_max = 12.f;


    // no bullet is perfect
    const float direction_spread_cone = 10;



    // Launches the bullet towards a target position.
    void Go(Alignment::ID f, Dynacoe::Point sourcePos, Dynacoe::Point targetPos, Dynacoe::Entity::ID source) {
        sourceID = source;
        node.local.position = sourcePos;
        obj.AddContactBox(
            Dynacoe::Point(-hitbox_size/2, -hitbox_size/2),
                 hitbox_size,
                 hitbox_size
        );
        obj.InstallHandler("on-collide", BulletCollision);


        float direction = sourcePos.RotationZDiffRelative(targetPos) + Dynacoe::Random::Spread(
            -direction_spread_cone/2,
             direction_spread_cone/2
        );


        obj.AddVelocity(
            Dynacoe::Random::Spread(speed_min, speed_max),
            direction
        );

        node.local.rotation.z = -direction;

        faction.Change(f);
        source = sourceID;
    }




    Dynacoe::Entity::ID GetSourceID() {
        return source;
    }






    Bullet() {
        SetName("Bullet");
        AddComponent(&faction);
        AddComponent(&obj);
        AddComponent(&line);

        life.Set(life_duration * 1000);

        PlaySound();
        line.FormLines({
            {-4.f, 0},
            {4.f, 0}
        });

    }





    void OnStep() {

        // Rapidly changing the color makes it seem for energy like
        line.color = Dynacoe::Color(
            Dynacoe::Random::Value(),
            Dynacoe::Random::Value(),
            Dynacoe::Random::Value(),
            1.f
        );

        if (life.IsExpired()) {
            Remove();
        }
    }



    void PlaySound() {
        static Dynacoe::AssetID asset;
        if (!asset.Valid()) {
            asset = Dynacoe::Assets::Load("wav", "bullet.wav");
        }

        Listener::PlaySoundProximity(node.global.position, asset);
    }

  private:


    Dynacoe::Entity::ID sourceID;





    // collision resolution for the bullet.
    // actually deals the damage


    static DynacoeEvent(BulletCollision) {//(Dynacoe::Entity::ID selfID, Dynacoe::Entity::ID otherID) {

        const float damage_min = 80;
        const float damage_max = 120;

        static Dynacoe::AssetID hitSound;
        if (!hitSound.Valid()) {
            hitSound = Dynacoe::Assets::Load("wav", "hurt.wav");
        }

        if (!self.Valid() || !source.Valid()) return true;

        Bullet & bullet= *self.IdentifyAs<Bullet>();
        Entity & other = *source.Identify();

        Alignment * faction = other.QueryComponent<Alignment>();
        Health * health = other.QueryComponent<Health>();
        if (health &&
           faction &&
           faction->GetID() != bullet.QueryComponent<Alignment>()->GetID()) {
            health->DealDamage(Dynacoe::Random::Spread(
                damage_min,
                damage_max
            ), bullet.sourceID);

           Listener::PlaySoundProximity(bullet.node.global.position, hitSound);
           bullet.Remove();
        }
        return true;
    }


    Dynacoe::Entity::ID source;
    Dynacoe::Clock life;
    Dynacoe::Object2D obj;
    Dynacoe::Shape2D line;
    Alignment faction;


};






#endif
