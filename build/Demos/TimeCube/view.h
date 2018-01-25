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


#ifndef H_VIEW_INCLUDED
#define H_VIEW_INCLUDED


#include <Dynacoe/Library.h>    
#include <Dynacoe/Util/Timer.h>

/* What's a modern 2d game without tons of easing? */
/*
    Behaviorly, the camera is a stack.
    You push targets for it to follow. Once the target expires
    (i.e. it dies, or PopTarget() is called), the next target
    that was pushed is followed. If no targets exist, the
    camera goes into a "free" state where it is controlled by the 
    user.
*/

class View : public Dynacoe::Entity {
  public:

    const float drama_bar_height   = 60;
    const float drama_bar_width    = 4000;// hacky
    const float drama_ease_rate    = .07f;
    const float movement_ease_rate = .07f;
    const float free_move_speed    = 2.3f;

    const Dynacoe::Color drama_color = Dynacoe::Color(16, 16, 16, 255);





    // Sets a new target to follow
    void PushTarget(Dynacoe::Entity::ID id) {
        CameraTarget target;
        target.id = id;
        target.hasDrama = false; // boring
        targets.push(target);
    }

    // Same as SetTarget, but hella dramatic
    void PushTargetDramatic(Dynacoe::Entity::ID id) {
        CameraTarget target;
        target.id = id;
        target.hasDrama = true; // meow were talking
        targets.push(target);
    }

    // ignores the active target if any
    void PopTarget() {
        if (targets.empty()) return;
        targets.pop();
    }

    // ignores, well, all targets ever set EVER
    void PopAllTargets() {
        while(!targets.empty()) targets.pop();
    }


    // whats my id always
    static Entity::ID ID() {
        return instanceID;
    }

    
    // makes an entity part of the HUD, so that it
    // is drawn immediately closest to the camera.    
    static void HUDify(Entity & ent) {
        View & view = *(instanceID).IdentifyAs<View>();
        assert(&ent && &view);
        ent .GetWorld().Detach(ent.GetID());
        view.GetWorld().Attach(ent.GetID());
        ent. SetPriority(view.GetPriority()+1);
    }
    




  public:



    void Init() {
        target = GetID();
        instanceID = GetID();
        dramaBar.SetColor(drama_color);

        dramaBar.FormRectangle(
            drama_bar_width, 
            drama_bar_height
        );

        dramaBar.SetCenter(Dynacoe::Point(
            drama_bar_width/2,
            0
        ));

        targetTop    = Dynacoe::Point(Dynacoe::Graphics::GetRenderResolutionWidth()/2, -Dynacoe::Graphics::GetRenderResolutionHeight());
        targetBottom = Dynacoe::Point(
            Dynacoe::Graphics::GetRenderResolutionWidth()/2, 
            Dynacoe::Graphics::GetRenderResolutionHeight()        
        );

        claspTop    = targetTop;
        claspBottom = targetBottom;
        


        Position = Dynacoe::Graphics::GetFlatPosition();
        view3d = Dynacoe::Graphics::GetActiveCamera();
        Dynacoe::Camera & cam = *Dynacoe::Graphics::GetCamera(view3d);
        cam.MoveTo(Dynacoe::Point(0, 0, 4));

    }

    void Run() {
        if (!targets.empty()) {

            // set up the drama bars
            if (targets.top().hasDrama) {
                targetTop.y    = 0;
                targetBottom.y = Dynacoe::Graphics::GetRenderResolutionHeight() - drama_bar_height;
            } else {
                targetTop.y    = - drama_bar_height;
                targetBottom.y = Dynacoe::Graphics::GetRenderResolutionHeight();
            }


            Entity * ent = targets.top().id.Identify();
            if (!ent) {
                targets.pop();
                return;   
            }
            Position = ent->Position - Dynacoe::Point(
                Dynacoe::Graphics::GetRenderResolutionWidth()/2,
                Dynacoe::Graphics::GetRenderResolutionHeight()/2
            );


            

        } else {
            if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_w)) {
                Position.y -= free_move_speed;
            }

            if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_a)) {
                Position.x -= free_move_speed;
            }

            if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_s)) {
                Position.y += free_move_speed;
            }

            if (Dynacoe::Input::GetState(Dynacoe::Keyboard::Key_d)) {
                Position.x += free_move_speed;
            }
        } 



        MoveCamera();

    }


    void Draw() {

        // Cinematic bars are always drawn where the clasp 
        // positions are + the current cameram position
        claspTop.x = Dynacoe::Mutator::StepTowards(claspTop.x, targetTop.x, drama_ease_rate);
        claspTop.y = Dynacoe::Mutator::StepTowards(claspTop.y, targetTop.y, drama_ease_rate);

        claspBottom.x = Dynacoe::Mutator::StepTowards(claspBottom.y, targetBottom.y, drama_ease_rate);
        claspBottom.y = Dynacoe::Mutator::StepTowards(claspBottom.y, targetBottom.y, drama_ease_rate);


        Dynacoe::Point camPos = Dynacoe::Graphics::GetFlatPosition();
        dramaBar.SetPosition(camPos + claspBottom);
        Dynacoe::Graphics::Draw(dramaBar);

        dramaBar.SetPosition(camPos + claspTop);
        Dynacoe::Graphics::Draw(dramaBar);
    }







    
  private:

    struct CameraTarget {
        Dynacoe::Entity::ID id;
        bool hasDrama;  
    };

    void MoveCamera() {  
        /*
        if (time.IsExpired()) {
            vector<uint64_t> vec = GetWorld().FindByName("UnitAI");
            if (vec.size()) {
                SetTarget(vec[vec.size()*Dynacoe::Random::Value()]);
                SetMode(Mode::TargetDramatic);
            }
            time.Reset(7000);
        }
        */
        
    
        Dynacoe::Point curPos(Dynacoe::Graphics::GetFlatPosition());
        curPos.x = Dynacoe::Mutator::StepTowards(curPos.x, Position.x, movement_ease_rate);
        curPos.y = Dynacoe::Mutator::StepTowards(curPos.y, Position.y, movement_ease_rate);
        Dynacoe::Graphics::MoveFlatTo(curPos);


    
    }

    Dynacoe::Entity::ID target;
    Dynacoe::AspectShape     dramaBar;
    Dynacoe::StateControl barPhase;

    // "It's a drama clasp!" - Katia Managan
    Dynacoe::Point claspTop;
    Dynacoe::Point claspBottom;

    Dynacoe::Point targetTop;
    Dynacoe::Point targetBottom;

    static Dynacoe::Entity::ID instanceID;
    std::stack<CameraTarget> targets;
    Dynacoe::CameraID  view3d;


};

Dynacoe::Entity::ID View::instanceID ;


#endif
