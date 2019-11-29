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

#ifndef H_COEDEUTIL_CLOCK_INCLUDED
#define H_COEDEUTIL_CLOCK_INCLUDED

/* A basic timer class with millisecond resolution */

#include <Dynacoe/Component.h>


#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

namespace Dynacoe {

/**
 * \brief Timing class with millisecond resolution.
 *
 * The setup is made to be very much like a stopwatch.
 *
 * Supported Component events:
 * - clock-step: Called every step when the clock is active.
 * - clock-draw: Called every draw when the clock is active.
 * - clock-expire: Called when the clock expires
 */
class Clock : public Component {

  public:
      


    /// \brief Initializes the timer with no expiration time.
    ///
    Clock();
    

    /// \brief Returns the ms since the start of timing.
    ///
    ///
    int GetTimeSince();

    /// \brief  Returns the ms until the timer expires.
    ///
    int GetTimeLeft();

    /// \brief Returns the duration of the clock.
    ///
    int GetDuration();

    /// \brief Resets the timer with the time it should expire.
    ///
    void Set(int msToExpire = -1);

    /// \brief Resets the timer with the previously used time limit.
    ///
    void Reset();

    /// \brief Pauses timing.
    ///
    void Pause();

    /// \brief Resumes timing.
    ///
    void Resume();

    /// \brief Returns whether or not time has run out on the timer.
    ///
    /// If there is no time limit, it will always return false.
    bool IsExpired();

    /// \brief Returns whether or not this time is paused.
    ///
    bool IsPaused();


    std::string GetInfo();
    void OnDraw();
    void OnStep();

  private:


    long startTime;
    long endTime;

    long pauseStart;
    long timePaused;
    int lastDuration;

    bool expired;
    void checkTime();
    long getMS();

    bool paused;


};


}

#endif
