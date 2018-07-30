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

#include <Dynacoe/BuiltIn/FillBar.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Components/Mutator.h>

#include <cmath>
#include <Dynacoe/Modules/Graphics.h>

using namespace Dynacoe;

const float ease_slow_rate      = .01;
const float ease_fast_rate      = .5f;

const float linear_slow_rate    = .1f; // percent of full fill
const float linear_fast_rate    = .5;

FillBar::FillBar() {
    SetName("FillBar");

    fraction    = 0;
    border      = 0;
    realFraction= 0;
    
    
    empty = CreateChild<Entity>();
    empty->SetName("empty");
    empty->AddComponent<Shape2D>();
    empty->QueryComponent<Shape2D>()->color = Dynacoe::Color(0, 0, 0);
    Attach(empty->GetID());
    
    full = CreateChild<Entity>();
    full->SetName("full");
    full->AddComponent<Shape2D>();
    full->QueryComponent<Shape2D>()->color = Dynacoe::Color(255, 255, 255);
    Attach(full->GetID());


    
    borderUp = CreateChild<Entity>();
    borderDown = CreateChild<Entity>();
    borderLeft = CreateChild<Entity>();
    borderRight = CreateChild<Entity>();
    
    
    borderUp->AddComponent<Shape2D>();
    borderDown->AddComponent<Shape2D>();
    borderLeft->AddComponent<Shape2D>();
    borderRight->AddComponent<Shape2D>();



    
    Attach(borderUp->GetID());
    Attach(borderLeft->GetID());
    Attach(borderRight->GetID());
    Attach(borderDown->GetID());
    

    
    SetFillTransition(FillTransition::EaseFast);
    SetFillDirection(FillDirection::Right);
    SetDims(100, 20);
    SetFill(1);

}


void FillBar::SetDims(int w, int h) {
    width  = w;
    height = h;

    if (border) {
        borderUp->   Node().Position() = {(float)border, 0.f};
        borderDown-> Node().Position() = {(float)border, (float)height - border};
        borderLeft-> Node().Position() = {0.f, 0.f};
        borderRight->Node().Position() = {(float)width - border, 0.f};


        borderUp->QueryComponent<Shape2D>()->   FormRectangle(width - border*2, border);
        borderDown->QueryComponent<Shape2D>()-> FormRectangle(width - border*2, border);
        borderLeft->QueryComponent<Shape2D>()-> FormRectangle(border, height);
        borderRight->QueryComponent<Shape2D>()->FormRectangle(border, height);
    }

    empty->QueryComponent<Shape2D>()->FormRectangle(w-border*2, h-border*2);
    full->QueryComponent<Shape2D>()->FormRectangle(w-border*2, h-border*2);
    SetFill(fraction);
}


void FillBar::SetBorder(int px) {
    border = px;
    empty->Node().Position() = {(float)px, (float)px};
    SetFill(fraction);
    SetDims(width, height);
}


void FillBar::SetFillDirection(FillDirection f) {
    fill = f;
    SetFill(fraction);
}

void FillBar::SetFillTransition(FillTransition f) {
    transition = f;
    SetFill(fraction);
}

void FillBar::SetColorFill(const Dynacoe::Color & c) {
    full->QueryComponent<Shape2D>()->color = c;
}

void FillBar::SetColorEmpty(const Dynacoe::Color & c) {
    empty->QueryComponent<Shape2D>()->color = c;
}

void FillBar::SetColorBorder(const Color & c) {
    borderUp->QueryComponent<Shape2D>()->color = (c);
    borderDown->QueryComponent<Shape2D>()->color = (c);
    borderLeft->QueryComponent<Shape2D>()->color = (c);
    borderRight->QueryComponent<Shape2D>()->color = (c);
}


void FillBar::SetFill(float f) {
    Dynacoe::Math::Clamp(f, 0.f, 1.f);

    fraction = f;
}

void FillBar::Run() {
    if (fraction != realFraction) {
        switch(fill) {
          case FillDirection::Up:
            full->Node().Position() = (Vector(border, height - border));
            full->QueryComponent<Shape2D>()->FormRectangle(
                width-border*2,
                -((height-border*2)*realFraction)
            );
            break;
        
          case FillDirection::Right:
            full->Node().Position() = (Vector(border, border));
            full->QueryComponent<Shape2D>()->FormRectangle(
                (width - border*2)*realFraction,
                height - border*2
            );
            break;

          case FillDirection::Down:
            full->Node().Position() = (Vector(border, border));
            full->QueryComponent<Shape2D>()->FormRectangle(
                width-border*2,
                ((height-border*2)*realFraction)
            );
            break;

          case FillDirection::Left:
            full->Node().Position() = Vector(width - border, height - border);
            full->QueryComponent<Shape2D>()->FormRectangle(
                -(width - border*2)*realFraction,
                -(height - border*2)
            );
            break;
        };


        // handle movement of the fill bar. 
        
        switch(transition) {
          case FillTransition::Instant:
            realFraction = fraction;
            break;

          case FillTransition::EaseSlow:
            realFraction = Mutator::StepTowards(realFraction, fraction, ease_slow_rate);
            break;

          case FillTransition::EaseFast:
            realFraction = Mutator::StepTowards(realFraction, fraction, ease_fast_rate);
            break;
        
          case FillTransition::LinearSlow:
            realFraction += linear_slow_rate * (realFraction < fraction ? 1 : -1);
            if (fabs(realFraction - fraction) < linear_slow_rate)
                realFraction = fraction; 

            break;
          case FillTransition::LinearFast:
            realFraction += linear_fast_rate * (realFraction < fraction ? 1 : -1);
            if (fabs(realFraction - fraction) < linear_fast_rate)
                realFraction = fraction; 
            break;


        }
    }
}





