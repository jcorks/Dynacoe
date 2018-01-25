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


#include <Dynacoe/Sequence.h>

using namespace Dynacoe;



Sequence::Note::Note() {
    duration = -1;
    sound = AssetID();
    volume = 0;
    panning = 0;

}






Sequence::Chord::Chord(uint32_t c) {
    setTime = 0;
    channel = c;
}

Sequence::Chord::Chord(const std::vector<Note> & t, float startTime, uint32_t c) {
    channel = 0;
    setTime = startTime;

    for(auto i = t.begin(); i != t.end(); ++i) {
        AddNote(*i);
    }
}

Sequence::Note & Sequence::Chord::GetNote(int n) {
    if (n < 0 || n >= (int) nList.size()) {
        static Note bad;
        return bad;
    }

    return nList[n];
}

int Sequence::Chord::GetNumNotes() {
    return nList.size();    
}

void Sequence::Chord::AddNote(const Sequence::Note & n) {
    nList.push_back(n);
}


void Sequence::Chord::RemoveNote(int n) {
    if (n < 0 || n >= nList.size()) return;
    nList.erase(nList.begin() + n);
}

void Sequence::Chord::SetStartTime(float t) {
    setTime = t;
}

float Sequence::Chord::GetStartTime() {
    return setTime;
}

uint32_t Sequence::Chord::GetChannel() {
    return channel;
}

void Sequence::Chord::SetChannel(uint32_t i) {
    channel = i;
}






Sequence::Measure::Measure() {
    index = 0;
}

Sequence::Measure::Measure(const std::vector<Chord> & t) {
    index = 0;
    for(auto i = t.begin(); i != t.end(); ++i) {
        AddChord(*i);    
    }
}

Sequence::Chord & Sequence::Measure::GetChord(int i) {
    if (i < 0 || i >= cList.size()) { 
        static Chord bad(0);
        return bad; 

    }
    return cList[i];
}


void Sequence::Measure::AddChord(const Chord & c) {
    cList.push_back(c);
}


void Sequence::Measure::RemoveChord(int n) {
    if (n < 0 || n >= cList.size()) return;
    cList.erase(cList.begin() + n);
}
int Sequence::Measure::GetNumChords() {
    return cList.size();
}












Sequence::Sequence(const std::string & n) : Asset(n){

    beatsPerMinute = 120;
    beatsPerMeasure = 4;
    beatDuration = 1/4;
    //lastPlayed = 0;
}

float Sequence::GetLength() {

    for (int i = mList.size() - 1; i > -1; --i) {
        Measure & m = mList[i];
        if (m.GetNumChords()) {
            return (beatsPerMeasure *(i + 2) / (beatsPerMinute / 60.0));
        }
    }
    return -1;
}


int Sequence::GetNumMeasures() {
    return mList.size();
}

int Sequence::GetNumMeasuresUsed() {
    for(int i = GetNumMeasures(); i > -1; --i) {
        Measure & target = GetMeasure(i);
        if (target.GetNumChords()) {
            return i;
        }
    }
    return 0;
}

void Sequence::AddMeasures(const std::vector<Measure> & s) {
    
    for(auto i = s.begin(); i != s.end(); ++i) {
        mList.push_back(*i);
    }
}

Sequence::Measure & Sequence::GetMeasure(int i) {
    if (i < 0 || i >= (int)mList.size()) {
        static Measure bad;
        return bad;
    }
    //Sys::log.oInfo("Processed request for measure "); Sys::log.oStream(i);
    return mList[i];
}



void Sequence::RemoveMeasure(int n) {
    if (n < 0 || n >= mList.size()) return;
    mList.erase(mList.begin() + n);
}





