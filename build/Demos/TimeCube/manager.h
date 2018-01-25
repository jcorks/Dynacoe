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


#ifndef H_TCManager_INCLUDED
#define H_TCManager_INCLUDED

#include "base.h"
#include "unit.h"
#include "bullet.h"
#include "view.h"
#include "unitPlayer.h"
#include "bg.h"
#include "artifact.h"
#include "listener.h"
#include "menu.h"



/* Manages everything.*/

// Typically, it is helpful to have some sort of toplevel manager
// rather than inserting your game objects directly into the engine.


class TCManager : public Dynacoe::World {
  public:

    const char * title_string        = "Time Cube";
    const float default_win_width    = 720;
    const float default_win_height   = 540;


    

  public:
    static Dynacoe::Entity::ID ID() {
        return mainID;
    }





    


  private:
    // Has the camera focus dramatically on an object
    //   - all input should be ignored
    //     all objects should have their run's frozen until EndDramaticScene
    //     if there are multiple, queue them up.

    // callback on death for Bases
    static void DeathByGlamour(Entity::ID e) {
        TCManager * m = TCManager::ID().IdentifyAs<TCManager>();
        View * cam   =    m->cameraID.IdentifyAs<View>();

        assert(cam && m); // the view should always exist unless something went terribly wrong
        cam->PushTargetDramatic(e);
    }



    void AddBase(const Dynacoe::Point & p, Alignment::ID id) {
        Base * e = gameLayer.Create<Base>();
        e->Position = (p);
        e->SetDeathCallback(DeathByGlamour);
        bases.push_back(e);
        e->QueryComponent<Alignment>()->Change(id);
    }

    void SpawnPlayer() {

        UnitPlayer * player = gameLayer.Create<UnitPlayer>();
        View * camera = cameraID.IdentifyAs<View>();
        player->Position = Dynacoe::Point();
        player->SetBase(bases[0]->GetID());
        camera->PushTarget(player->GetID());
        playerID = player->GetID();
    }






    static std::string State_StartGame(Entity::ID ent) {
        if (!ent.Valid()) return "null";

        TCManager & self = *ent.IdentifyAs<TCManager>();



        Alignment::ID side1 = Alignment::New();
        Alignment::ID side2 = Alignment::New();
    


        // add bases 
        self.AddBase(Dynacoe::Point(0,  100), side1);
        self.AddBase(Dynacoe::Point(0, -100), side1);


        self.AddBase(Dynacoe::Point(800,  600), side2);
        self.AddBase(Dynacoe::Point(800, -600), side2);
        self.AddBase(Dynacoe::Point(1200, 0), side2);
    


        // artifact
        Artifact * art = self.gameLayer.Create<Artifact>();
        art->SetDeathCallback(DeathByGlamour);
        art->OwnedBy(side1);


        static Dynacoe::AssetID bgm;
        if (!bgm.Valid()) {
            bgm = Dynacoe::Assets::Load("ogg", "s1.ogg");
            Dynacoe::ActiveSound s = Dynacoe::Sound::PlayAudio(bgm, 1);
            s.SetRepeat(true);
        }
        




        self.SpawnPlayer();
        
        return "check-paused";
    }



    /// respawn player. Controlled by the respawn Scheduler.
    static bool Task_Respawn(Entity::ID ent) {
        TCManager & m = *ent.IdentifyAs<TCManager>();
        m.SpawnPlayer();
        m.respawnTimer.Reset();            
        return false;
    }


    static std::string State_CheckPaused(Entity::ID ent) {
        if (Dynacoe::Input::IsPressed(Dynacoe::Keyboard::Key_esc)) {
            mainID.IdentifyAs<TCManager>()->gameLayer.AutoRun(false);
            mainID.IdentifyAs<TCManager>()->uiLayer.AutoRun(true);
            mainID.IdentifyAs<TCManager>()->respawn.Pause(); // Lets not try to cheat the spawning, now!
            menuID.IdentifyAs<Menu>()->Position = Dynacoe::Graphics::GetFlatPosition();
            menuID.IdentifyAs<Menu>()->SetOptions({
                {"Resume Game",Opt_UnPause},
                {"Options",    Opt_Options},
                {"About",      Opt_About},
                {"Quit",       Opt_Quit},
            });

            return "paused";
        }

        // handle respawn task setup.
        TCManager & m = *ent.IdentifyAs<TCManager>();
        if (!m.playerID.Valid() && m.respawn.GetTasks().empty()) {
            
            // Start a one-time task to respawn the player.
            m.respawn.StartTask("Respawn", 0, Task_Respawn, 7000);
        }

        return "";
    }

