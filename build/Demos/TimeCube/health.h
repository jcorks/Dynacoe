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


#ifndef H_HEALTH_INCLUDED
#define H_HEALTH_INCLUDED

#include <Dynacoe/Library.h>


/* WHATS YOUR DAMAGE */
// (Also: who dun it?)

class Health : public Dynacoe::Component {
  public:

    Health() : Component("Health") {


        // assigns a function to be called when something hurts the host
        InstallEvent("damage");

        // assigns a function to be called when the host dies :(
        InstallEvent("death");

        // assigns a function to be called when the host scorez
        // first entity -> the killer, the second entry -> the victim
        InstallEvent("score");
    }

    std::string GetInfo() {
        Dynacoe::Chain() <<
            "Health: " << amount << " / " << total << "\n";
        ;
    }



    // hurt (or heal)
    void DealDamage(int i, Dynacoe::Entity::ID dealer) {
        amount-=i;
        if (amount < 0) amount = 0;
        EmitEvent("damage", dealer);
        // consume the death signal
        if (!amount)  {


            Health * other = dealer.Valid()?
                dealer.Identify()->QueryComponent<Health>()
                    :
                nullptr;

            if (other) {
                //other->scoreCB(dealer, GetHost()->GetID());
                EmitEvent("score", dealer);
            }

            score[dealer.Value()]++;
            EmitEvent("dealth", dealer);

        }
    }



    // Cheat
    void SetHealth(int i) {
        total = i;
        amount = i;
    }

    void HealUp() {
        amount = total;
    }

    // return health percent
    float GetHealthFraction() {
        return (amount / (float)total);
    }

    // RIP?
    bool IsDead() {
        return amount <= 0;
    }


    // Random health is always fun
    void OnAttach() {
        SetHealth(130 + Dynacoe::Random::Integer(50));
    }






    // return current score for a particular id
    static uint64_t GetKills(Dynacoe::Entity::ID id) {
        return score[id.Value()];
    }






  private:
    int amount;
    int total;

    static std::map<uint64_t, uint64_t> score;
};
std::map<uint64_t, uint64_t> Health::score;

#endif
