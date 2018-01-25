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

#include <Dynacoe/Components/Sequencer.h>
#include <Dynacoe/AudioBlock.h>
#include <Dynacoe/Util/Time.h>
using namespace Dynacoe;

class Dynacoe::Delay {
    int createTime;
  public:
    Delay();
    int getCreateTime();
    uint32_t channel;
    AssetID data;
    uint8_t volume;
    double delay;
    float leftPanning;
    float rightPanning;
    double duration;
};

bool DelayComparison(const Delay * a, const Delay * b) {
    return (a->delay < b->delay);
}


Sequencer::Sequencer() : Component("Sequencer") {
    bool (*comp)(const Delay *, const Delay *) = DelayComparison;
    soundQueue = new std::set<Delay *, bool(*)(const Delay *, const Delay *)>(comp);
    playbackActive = false;
    lockPlayback = false;
    pause = false;
    playbackProgress = 0;
    playbackStart = 0;
    pauseTime = 0;
    pauseStartTime = 0;
    totalPauseTime = 0;
}

Sequencer::~Sequencer() {
    delete soundQueue;
}


void Sequencer::Pause() {
    if (!IsPlaying()) return;

    pause = true;
        pauseStartTime = Time::MsSinceStartup();
        /*
        Console::Info()<<("Total pause time@ "); Console::Info()<<(pauseTime); Console::Info()<<("ms");
        Console::Info()<<("Playback Progress@ "); Console::Info()<<(playbackProgress); Console::Info()<<("ms");
        Console::Info()<<("Playback Started @ "); Console::Info()<<(playbackStart); Console::Info()<<("ms");
        */


}

void Sequencer::Resume() {
    if (!pause || !IsPlaying()) return;

    pause = false;
    totalPauseTime += pauseTime;

}


int Sequencer::GetDuration() {
    if (soundQueue->size())
        return (*soundQueue->rbegin())->delay;
    else
        return 0;

}







uint32_t Sequencer::GetPlaybackTime() {
    return playbackProgress;
}

uint32_t Sequencer::GetPlaybackStart() {
    return playbackStart;
}




int Sequencer::Queue(const Sequence & i, double delay) {

    Sequence * seq = (Sequence *)&i; // "Trust me"

    /*
    Console::Info()<<("  -"); Console::Info()<<(seq->beatsPerMinute);
                          Console::Info()<<(" Beats per minute");
    Console::Info()<<("  -"); Console::Info()<<(seq->GetNumMeasures());
                          Console::Info()<<(" Measures");
    Console::Info()<<("  -"); Console::Info()<<("Duration: Approx ");
                          Console::Info()<<(seq->GetLength());
                          Console::Info()<<(" s.");
    */


    for(int n = 0; n < seq->GetNumMeasures(); ++n) {
        int numNotes = seq->GetMeasure(n).GetNumChords();
        double curTime = (n * seq->beatsPerMeasure) / (seq->beatsPerMinute / 60.0 );
        //Console::Info()<<("Loading Measure "); Console::Info()<<(n); Console::Info()<<(", ");
        //Console::Info()<<(numNotes); Console::Info()<<(" intervals");
        for(int j = 0; j < numNotes; ++j) {
                //Console::Info()<<("  Loading Note "); Console::Info()<<(j);
                Sequence::Chord * cur_Chord = &seq->GetMeasure(n).GetChord(j);
                double addedTime = cur_Chord->GetStartTime() / (seq->beatsPerMinute / 60.0 );
                for(int k = 0; k < cur_Chord->GetNumNotes(); ++k) {
                    Sequence::Note * curNote = &cur_Chord->GetNote(k);
                    if (!curNote->sound.Valid()) {
                        //Console::Info()<<("Discarded note from sequence play. Bad chunk id[");
                        //Console::Info()<<(curNote->sound); Console::Info()<<("]");
                    } else {
                        AddDelay(cur_Chord->GetChannel(), curNote->sound, curNote->volume, curTime + addedTime + delay, curNote->panning, curNote->duration *((seq->beatsPerMeasure) / (seq->beatsPerMinute / 60.0 )));
                    }
                }

        }

    }




    return playbackStart;
}





