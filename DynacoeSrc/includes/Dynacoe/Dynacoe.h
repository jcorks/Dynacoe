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

#ifndef H_CB_MAIN_INCLUDED
#define H_CB_MAIN_INCLUDED







#include <Dynacoe/Components/Clock.h>
#include <Dynacoe/Entity.h>
#include <cstdlib>
#include <sstream>
#include <iostream>


namespace Dynacoe {
/** \brief Main class that handles automated updates of Dynacoe.
 *
 */
namespace Engine {

    /// \brief 
    ///
    /// Returns whether Dynacoe has been initialized successfully.
    ///
    bool Initialized();

    /// \brief Initializes all Dynacoe assets.
    ///
    /// Upon encountering an irrecoverable error, the statup
    /// function will return 0. Otherwise, it will return 1.
    /// No other functions should be used prior to calling this function.
    ///
    ///
    int Startup();

    /// \brief Begins the Engine execution loop.
    ///
    /// Once run is initiated, all drawing to the screen and logic updating
    /// is automated. While running, it is also possible to utilize the
    /// debugging features such as the console and Entity debugger.
    int Run();
    
    
    /// \brief Pauses the Engine execution loop.
    ///
    /// Once in a paused state, 
    void Pause();
    
    // like pause but disrupts the engine immediately rather than safely waiting 
    // until the next iteration. Most useful for debugging prupose
    void Break();
    
    // resumes from a paused or broken state.
    void Resume();
    
    // Not equivalent to a frame, since frame throttling can occur.
    void Iterate(); 


    /// \brief Returns the toplevel Entity. 
    ///
    /// From here, you can 
    /// set the Entity that holds the toplevel of the project. 
    /// By default there is none.
    Entity::ID & Root();

    /// \brief Attaches a management-type entity.
    ///
    /// If pausable is false, the manager will continue to update even when 
    /// the engine is in a paused state.
    void AttachManager(Entity::ID, bool pausable = true);

	/// \brief Ends the Engine execution loop.
	///
    void Quit();


    /// \brief Sleeps until the time required for the target frames-per-second is reached.
    ///
    /// @param FPS  The target FPS; useful for loops.
    ///
    /// The actual resolution is machine-dependent, but it tends to be millisecond resolution.
    void Wait(int FPS);



    /// \brief Sets the ideal frame per second that the main loop should clock at.
    ///
    void SetMaxFPS(int i);

    /// \brief Gets the maximum FPS allowable by the Engine.
    ///
    int GetMaxFPS();

    /// \brief Returns the starting directory of Dynacoe.
    ///
    /// Reutrns an empty string if unavailable.
    std::string GetBaseDirectory();

    /// \brief Returns the version of the Dynacoe library as a whole.
    ///
    std::string Version();
    
    
    
    struct Diagnostics {
        float drawTimeMS;
        float stepTimeMS;
        float systemTimeMS;
        float engineRealTimeMS;        
        int currentFPS;
    };
    
    const Diagnostics & GetDiagnostics();


};
};

#endif // H_CB_MAIN_INCLUDED