    static std::string State_Paused(Entity::ID ent) {
        if (Dynacoe::Input::IsPressed(Dynacoe::Keyboard::Key_esc)) {
            return "unpause";
        }
        return "";
    }
    
    
    static std::string State_UnPause(Entity::ID ent) {
        mainID.IdentifyAs<TCManager>()->gameLayer.AutoRun(true);
        mainID.IdentifyAs<TCManager>()->uiLayer.AutoRun(true);
        mainID.IdentifyAs<TCManager>()->respawn.Resume();
        menuID.IdentifyAs<Menu>()->SetOptions({});
        return "check-paused";
    }

    


  public:

    static void Opt_StartGame() {
        // Transition to field
        menuID.IdentifyAs<Menu>()->SetOptions({});        
        mainID.IdentifyAs<TCManager>()->menuState.Execute("start");
    }

    static void Opt_UnPause() {
        // Transition to field
        mainID.IdentifyAs<TCManager>()->menuState.Execute("unpause");
    }
    
    static void Opt_MainMenu() {
        menuID.IdentifyAs<Menu>()->SetOptions({
            {"Start Game", Opt_StartGame},
            {"Options",    Opt_Options},
            {"About",      Opt_About},
            {"Quit",       Opt_Quit},
        });
    }

    
    static void Opt_Options() {
        menuID.IdentifyAs<Menu>()->SetOptions({
            {"Back", Opt_MainMenu},
        }); 
    }


    static void Opt_About() {
        menuID.IdentifyAs<Menu>()->SetOptions({
            {"Back", Opt_MainMenu},
        });        
    }


    static void Opt_PuaseOpts() {

    }

    static void Opt_Quit() {
        menuID.IdentifyAs<Menu>()->SetOptions({
            {"Yes", Opt_QuitConfirm},
            {"No", Opt_MainMenu} 
       });   
    }



    static void Opt_QuitConfirm() {
        Dynacoe::Engine::Quit();
    }

  
    TCManager() {


        mainID  = GetID();
                
        SetName("TCManager");
        Entity::Attach(&respawn);
        Entity::Attach(&menuState);
        
        menuState.CreateState("start",        State_StartGame);
        menuState.CreateState("paused",       State_Paused);
        menuState.CreateState("check-paused", State_CheckPaused);
        menuState.CreateState("unpause",      State_UnPause);
        
        Dynacoe::Graphics::EnableFiltering(true); // makes the fonts come off a littel nicer


        // We need a window or something, right?
        Dynacoe::ViewManager::SetMain(
            Dynacoe::ViewManager::New(
                title_string, 
                default_win_width, 
                default_win_height
            )
        );

        
        // may as well have the renderer match those dimensions
        Dynacoe::Graphics::GetTargetCamera().SetRenderResultion(
            default_win_width,
            default_win_height
        );

        uiLayer.  SetName("RR::UI");
        gameLayer.SetName("RR::GameLayer");
        bgLayer.  SetName("RR::BGLayer");

        
        Attach(uiLayer.GetID());
        Attach(gameLayer.GetID());
        Attach(bgLayer.GetID());


        // Helps finalize what order each worlds should be drawn in
        uiLayer  .SetPriority(1);
        gameLayer.SetPriority(0);
        bgLayer  .SetPriority(-1);


        // for now, jsut create some silly bases that may or may not be the same color




        // View is our camera!
        View * camera = uiLayer.Create<View>();
        cameraID = camera->GetID();


        


        // starfield
        bgLayer.Create<Space>();
        

        Menu * m = uiLayer.Create<Menu>();
        menuID  = m->GetID();
        Opt_MainMenu();
    }
    

    Dynacoe::Timer respawnTimer;
    void OnPreStep() {
        

        // Don't forget to run its worldly duties!

        Listener::SetPosition(cameraID.Identify()->Position + 
            Dynacoe::Point(Dynacoe::Graphics::GetRenderResolutionWidth()/2.f,
                           Dynacoe::Graphics::GetRenderResolutionHeight()/2.f));

    }


    


  private:
    Dynacoe::Scheduler    respawn;
    Dynacoe::StateControl menuState;
    Dynacoe::World uiLayer; 
    Dynacoe::World gameLayer;
    Dynacoe::World bgLayer;

    std::vector<Base *> bases;
    
    static Dynacoe::Entity::ID mainID;
    static Dynacoe::Entity::ID menuID;
    Dynacoe::Entity::ID cameraID;
    Dynacoe::Entity::ID playerID;

};

// this is not good
Dynacoe::Entity::ID TCManager::mainID;
Dynacoe::Entity::ID TCManager::menuID;

#endif
