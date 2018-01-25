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



#ifndef DYNACOEVIEWMANAGER_H_INCLUDED
#define DYNACOEVIEWMANAGER_H_INCLUDED


#include <Dynacoe/Util/Table.h>
#include <Dynacoe/Modules/Module.h>
#include <Dynacoe/Backends/Display/Display.h>
#include <string>
#include <vector>
#include <map>





namespace Dynacoe {

using ViewID = LookupID; 

class Graphics;
class Color;






/**
 *  \brief The module that manages all Displays.
 *
 */
 // name change: ViewManager
class ViewManager : public Module {
public:
    friend class Engine;
    /* Display management */



    /// \brief Creates a new Display with the given w and h in pixels.
    /// Its ID is returned.
    ///
    static ViewID New(const std::string & name, int w, int h);


    /// \brief Convenience function for creating and immediately setting the new display as the current display to use.
    /// The graphics resultion is also set to match the display's dimensions.
    ///
    static void NewMain(const std::string & name = "", int w = 640, int h = 480);

   
    /// \brief Destroys the specified display.
    ///
    /// If this display was the main display and another display exists, 
    /// one of the existing displays will be set as the current display. It is not 
    /// specified which of the remaining displays is set to be the main one when this occurs.
    static void Destroy(ViewID);

    /// \brief Returns the Display corresponding to the given ID.
    ///
    static Display * Get(ViewID);


    /// \brief Sets the given Display as the one to update and draw to.
    ///
    static void SetMain(ViewID);


    /// \brief Returns a pointer to the currently set Display.
    ///
    static ViewID GetCurrent();

    /// \brief Gets the dimensions of the currently set Display.
    /// If no valid display is set, 0 is returned.
    ///
    static int GetViewHeight();

    /// \brief Gets the width of the currently set Display.
    ///
    static int GetViewWidth();


    /// \brief Returns a list of all currently created Displays.
    ///
    static std::vector<ViewID> ListViews(); 














  private:
    static void init();
    static void initBase();

    static std::string origCWD;
    
    struct LogicalDisplay {
        Display * display;
    };

    static Dynacoe::Table<LogicalDisplay> displays;
    static ViewID currentDisplay;

  public:
    std::string GetName() { return "ViewManager"; }
    void Init(); void InitAfter(); void RunBefore(); void RunAfter(); void DrawBefore(); void DrawAfter();
    Backend * GetBackend();
} ;
}
///\}














#endif // DYNACOESYS_H_INCLUDED

