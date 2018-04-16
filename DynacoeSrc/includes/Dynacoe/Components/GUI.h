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

#ifndef H_DC_GUI_COMPONENT_INCLUDED
#define H_DC_GUI_COMPONENT_INCLUDED

#include <Dynacoe/Components/Node.h>

namespace Dynacoe {


/// \brief A component that provides managed input interaction suitable for 
/// making things like buttons, text boxes, etc.
///
/// Supported events:
/// - "on-click"
/// - "on-enter"
/// - "on-leave"
/// - "on-drag"
/// - "on-focus"
/// - "on-unfocus"
class GUI : public Component {
  public:
    GUI();
    ~GUI();


    

    /// \brief Defines the input pointer region.
    /// 
    /// This is defined as a rectangle region where the given point is the topleft point
    /// of the rectangle and w and h are the width and height respectively of the rectangle
    void DefineRegion(int w, int h);



    /// \brief Returns whether the GUI is focused view the SetFocus() function.
    ///
    bool IsFocused();

    /// \brief Returns whether user's pointer is over this GUI's region this frame.
    ///
    bool IsHovered();

    /// \brief Returns whether the user's pointer clicked within this GUI's region this frame.
    ///
    bool IsClicked();

    /// \brief Returns whether the user's pointer is attempting to drag the region.
    ///
    bool IsBeingDragged();

    Node node;


    /// \brief Attempts to grab the input focus for this GUI.
    ///
    /// If a GUI grabs the input focus, other GUI's will not have input events pushed 
    /// to them.
    void GrabInput();

    /// \brief Ungrabs the GUI that has grabbed input.
    ///
    /// If no GUI has grabbed input, no action is taken.
    static void UngrabInput();

    /// \brief Sets this GUI to be the focused widget.
    ///
    /// Focusing has no inate effect on the GUI except for the return values 
    /// from the IsFocused* set of functions.
    void SetFocus();

    /// \brief Unsets the current;y focused widget.
    ///
    /// This triggers the Unfocus event for the GUI that was previously focused.
    static void Unfocus();

    /// \brief Convenience function to get whether the focused widget is clicked.
    ///
    static bool IsFocusedClicked();
    
    
    /// \brief Convenience function to get whether the focused widget is hovered. 
    ///
    static bool IsFocusedHovered();


    /// \brief Returns the width of the defined region.
    ///
    int GetRegionW();

    /// \brief Returns the height of the defined region.
    ///
    int GetRegionH();
    
    /// \brief Sets text to appear if the pointer is above it.
    ///
    void SetTooltipText(const std::string & tooltip);
    
    /// \brief Returns the currently set tooltip text. The default is the empty string.
    ///
    const std::string & GetTooltipText() const;


    
    void OnStep();
    std::string GetInfo();
    void OnAttach();

  private:
    static GUI * focused;
    static GUI * grabbed;
    std::string tooltipText;
    bool hovered;
    int w, h;
};



}
#endif
