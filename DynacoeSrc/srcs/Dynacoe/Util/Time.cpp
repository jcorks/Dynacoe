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


#include <Dynacoe/Util/Time.h>


#ifdef DC_OS_WINDOWS
    #include <windows.h>
#endif

#ifdef DC_OS_LINUX
    #include <unistd.h>
    #include <time.h>
#endif


void Dynacoe::Time::SleepMS(size_t ms) {
    #ifdef DC_OS_WINDOWS
        Sleep(ms);
    #endif


    #ifdef DC_OS_LINUX
        usleep(ms * 1000);
    #endif



}



double Dynacoe::Time::MsSinceStartup() {
    static bool ticksBegin = false;

    
    #ifdef DC_OS_WINDOWS
        static LARGE_INTEGER ticksPerSecond;
        static LARGE_INTEGER beginTicks;
        static LARGE_INTEGER cTicks;
        if (!ticksBegin) {
            QueryPerformanceFrequency(&ticksPerSecond);
            QueryPerformanceCounter(&cTicks);
            
            
            ticksBegin = true;
        }
        QueryPerformanceCounter(&cTicks);
        return (cTicks.QuadPart - (double)beginTicks.QuadPart) * (1000.0 / (double)ticksPerSecond.QuadPart);
    #endif
        

    #ifdef DC_OS_LINUX

        static timespec time;
        static size_t beginTicks;

        if (!ticksBegin) {
            ticksBegin = true;
            clock_gettime(CLOCK_REALTIME, &time);
            beginTicks = (time.tv_nsec / 1000000.0) + (time.tv_sec *1000.0);
        }
        
        clock_gettime(CLOCK_REALTIME, &time);
        return ((time.tv_nsec / 1000000.0) + (time.tv_sec *1000.0)) - beginTicks;

    #endif
}


