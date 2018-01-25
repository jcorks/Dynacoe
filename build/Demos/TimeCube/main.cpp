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

#include <cassert>
//#include "manager.h"
#include "unitAI.h"
#include <Dynacoe/Library.h>

// All around stress test, though specifically for Entity addition / deletion
// with no asset handling


/*

TODO:


    units in general:
        - levels degrade: need to continually get upgrades or else levels
          will decrease


    player unit:
        - respawn from base if
            after 3 seconds
                and
            units can still be spawned




    alignment:
        - Unique team and colors?

    explosion:
        - camera shake if close to camera?



    base changes
        - cap on unit spawn
        - can fire on its own
            . three or four turrets: burst fire, large clip, very slow recharge
            . no target range
    title screen
        - Play (two fleets, you get one side
            - perhaps third one special event
        - Spectate (Watch two fleets destroy each other





*/

using std::vector;
using namespace Dynacoe;

/*
DynacoeEvent(new_bullet) {
    Turret * exp = self.IdentifyAs<Turret>();
    exp->AimAt({
        Input::MouseX(),
        Input::MouseY()
    });
    exp->Fire(Alignment::ID());

    return true;
}
*/


int main() {
    Engine::Startup();
    //Engine::Root() = Entity::Create<Manager>();

    ViewManager::NewMain("Test");
    Engine::Root() = Entity::Create();
    /*
    Turret * t = Engine::Root().Identify()->CreateChild<Turret>();
    t->node.local.position = {100, 100};
    t->Setup(.5, .2, .01);
    Scheduler * s = t->BindComponent<Scheduler>();
    s->StartTask("Make", 30, new_bullet);
    */
    Turret * t = Engine::Root().Identify()->CreateChild<UnitAI>();
    Engine::Run();
}
