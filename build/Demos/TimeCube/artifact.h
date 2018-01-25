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


#ifndef H_ARTIFACT_INCLUDED
#define H_ARTIFACT_INCLUDED


#include "health.h"
#include "alignment.h"
#include "whiteout.h"
#include <Dynacoe/Library.h>



/* The big object(ive) */

class Artifact : public Dynacoe::Entity {
  public:


    /* Public functions */
    void OwnedBy(Alignment::ID id) {
        align.Change(id);
    }


    void SetDeathCallback(Entity::Event e) {
        deathCB = e;
    }





    void Init() {
        shake = 0.f;
        static bool inited = false; 
        static Dynacoe::Material material;
        static Dynacoe::AspectLight light;
        if (!inited) {

            material.SetSpecularColor(Dynacoe::Color(32, 128, 255, 255));
            material.SetDiffuseColor(Dynacoe::Color(125, 125, 125, 255));
            material.SetAmbientColor(Dynacoe::Color(64, 64, 32, 255));
            material.SetProgram(Dynacoe::Material::CoreProgram::Lighting);
            material.SetShininess(5);            

            light.FormLight(
                Dynacoe::AspectLight::Light::Point,
                20,
                Dynacoe::Color("white")
            );

            light.SetPosition(Dynacoe::Point(0, 0, -5)); 
            light.Enable(true);
            inited = true;

        }

        lightPtr = &light;
        tetra.FormPrimitive(
            Dynacoe::AspectMesh::PrimitiveMesh::PrimitiveCube    
        );
        tetra.SetMaterial(material);
        tetra.SetScale(Dynacoe::Point(.2, .2, .2));

        

        Attach(&obj);
        Attach(&health);
        Attach(&align);
        Attach(&state);
        obj.AddContactBox(Dynacoe::Point(-70, -70), 140, 140);
        health.SetHealth(10000);
        health.SetDamageCallback(Event_Hurt);
        health.SetDeathCallback(OnDeath);
        SetPriority(-10);
        

        lightIntensity = 10;
        

        // Set up states of the artifact
        state.CreateState("Idle",    Dynacoe::StateControl::StateLoop(State_Idle));
        state.CreateState("Hurt",    Dynacoe::StateControl::StateLoop(State_Hurt));
        state.CreateState("Hurting", Dynacoe::StateControl::StateLoop(State_Hurting));
        state.CreateState("Dying",   Dynacoe::StateControl::StateLoop(State_Dying));
        state.CreateState("Dead",    Dynacoe::StateControl::StateLoop(State_Dead));
        state.Execute("Idle");
    }


    void Run() {
        //lightPtr->SetPosition(tetra.GetPosition()+(Dynacoe::Point(-5, 5, -5)));
        lightPtr->SetLightIntensity(lightIntensity);

        lightIntensity = Dynacoe::Mutator::StepTowards(lightIntensity, 10.f, .05);
        rotationRate   = Dynacoe::Mutator::StepTowards(
            rotationRate,   
            .1 + 10*(health.GetHealthFraction()-1) * (health.GetHealthFraction()-1), 
            .08
        );
        
    }


    void Draw() {

        //Position = Dynacoe::Point(Dynacoe::Input::MouseX(), Dynacoe::Input::MouseY());

        Dynacoe::Point pos = 
            Dynacoe::Graphics::GetCamera(Dynacoe::Graphics::GetActiveCamera())->Transform2Dto3D(Position, 10);
        tetra.SetPosition(pos);
        lightPtr->SetPosition(pos + Dynacoe::Point(0, 0, 5));
        


        tetra.SetRotation(
            tetra.GetRotation() + Dynacoe::Point(
                rotationRate, 2*rotationRate, rotationRate
            )
        );
        
        Dynacoe::Graphics::Draw(tetra);

        
    }
    

    





    static std::string State_Idle(Entity::ID id) {
        Artifact * self = id.IdentifyAs<Artifact>(); 
        if (self->health.IsDead()) {
            return "Dying";
        }
        return "";
    }

    
    static std::string State_Hurt(Entity::ID id) {
        Artifact * self = id.IdentifyAs<Artifact>();
        self->originalPos = self->Position;
        self->shake += 3;
        self->lightIntensity = 200.f;
        self->rotationRate += 5;
        return "Hurting";
    }


    static std::string State_Hurting(Entity::ID id) {
        Artifact * self = id.IdentifyAs<Artifact>();
        if (self->shake > .01) {
            self->Position = self->originalPos + Dynacoe::Point(
                Dynacoe::Random::Spread(-self->shake, self->shake),
                Dynacoe::Random::Spread(-self->shake, self->shake),
                Dynacoe::Random::Spread(-self->shake, self->shake)
            );
            self->shake *= .9;
        
            return "";
        } else {
            self->Position = self->originalPos;
            return "Idle";
        }
    } 




    Dynacoe::Timer deathTime;
    static std::string State_Dying(Entity::ID id) {
        Artifact * self = id.IdentifyAs<Artifact>();
        self->tetra.SetScale(Dynacoe::Point(
            Dynacoe::Random::Spread(.2, .2+self->crazyMultiplier),
            Dynacoe::Random::Spread(.2, .2+self->crazyMultiplier),
            Dynacoe::Random::Spread(.2, .2+self->crazyMultiplier)
            
        ));
        self->rotationRate*=1.005;
        self->lightIntensity*=1.1;
        self->crazyMultiplier*=1.001;
        if (self->deathTime.IsExpired()) {
            self->GetWorld().Create<Whiteout>(self->Position);
            self->deathTime.Reset(10);
            return "Dead";
        }
        return "";
    }

    static std::string State_Dead(Entity::ID id) {
        Artifact * self = id.IdentifyAs<Artifact>();
        
        self->rotationRate = 0;
        self->lightIntensity = 100;
        self->crazyMultiplier*=.9;

        self->tetra.SetScale(Dynacoe::Point(
            self->crazyMultiplier,
            self->crazyMultiplier,
            self->crazyMultiplier
            
        ));

        if (self->crazyMultiplier < .001) self->Remove();
        return "";
    }



    static void Event_Hurt(Entity::ID id) {
        Artifact * self = id.IdentifyAs<Artifact>();
        if (self->state.GetCurrentState() == "Hurting") 
            self->Position = self->originalPos;

        self->state.Execute("Hurt");
    }


    static void OnDeath(Entity::ID id, Entity::ID killer) {
        Artifact * self = id.IdentifyAs<Artifact>();
        self->RemoveComponent(&self->health);
        if (self->deathCB) self->deathCB(id);
        self->deathTime.Reset(4000);
        self->crazyMultiplier = .1;
        self->state.Execute("Dying");
        
    }



  private:
    float shake;
    Dynacoe::StateControl state;
    Dynacoe::Point originalPos;
    Dynacoe::AspectMesh tetra;
    Dynacoe::Object2D   obj;
    Dynacoe::AspectLight * lightPtr;
    Health              health;
    Alignment           align;
    float lightIntensity;
    float rotationRate;
    float crazyMultiplier;
    Dynacoe::Entity::Event deathCB;
    
};


#endif
