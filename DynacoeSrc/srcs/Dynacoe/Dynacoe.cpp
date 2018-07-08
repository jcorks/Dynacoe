/*

Copyright (c) 2018, Johnathan Corkery. (jcorkery@umich.edu)
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


#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Util/Time.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Util/Filesys.h>
#include <Dynacoe/Util/Random.h>
#include <Dynacoe/Modules/Debugger.h>

#include <Dynacoe/Modules/Input.h>
#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Modules/ViewManager.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Modules/Sound.h>
#include <Dynacoe/Modules/Console.h>

#include <map>
#include <vector>


using namespace Dynacoe;
using std::string;

static std::string origCWD;



std::string Engine::Version() {
    #ifdef DYNACOE_GIT_LINEAR_COUNT
        std::string count = DYNACOE_GIT_LINEAR_COUNT;
    #else
        std::string count = "???";
    #endif


    #ifdef DYNACOE_GIT_HASH
        std::string hash = DYNACOE_GIT_HASH;
    #else
        std::string hash = "[unknown hash]";
    #endif

    return std::string() +
           "v0.96."+count+" (hash:"+hash+")";


}










static int maxFPS;
static bool quit;
Dynacoe::Clock        Engine::drawTime;
Dynacoe::Clock        Engine::runTime;
Dynacoe::Clock        Engine::sysTime;
Dynacoe::Clock        Engine::debugTime;
Dynacoe::Clock        Engine::frameTime;
static Dynacoe::Clock        engineTime;

std::vector<Module*>  Engine::modules;

Entity *               Engine::systemWorld;



static Engine::Diagnostics diagnostics;



int                     Engine::frameCount;
int                     Engine::valid = 0;


double                  Engine::frameStart = 0;
double                  Engine::frameEnd = 0;


Entity::ID                 Engine::universe;
Entity::ID managers;


static char buffer[1024];
string test="";

int Engine::Run() {

    if (!valid) return -1;



    if (quit == true) {
        Console::Error()<<("A component of Dynacoe has requested early termination of the instance")<< Console::End;;
        return -2;
    }

    Console::OverlayMessageMode(Console::MessageMode::Disabled);
    Console::Info() << "Dynacoe " << Version() << "\n\n";
    Console::Info() << "Johnathan Corkery, 2016\nhttp://coebeef.net/dynacoe\n_________________________\n\nRegistered modules:\n\n";
    for(int i = 0; i < modules.size(); ++i) {
        Backend * b;
        Console::Info() << "-  " << modules[i]->GetName().c_str() << ": ";
        if ((b = modules[i]->GetBackend())) {
            Console::Info() << b->Name()    << " [" <<
                               b->Version() << "]";

        } else {
            Console::Info() << "[Default]";
        }
        Console::Info() << "\n";
    }
    Console::Info() << "\n";
    Console::OverlayMessageMode(Console::MessageMode::Standard);



    while (!(quit)) {

        if (frameTime.GetTimeSince() >= 1000) {
            frameTime.Reset();

            diagnostics.drawTimeMS = drawTime.GetTimeSince() / (float)frameCount;
            diagnostics.stepTimeMS = runTime.GetTimeSince() / (float)frameCount;
            diagnostics.systemTimeMS = sysTime.GetTimeSince() / (float)frameCount;
            diagnostics.engineRealTimeMS = engineTime.GetTimeSince() / (float) frameCount;

            //if (lastDrawTime >4) cout << lastDrawTime << endl;

            diagnostics.currentFPS = frameCount;
            frameCount = 0;


            sysTime.Set();
            drawTime.Set();
            runTime.Set();
            debugTime.Set();
            engineTime.Set();

        }

        update();
        render();



        engineTime.Pause();
        if (GetMaxFPS() >= 0) {
            Engine::Wait(GetMaxFPS());
        } else {
            return -3;
        }
        engineTime.Resume();


        drawTime.Pause();
        runTime.Pause();
        debugTime.Pause();

        frameCount++;
    }


    return 0;

}

void Engine::AddModule(Module * m) {
    modules.push_back(m);
}


void Engine::Wait(int FPS) {

        frameEnd = Time::MsSinceStartup();
        double realEnd = frameEnd;

        while (realEnd - frameStart < 1000.0 / ((float)FPS)) {
            Time::SleepMS(1);
            realEnd = Time::MsSinceStartup();
        }

        frameStart = Time::MsSinceStartup();
}


void Engine::SetMaxFPS(int i) {
    maxFPS = i;
}

int Engine::GetMaxFPS() {
    return maxFPS;
}




std::string Engine::GetBaseDirectory() {
    return origCWD;
}

int Engine::Startup() {
    {
        Filesys obj;
        origCWD = obj.GetCWD();
    }


    AddModule(new Assets);
    AddModule(new Graphics);
    AddModule(new Sound);
    AddModule(new ViewManager);
    AddModule(new Input);
    AddModule(new Debugger);
    AddModule(new Console);


    quit = false;
    maxFPS = 60;

    diagnostics.drawTimeMS = 0;
    diagnostics.stepTimeMS = 0;
    diagnostics.systemTimeMS = 0;
    diagnostics.engineRealTimeMS = 0;



    frameCount = 0;
    frameTime.Reset();

    //Console::Info()  << "Initialized.";



    valid = true;
    universe = Entity::ID();
    managers = (new Entity())->GetID();


    for(int i = 0; i < modules.size(); ++i) {
        (modules[i]->Init());
    }
    for(int i = 0; i < modules.size(); ++i) {
        (modules[i]->InitAfter());
    }


    return 1;


}


Entity::ID & Engine::Root() {
    return universe;
}

void Engine::AttachManager(Entity::ID id) {
    Entity * ent = id.Identify();
    if (!ent) return;
    if (managers.Valid())
        managers.Identify()->Attach(id);
}

void Engine::render() {
    sysTime.Resume();
    for(uint32_t i = 0; i < modules.size(); ++i) {
        modules[i]->DrawBefore();
    }
    sysTime.Pause();

    drawTime.Resume();
    Entity * base = universe.Identify();
    if (base) base->Draw();
    drawTime.Pause();

    sysTime.Resume();
    for(uint32_t i = 0; i < modules.size(); ++i) {
        modules[i]->DrawAfter();
    }
    if (managers.Valid())
        managers.Identify()->Draw();
    sysTime.Pause();
    
    drawTime.Resume();
    if (Graphics::DrawEachFrame())
        Graphics::Commit();
    drawTime.Pause();
}


void Engine::update() {
    sysTime.Resume();
    for(uint32_t i = 0; i < modules.size(); ++i) {
        modules[i]->RunBefore();
    }
    sysTime.Pause();
    
    runTime.Resume();
    Entity * base = universe.Identify();
    if (base)base->Step();
    runTime.Pause();

    sysTime.Resume();
    for(uint32_t i = 0; i < modules.size(); ++i) {
        modules[i]->RunAfter();
    }
    
    if (managers.Valid())
        managers.Identify()->Step();

    sysTime.Pause();
}

void Engine::Quit() {
    quit = true;
}

const Engine::Diagnostics & Engine::GetDiagnostics() {
    return diagnostics;
}
