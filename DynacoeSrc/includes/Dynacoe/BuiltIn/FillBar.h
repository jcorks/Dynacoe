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

#ifndef H_DC_FILL_BAR_INCLUDED
#define H_DC_FILL_BAR_INCLUDED

#include <Dynacoe/Entity.h>
#include <Dynacoe/Color.h>
#include <Dynacoe/Components/Object2D.h>
#include <Dynacoe/Components/Shape2D.h>


namespace Dynacoe {

/// \brief Simple visual Entity representing a fraction of something.
///
/// In situations where you need a visual to show a bar, like a loading progress bar
///, or a health bar, this is a viable option. 
class FillBar : public Entity {
  public:

    /// \brief Sets the fill direction of the bar.
    ///
    enum class FillDirection {
        Up,     ///< Fill from the bottom to the top.
        Left,   ///< Fill from the right to the left.
        Down,   ///< Fill from the top to the bottom.
        Right   ///< Fill from the left to the right. This is the default.
    };

    /// \brief Sets how the bar will show the fraction of fill
    /// when it changes.
    ///
    enum class FillTransition {
        Instant,    ///< Always shows exactly the fill amount.
        EaseSlow,   ///< Will slowly ease the bar to match the fill.
        EaseFast,   ///< Will quickly ease the bar to match the fill. This is the default.
        LinearSlow, ///< Will linearly fill the bar at a moderate rate.
        LinearFast  ///< Will linearly fill the bar at a slow rate.
    };



    FillBar();

    /// \brief Sets the dimensions of the bar.
    /// 
    void SetDims(int w, int h);

    /// \brief Sets the number of pixels to make the border.
    ///
    void SetBorder(int px);

    /// \brief Sets the fill direction.
    ///
    void SetFillDirection(FillDirection f);
    
    /// \brief Sets the fill transition.
    ///
    void SetFillTransition(FillTransition f);



    /// \brief Sets the color of the filling visual.
    ///
    void SetColorFill(const Color & c);

    /// \brief Sets the background color for the empty portion of the bar. 
    ///
    void SetColorEmpty(const Color & c);
    
    /// \brief Sets the color of the fill border.
    ///
    void SetColorBorder(const Color & c);

    /// \brief Sets the fill fraction amount. 
    ///
    /// Should be a fraction from 0.f to 1.f. If the amount is outside these ranges
    ///, the value is clamped.
    void SetFill(float f);



    



    void Run();



  private:
      
    
    Entity * empty;
    Entity * full;

    Entity * borderUp;
    Entity * borderDown;
    Entity * borderLeft;
    Entity * borderRight;

    float fraction;
    float realFraction;

    int border;  
    int width;
    int height;

    FillDirection fill;
    FillTransition transition;
};
}

#endif