void Sequencer::SkipPlayback(int millisecs) {
    if (!IsPlaying()) return;

    //Console::Info()  << "[Sound] Skipped playback by " << millisecs / 1000.0 << "s"<< Console::End;

    std::vector<Delay *> toRemove;
    for(auto i = soundQueue->begin(); i != soundQueue->end(); ++i) {
        Delay * target = *i;
        if (playbackProgress + millisecs > target->delay) {
            toRemove.push_back(target);
        }
    }

    for(uint32_t i = 0; i < toRemove.size(); ++i) {
        soundQueue->erase(soundQueue->find(toRemove[i]));
    }
    totalPauseTime -= millisecs;
    playbackProgress = (Time::MsSinceStartup() - GetPlaybackStart()) - totalPauseTime;

}





void Sequencer::AddDelay(uint32_t channel, AssetID data, uint8_t volume, double delaySeconds, uint8_t panning, double duration) {
    Delay * newSound = new Delay();
    newSound->channel = channel;
    newSound->data = data;
    newSound->volume = volume;
    newSound->delay = delaySeconds * 1000;
    newSound->leftPanning = (256 - panning) / 256.0 ;
    newSound->rightPanning = (panning) / 256.0 ;

    int durationFrames = duration * Sound::GetManager()->GetSampleRate();
    int sizeFrames = Assets::Get<AudioBlock>(data).NumBytes();


    if (durationFrames < sizeFrames) {
        newSound->duration = durationFrames;
        //Console::Info()<<("Queued delay with modified duration ");
        //Console::Info()<<(durationFrames); Console::Info()<<(" frames instead of default size ");
        //Console::Info()<<(sizeFrames); Console::Info()<<(" frames ");
    } else {
        newSound->duration = sizeFrames;
        //Console::Info()<<("Queued delay with default duration ");
        //Console::Info()<<(sizeFrames); Console::Info()<<(" frames instead of suggested size ");
        //Console::Info()<<(durationFrames); Console::Info()<<(" frames ");
    }


    soundQueue->insert(newSound);
}




void Sequencer::Start() {
    if (lockPlayback) return;
    if (IsPlaying()) Stop();

    //Console::Info()  << "[SOUND] Starting playback" << Console::End;



    playbackStart = Time::MsSinceStartup();

    playbackProgress = 0;
    pauseTime = 0;
    pauseStartTime = 0;
    totalPauseTime = 0;
    pause = false;

}


Delay::Delay() {
    createTime = Time::MsSinceStartup();
    data = AssetID();
    volume = 128;
    delay = 0;
    leftPanning = .5;
    rightPanning = .5;
}

int Delay::getCreateTime() {
    return createTime;
}

bool Sequencer::IsPlaying() {
    return playbackActive;
}

bool Sequencer::IsPaused() {
    return pause;
}

void Sequencer::Stop() {
    if (!IsPlaying()) return;

    //Console::Info()  << "[SOUND] Stopped playback" << Console::End;
    playbackStart = 0; // send initial signal for playback stopping.

    for(auto i = soundQueue->begin(); i != soundQueue->end(); ++i) {
        delete *i;
    }
    soundQueue->clear();

    playbackActive = false;

}


void Sequencer::OnStep() {

    bool finishedTrying = false;
    Delay * delay;
    while (soundQueue->size() && !finishedTrying) {
        delay = *soundQueue->begin();
        if (Time::MsSinceStartup() - playbackStart >= delay->delay) {
            Sound::PlayAudio(
                delay->data,
                delay->channel,
                delay->volume,
                delay->rightPanning
            );
            soundQueue->erase(soundQueue->begin());
        } else {
            finishedTrying = true;
        }
    }
}




