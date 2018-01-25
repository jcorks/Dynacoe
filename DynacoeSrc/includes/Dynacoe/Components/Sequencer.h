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


#ifndef H_DC_COMPONENT_SEQUENCER_INCLUDED
#define H_DC_COMPONENT_SEQUENCER_INCLUDED


#include <Dynacoe/Component.h>
#include <Dynacoe/Library.h>
#include <vector>



namespace Dynacoe {
class Delay;

class Sequencer : public Component {

  public:
    Sequencer();
    ~Sequencer();
    /// \brief Pause playback of all sequences.
    ///
    void Pause();

    /// \brief Resume playback of all sequences.
    ///
    void Resume();
    
    /// \brief Returns the time since playback begun.
    ///
    uint32_t GetPlaybackTime();

    /// \brief Returns when playback was started.
    ///
    uint32_t GetPlaybackStart();

    /// \brief Returns whether or not playback is paused.
    ///
    bool IsPaused();

    /// \brief Plays all sequences queued with QueueSequence().
    ///
    /// The sequence is played back in the state that it was in 
    /// at the time of the call to QueueSequence().
    void Start();

    /// \brief Stops all playback.
    ///
    /// All Sequence information queued is lost.
    void Stop();

    /// \brief Returns whether or not sequences are currently being played.
    ///
    bool IsPlaying();

    /// \brief Queues the sequence for playback after a specified number of seconds.
    ///
    int Queue(const Sequence &, double delay = 0.f);

    /// \brief returns the duration of the currently stored set of notes in seconds.
    ///
    int GetDuration();

    /// \brief skips the playback forward the specified number of seconds.
    ///
    void SkipPlayback(int millisecs);



    void OnStep();
  private:

    

    std::vector<Delay*> playbackTrash;
    std::vector<Delay*> playbackTrash_Wrapped;



    std::set <Delay *, bool(*)(const Delay *, const Delay *)> * soundQueue;
    void sortDelays();
    void AddDelay(uint32_t channel, AssetID, uint8_t volume, double delaySeconds, uint8_t panning,double duration);

    bool playbackActive;
    bool lockPlayback;
    bool pause;

    uint32_t playbackProgress;
    uint32_t playbackStart;
    uint32_t pauseTime;
    uint32_t pauseStartTime;
    uint32_t totalPauseTime;



};
}
#endif
