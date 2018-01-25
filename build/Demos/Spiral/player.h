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


#ifndef H_PLAYER_INCLUDED
#define H_PLAYER_INCLUDED

#include <Dynacoe/Library.h>
#include "bit.h"




// What the player controls
//
// Creates little "Bits" that follow the pointer.
// The player is used as the camera.
// 
class Player : public Dynacoe::Entity {
    const int 		num_init_bits_c		=	40;
    const float		init_friction_c 	=	.04f;
    const float		rotation_offset_c	=	.05f;
    const float		max_bit_speed_c     =   2.f;
    const float 	min_bit_speed_c     =   .1f;

    const int       create_bit_time_c   =   5000;
    const int       remove_bit_time_c   =   5000;
    const int       remove_bit_offset_c =   2500;
    const int       change_speed_time_c =   25000;
    const int       change_color_time_c =   3000;

  public:
	Player() {
        SetName("Player");
	    AddComponent(&object2d);
        AddComponent(&scheduler);        

	    object2d.SetFrictionX(init_friction_c);
	    object2d.SetFrictionY(init_friction_c);
	    ptRotate = 0;
        bitVel = 0;


        // Lets set up tasks. Tasks run periodically over time repeatedly.
        scheduler.StartTask("createBit",   create_bit_time_c,    Task_CreateBit);
        scheduler.StartTask("removeBit",   remove_bit_time_c,    Task_RemoveBit, remove_bit_offset_c);
        scheduler.StartTask("changeSpeed", change_speed_time_c,  Task_ChangeSpeed);
        scheduler.StartTask("changeColor", change_color_time_c,  Task_ChangeColor); 


        // Sets set the first color.
	    targetColor = Dynacoe::Color(
            Dynacoe::Random::Spread(0, 1.f),
		    Dynacoe::Random::Spread(0, 1.f),
		    Dynacoe::Random::Spread(0, 1.f), 1.f);


        // Lets create the first few bits to get us started
        for(int i = 0; i < num_init_bits_c; ++i) {
            Task_CreateBit(nullptr, GetID(), Entity::ID(), {});
        }
    }


	void OnStep() {
        

	    ptRotate += Dynacoe::Point(
		    object2d.GetVelocityX(), 
		    object2d.GetVelocityY()
	    ).Length()  + rotation_offset_c;




        // WASD allwso for moving the player object, which moves the camera.        

	    if (Dynacoe::Input::IsHeld("playerUp")) {
	    	object2d.AddVelocity(.3, 270);
	    }
	    if (Dynacoe::Input::IsHeld("playerRight")) {
        	object2d.AddVelocity(.3, 0);
	    }
	    if (Dynacoe::Input::IsHeld("playerLeft")) {
        	object2d.AddVelocity(.3, 180);
	    }
	    if (Dynacoe::Input::IsHeld("playerDown")) {
        	object2d.AddVelocity(.3, 90);
	    }




        // arrow keys control the speed of the bits flying around
	    if (Dynacoe::Input::IsPressed("playerFaster")) {
		    CommandFaster();
	    }
	    if (Dynacoe::Input::IsPressed("playerSlower")) {
		    CommandSlower();
	    }
	    if (Dynacoe::Input::IsPressed("playerFastest")) {
		    CommandFastest();
	    }
	    if (Dynacoe::Input::IsPressed("playerSlowest")) {
		    CommandSlowest();
	    }


        // spacebar creates a new bit.
	    if (Dynacoe::Input::IsPressed("playerMakeBit")) {
            Task_CreateBit(nullptr, GetID(), Entity::ID(), {});
	    }



        


        // update bit positions
	    for(size_t i = 0; i < bits.size(); ++i) {
            bits[i]->Follow(
                Dynacoe::Point(Dynacoe::Input::MouseX(), Dynacoe::Input::MouseY())
                + Dynacoe::Point(0, dLen).RotateZ(ptRotate + (360.0 / (bits.size()+1))*i)
            );
	    }


    }










  private:
	void CommandFaster() {
	    bitVel+=.1;
	    if (bitVel > max_bit_speed_c) bitVel=max_bit_speed_c;
	
	    for(size_t i = 0; i < bits.size(); ++i) {
		    bits[i]->SetAcceleration(bitVel);
	    }
    }
	void CommandSlower() {
	    bitVel-=.1;
	    if (bitVel < min_bit_speed_c) bitVel=min_bit_speed_c;
	
	    for(size_t i = 0; i < bits.size(); ++i) {
		    bits[i]->SetAcceleration(bitVel);
	    }
    }

	void CommandFastest() {
	    bitVel = max_bit_speed_c;
	    for(size_t i = 0; i < bits.size(); ++i) {
		    bits[i]->SetAcceleration(bitVel);
	    }
    }
	void CommandSlowest() {
	    bitVel = min_bit_speed_c;
	    for(size_t i = 0; i < bits.size(); ++i) {
		    bits[i]->SetAcceleration(bitVel);
	    }
    }




    // Creates a new bit
    static DynacoeEvent(Task_CreateBit) {
        Player & player = *self.IdentifyAs<Player>();

        Bit * bit;
	    bit = Dynacoe::Engine::Root().Identify()->CreateChild<Bit>();
        bit->Follow(player.node.local.position);
	    bit->Colorize(player.targetColor);

        player.bits.push_back(bit);
    	player.dLen = player.bits.size() / 3.0 + 10;

        return true;
    }


    // Removes a new bit
    static DynacoeEvent(Task_RemoveBit) {
        Player & player = *self.IdentifyAs<Player>();

        if (!player.bits.size()) return true;  
	    player.bits[0]->Dismiss();
	    player.bits.erase(player.bits.begin());

        return true;
    }


    static DynacoeEvent(Task_ChangeSpeed) {
        if (Dynacoe::Random::Value() < .5f)
            self.IdentifyAs<Player>()->CommandFaster();
        else 
            self.IdentifyAs<Player>()->CommandSlower();       
        return true; 
    }


    static DynacoeEvent(Task_ChangeColor) {
        Player & player = *self.IdentifyAs<Player>();

		player.targetColor = Dynacoe::Color(
                            Dynacoe::Random::Value(),
						    Dynacoe::Random::Value(),
						    Dynacoe::Random::Value(), 1.f);
        for(int i = 0; i < player.bits.size(); ++i) {
            player.bits[i]->Colorize(player.targetColor);
        }

        return true;
    }



	Dynacoe::Color targetColor;
	Dynacoe::Object2D object2d;
    Dynacoe::Scheduler scheduler;	

	std::vector<Bit *> bits;
	float dLen;
	float ptRotate;	
	float bitVel;





};

#endif
