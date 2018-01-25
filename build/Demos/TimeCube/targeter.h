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


#ifndef H_TARGETER_INCLUDED
#define H_TARGETER_INCLUDED

#include <Dynacoe/Component.h>

/* Finds the bad guys (according to what the host thinks a badguy is)*/




class Targeter : public Dynacoe::Component {
  public:
    const float max_dist_target = 1500.f; // just needs to be a really big number


    Targeter() : Component("Targeter") {
        criterion = nullptr;
    }

    // function pointer for what constitutes a target.
    // Returning true says "Yes, this entity is a target"
    // else returning false says "Nope ignore it"
    using Criterion = bool (*)(Dynacoe::Entity & host, Dynacoe::Entity & target);


    // Sets the condition for what consititues a valid target
    void SetTargetCriterion(Criterion c) {
        criterion = c;
    }


    // Returns the current target from the last FindNewTarget().
    // If none was found, the target ID is the Host ID
    Dynacoe::Entity::ID GetCurrentTarget() {
        return targetID;
    }

    // Resets the targeter.
    void ClearTarget() {
        targetID = GetHost()->GetID();
    }

    // Tries to find the closest new target. If none could be found,
    // the target is set to the Host's ID.
    // THe ID is returned.
    Dynacoe::Entity::ID FindNewTarget() {
        if (!criterion) return GetHost()->GetID();

        assert(GetHost()->HasParent());
        Dynacoe::World & thisWorld = GetHost()->GetParent();
        uint32_t numEnts = thisWorld.GetNumEnts();
        float bestDist = max_dist_target, curDist;

        Dynacoe::Entity * e;
        for(int i = 0; i < numEnts; ++i) {
            e = thisWorld.GetEntity(i).Identify();

            if (e == GetHost() ||  !criterion(*GetHost(), *e)) continue;


            curDist = GetHost()->Position.Distance(e->Position);
            if (curDist < bestDist) {
                bestDist = curDist;
                targetID = e->GetID();
            }
        }
        if (bestDist >= max_dist_target) {
            targetID = GetHost()->GetID();
        }
        return targetID;
    }

    std::string GetInfo() {
        return Chain() << "Target ID: " << targetID.Value() << targetID.Valid() ? targetID.Idnentify()->GetName() : "";
    }

  private:
    Dynacoe::Entity::ID targetID;
    Criterion criterion;

};


#endif
