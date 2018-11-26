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


#include <Dynacoe/Components/Clock.h>


#ifdef _WIN32
long Dynacoe::Clock::getMS() {
	DWORD time = GetTickCount();
	return time;
}
#else
long Dynacoe::Clock::getMS() {
	struct timeval timeValue;
	gettimeofday(&timeValue, 0);
	return timeValue.tv_usec/1000 + timeValue.tv_sec*1000;
}
#endif




Dynacoe::Clock::Clock() : Component(){
    SetTag("Clock");
	InstallEvent("clock-step");
	InstallEvent("clock-draw");
	InstallEvent("clock-expire");
	Set(-1);
}


std::string Dynacoe::Clock::GetInfo() {
	return (Chain() << "Time Left: " << GetTimeLeft() << " ms");
}


void Dynacoe::Clock::Set(int msToExpire) {
	startTime = getMS();
	endTime   = startTime + msToExpire;
	expired   = false;
	paused    = false;
	timePaused= 0;
	lastDuration = msToExpire;
}

void Dynacoe::Clock::Reset() {
    Set(lastDuration);
}

void Dynacoe::Clock::checkTime() {
    if (paused) return;
	if (endTime < startTime) {
            expired = false;
            return;
	}
	long currentTime = getMS();
	if (currentTime - timePaused >= endTime && !expired) {
		expired = true;
		EmitEvent("clock-expire");
	}
}

int Dynacoe::Clock::GetTimeLeft() {
	if (!expired) checkTime();

	if (expired) {
		return 0;
	}

	if (endTime < startTime) {
		return -1;
	}

    return (endTime - startTime) - GetTimeSince();
}

int Dynacoe::Clock::GetTimeSince() {
    if (!paused) {
        long currentTime = getMS();
        return currentTime - startTime - (timePaused);
    } else {
        return pauseStart - startTime - timePaused;
    }
}

bool Dynacoe::Clock::IsExpired() {
    checkTime();
    return expired;
}

void Dynacoe::Clock::Pause() {
    if (!paused) {
        paused = true;
        pauseStart = getMS();
    }
}

void Dynacoe::Clock::Resume() {
    if (paused) {
        timePaused += (getMS() - pauseStart);
        paused = false;
    }
}

bool Dynacoe::Clock::IsPaused() {
    return paused;
}

int Dynacoe::Clock::GetDuration() {
	return lastDuration;
}


void Dynacoe::Clock::OnStep() {
	if (!GetHost() || IsExpired()) return;
	EmitEvent("clock-step");
}

void Dynacoe::Clock::OnDraw() {
	if (!GetHost() || IsExpired()) return;
	float ratio = lastDuration < 0 ? 1.f : GetTimeLeft() / float (lastDuration);
	std::string data = (Chain() << ratio);
	EmitEvent("clock-draw");
}

