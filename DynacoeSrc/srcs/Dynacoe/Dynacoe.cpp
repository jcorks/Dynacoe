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
static int paused = false;




static double frameStart;
static double frameEnd;

static bool EXIT;







static Dynacoe::Clock * drawTime;
static Dynacoe::Clock * runTime;
static Dynacoe::Clock * sysTime;
static Dynacoe::Clock * debugTime;
static Dynacoe::Clock * frameTime;

static float lastDrawTime;
static float lastRunTime;
static float lastSysTime;
static float lastDebugTime;

static float curDrawTime;
static float curRunTime;
static float curSysTime;
static float curDebugTime;

static int frameCount;
static int valid;

static void render();
static void update();

static std::vector<Entity*> worlds;
static Entity::ID universe;
static Entity   * systemWorld;





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
           "v0.97."+count+" (hash:"+hash+")";


}










static int maxFPS;
static bool quit;
static Dynacoe::Clock        *engineTime;



static Engine::Diagnostics diagnostics;


static Entity::ID managers;
static Entity::ID managersNonPausable;

static char buffer[1024];
string test="";

void Engine::Iterate() {
    
    if (frameTime->GetTimeSince() >= 1000) {
        frameTime->Reset();

        diagnostics.drawTimeMS = drawTime->GetTimeSince() / (float)frameCount;
        diagnostics.stepTimeMS = runTime->GetTimeSince() / (float)frameCount;
        diagnostics.systemTimeMS = sysTime->GetTimeSince() / (float)frameCount;
        diagnostics.engineRealTimeMS = engineTime->GetTimeSince() / (float) frameCount;

        //if (lastDrawTime >4) cout << lastDrawTime << endl;

        diagnostics.currentFPS = frameCount;
        frameCount = 0;


        sysTime->Set();
        drawTime->Set();
        runTime->Set();
        debugTime->Set();
        engineTime->Set();

    }

    update();
    render();



    engineTime->Pause();
    if (GetMaxFPS() >= 0) {
        Engine::Wait(GetMaxFPS());
    }
    engineTime->Resume();


    drawTime->Pause();
    runTime->Pause();
    debugTime->Pause();

    frameCount++;
}

int Engine::Run() {

    if (!valid) return -1;



    if (quit == true) {
        Console::Error()<<("A component of Dynacoe has requested early termination of the instance")<< Console::End();;
        return -2;
    }

    Console::OverlayMessageMode(Console::MessageMode::Disabled);
    Console::Info() << "Dynacoe " << Version() << "\n\n";
    Console::Info() << "Johnathan Corkery, 2018\nhttps://jcorks.github.io/Dynacoe/\n_________________________\n\n";
    Console::Info() << "\n";
    Console::OverlayMessageMode(Console::MessageMode::Standard);



    while (!(quit)) {
        Iterate();
    }


    return 0;

}
class PlaceholderDisplay : public Entity {
    AssetID background;
    Shape2D * shape;

  public: 
    PlaceholderDisplay() {
        SetName("PAUSEDISPLAY");
        Display * disp = ViewManager::Get(ViewManager::GetCurrent());
        if (!disp) return;
        
        
        
        
        shape = AddComponent<Shape2D>();
        background = Graphics::GetRenderCamera().GetBackVisual();
        shape->FormImage(background);
        Node().Position() = Dynacoe::Graphics::GetCamera2D().Node().Position();
    }
    
    void OnRemove() {
        Assets::Remove(background);
    }

};

static Entity::ID placeholderID;


void Engine::Pause() {
    paused = true;
    placeholderID = Entity::Create<PlaceholderDisplay>();
    Engine::AttachManager(placeholderID, false);
    
}

void Engine::Resume() {
    if (placeholderID.Valid()) {
        placeholderID.Identify()->Remove();
    }
    paused = false;
}

void Engine::Break() {
    paused = true;
    placeholderID = Entity::Create<PlaceholderDisplay>();
    Engine::AttachManager(placeholderID, false);

    while(paused) {
        Iterate();
    }
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
static bool initialized = false;
int Engine::Startup() {
    Random::Seed();
    {
        Filesys obj;
        origCWD = obj.GetCWD();
    }
    Entity * timer = Entity::Create().Identify();
    drawTime = timer->AddComponent<Clock>();
    runTime = timer->AddComponent<Clock>();
    sysTime = timer->AddComponent<Clock>();
    frameTime = timer->AddComponent<Clock>();
    engineTime = timer->AddComponent<Clock>();
    debugTime = timer->AddComponent<Clock>();
    


    quit = false;
    maxFPS = 60;

    diagnostics.drawTimeMS = 0;
    diagnostics.stepTimeMS = 0;
    diagnostics.systemTimeMS = 0;
    diagnostics.engineRealTimeMS = 0;



    frameCount = 0;
    frameTime->Reset();

    //Console::Info()  << "Initialized.";



    valid = true;
    universe = Entity::ID();
    managers = Entity::Create();
    managersNonPausable = Entity::Create();


    Assets::Init();
    Graphics::Init();
    Input::Init();
    Sound::Init();
    Console::Init();

    Graphics::InitAfter();
    Sound::InitAfter();
    Debugger::InitAfter();
    Console::InitAfter();


    
    



    initialized = true;
    return 1;


}

bool Engine::Initialized() {
    return initialized;
}


Entity::ID & Engine::Root() {
    return universe;
}

void Engine::AttachManager(Entity::ID id, bool pausable) {
    Entity * ent = id.Identify();
    if (!ent) return;
    if (pausable) {
        if (managers.Valid())
            managers.Identify()->Attach(id);
    } else {
        if (managersNonPausable.Valid())
            managersNonPausable.Identify()->Attach(id);        
    }
}

void render() {

    if (!paused) {
        drawTime->Resume();
        Entity * base = universe.Identify();
        if (base) base->Draw();
        drawTime->Pause();
    }

    Console::DrawAfter();
    sysTime->Resume();
    
    if (!paused) {
        if (managers.Valid())
            managers.Identify()->Draw();
    }
    
    if (managersNonPausable.Valid())
        managersNonPausable.Identify()->Draw();

    sysTime->Pause();
    
    drawTime->Resume();
    if (Graphics::DrawEachFrame())
        Graphics::Commit();
    drawTime->Pause();
}


void update() {
    sysTime->Resume();

    Input::RunBefore();
    Debugger::RunBefore();
    Console::RunBefore();

    sysTime->Pause();
    


    if (!paused) {
        runTime->Resume();
        Entity * base = universe.Identify();
        if (base)base->Step();
        runTime->Pause();
    }
    
    sysTime->Resume();

    Sound::RunAfter();
    Console::RunAfter();
    if (!paused) {
        
        if (managers.Valid())
            managers.Identify()->Step();
    }
    if (managersNonPausable.Valid())
        managersNonPausable.Identify()->Step();

    sysTime->Pause();
}

void Engine::Quit() {
    quit = true;
}

const Engine::Diagnostics & Engine::GetDiagnostics() {
    return diagnostics;
}
