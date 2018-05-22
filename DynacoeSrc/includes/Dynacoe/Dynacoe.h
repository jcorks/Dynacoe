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
class Universe;
class Module;
/** \brief Main class that handles automated updates of Dynacoe.
 *
 */
class Engine {


  public:


    /// \brief Initializes all Dynacoe assets.
    ///
    /// Upon encountering an irrecoverable error, the statup
    /// function will return 0. Otherwise, it will return 1.
    /// No other functions should be used prior to calling this function.
    ///
    ///
    static int Startup();


    /// \breif Installs a module to be updated during the execution loop.
    ///
    /// A module is an updatable unit that is managed before and after
    /// the updating of entities. It is appropriate for behavior that should
    /// not interfere with the entity scope. FOr example, the Graphics
    /// interface is managed through a module.
    ///
    /// This function adds a new module to the Engine. See Module.h
    static void AddModule(Module *);


    /// \brief Begins the Engine execution loop.
    ///
    /// Once run is initiated, all drawing to the screen and logic updating
    /// is automated. While running, it is also possible to utilize the
    /// debugging features such as the console and Entity debugger.
    static int Run();

    /// \brief Returns the toplevel Entity. 
    ///
    /// From here, you can 
    /// set the Entity that holds the toplevel of the project. 
    /// By default there is none.
    static Entity::ID & Root();

    /// \brief Attaches a management-type entity.
    ///
    static void AttachManager(Entity::ID);

	/// \brief Ends the Engine execution loop.
	///
    static void Quit();


    /// \brief Sleeps until the time required for the target frames-per-second is reached.
    ///
    /// @param FPS  The target FPS; useful for loops.
    ///
    /// The actual resolution is machine-dependent, but it tends to be millisecond resolution.
    static void Wait(int FPS);



    /// \brief Sets the ideal frame per second that the main loop should clock at.
    ///
    static void SetMaxFPS(int i);

    /// \brief Gets the maximum FPS allowable by the Engine.
    ///
    static int GetMaxFPS();

    /// \brief Returns the starting directory of Dynacoe.
    ///
    /// Reutrns an empty string if unavailable.
    static std::string GetBaseDirectory();

    /// \brief Returns the version of the Dynacoe library as a whole.
    ///
    static std::string Version();
    
    
    
    struct Diagnostics {
        float drawTimeMS;
        float stepTimeMS;
        float systemTimeMS;
        float engineRealTimeMS;        
        int currentFPS;
    };
    
    static const Diagnostics & GetDiagnostics();

  private:




    static double frameStart;
    static double frameEnd;

    static bool EXIT;






    static std::vector<Module*> modules;

    static Dynacoe::Clock drawTime;
    static Dynacoe::Clock runTime;
    static Dynacoe::Clock sysTime;
    static Dynacoe::Clock debugTime;
    static Dynacoe::Clock frameTime;

    static float lastDrawTime;
    static float lastRunTime;
    static float lastSysTime;
    static float lastDebugTime;

    static float curDrawTime;
    static float curRunTime;
    static float curSysTime;
    static float curDebugTime;

    static int frameCount;
    static int valid;

    static void render();
    static void update();

    static std::vector<Entity*> worlds;
    static Entity::ID universe;
    static Entity   * systemWorld;
};
};

#endif // H_CB_MAIN_INCLUDED
