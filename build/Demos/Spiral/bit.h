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


#ifndef H_SPIRAL_BIT_INCLUDED
#define H_SPIRAL_BIT_INCLUDED


#include <Dynacoe/Library.h>
#include <Dynacoe/Components/StateControl.h>
#include <Dynacoe/Components/Mutator.h>

// A little entity that's always in motion



class Bit : public Dynacoe::Entity {
    const int   trail_length_c       = 20;
    const float init_velocity_rate_c = .1f;
    



  public:

    // Sets the position the bit should follow.
    // It will try to re-orient itself to get to the position
    void Follow(const Dynacoe::Point & p) {
        destination = p;
    }


    // Fades the Bit's current color to a new one
    void Colorize(const Dynacoe::Color & c) {
        targetColor = c;
    }


    // Commands the Bit to fade away
    void Dismiss() {
        state.Execute("dismissed");
    }

    
    // Sets the acceleration
    void SetAcceleration(float v) {
        acceleration = v;
    }







    
    Bit() {
        SetName("Bit");

        AddComponent(&object2d);
        AddComponent(&state);




        state.CreateState("normal",    Dynacoe::StateControl::StateLoop(State_Normal));
        state.CreateState("dismissed", Dynacoe::StateControl::StateLoop(State_Dismissed));
        state.Execute("normal");

	    for(size_t i = 0; i < trail_length_c; ++i) {
		    linePts.push_back (Dynacoe::Point());
            line.push_back(BindComponent<Dynacoe::Shape2D>());

            line[i]->mode = Dynacoe::Render2D::RenderMode::Translucent;
	        line[i]->absolute = true;
        }


	    object2d.SetFrictionX(.02);
	    object2d.SetFrictionY(.02);

        acceleration = init_velocity_rate_c;



    }
  


    void OnStep() {

        // Always tries to get where its going
        object2d.AddVelocityTowards(
            acceleration, 
            destination
        );

        // Here we update the trail it leaves behind.
        // the trail is just a buunch of lines, where each point 
        // was a previous position. 
	    linePts[trail_length_c - 1] = node.global.position;
	    for(size_t i = 0; i < trail_length_c - 1; ++i) {
		    linePts[i] = linePts[i+1];
            if (i > 0)
                line[i]->FormLines({linePts[i], linePts[i-1]});
	    }

        
    }


  


  private:

    static DynacoeEvent(State_Normal) {
        // normal color fading
        Bit & bit = *self.IdentifyAs<Bit>();

        bit.color.r = Dynacoe::Mutator::StepTowards(bit.color.r, bit.targetColor.r, .1);
        bit.color.g = Dynacoe::Mutator::StepTowards(bit.color.g, bit.targetColor.g, .1);
        bit.color.b = Dynacoe::Mutator::StepTowards(bit.color.b, bit.targetColor.b, .1);

	    for(size_t i = 0; i < bit.trail_length_c - 1; ++i) {
		    bit.line[i]->color    = bit.color;
            // The later in the trail, the more transparent it is.
            bit.line[i]->color.a  = (i / (float) bit.trail_length_c); 
	    }
        return "";
    }

    static DynacoeEvent(State_Dismissed) {
        Bit & bit = *self.IdentifyAs<Bit>();

        // fade out and remove itself
        for(size_t i = 0; i < bit.trail_length_c - 1; ++i) {
	        if (bit.line[i]->color.a > 0) bit.line[i]->color.a=bit.line[i]->color.a.Byte()-1;
        }
        if (bit.line[0]->color.a.Byte() == 0) bit.Remove();
        return true;
    }


	std::vector<Dynacoe::Point> linePts;
	std::vector<Dynacoe::Shape2D*> line; 

	Dynacoe::Point destination;

	Dynacoe::Object2D object2d;
    Dynacoe::StateControl state;

    Dynacoe::Color targetColor;
    Dynacoe::Color color;

    float acceleration;

    
};


#endif
