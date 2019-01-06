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

#ifdef ANDROID
#ifndef H_DC_ABAUDIO_MANAGER_INCLUDED
#define H_DC_ABAUDIO_MANAGER_INCLUDED

// handles the nitty-gritty of audio.
// THe AudioManager must continuously process any pushed data.
// Under most implementations, a threaded approach would be most appropriate.

#include <string>
#include <vector>
#include <Dynacoe/Backends/AudioManager/AudioManager.h>
namespace Dynacoe {
class ABAudioData;
class ABAudio : public Dynacoe::AudioManager {
  public:
    ABAudio();
    ~ABAudio();


    // Makes the connection to hardware to attempt to
    // bring the backend to a usable state. Returns whether or 
    // not the connection was successfully made. Else,
    // it is recommended to try again.
    bool Connect();

    // Sets the number of samples second.
    // The default is 44100.
    void SetSampleRate(uint32_t);

    // returns the current sample rate in kHz
    uint32_t GetSampleRate();

    // Queues data to be interpreted by the AudioManager. The format is in
    // 2 channel, interleaved form, meaning that the samples should be organized
    // in an interleaved channel format. In most device setups, the 
    // odd samples are for the left speaker, and the
    // even samples are for the right speaker
    // each sample is a floating point number
    // and expected to be normalized between -1 and 1.
    void PushData(float * data, uint32_t numSamples);


    // returns the number of samples pending to be consumed
    uint32_t PendingSamplesCount();



    // Returns whether or not the AudioManager is in an underrun state
    bool Underrun();

    // Enables or disables output
    void EnableOutput(bool doIt);
    

    // Set and Get the volume multipler. This value
    // is clamped to 0 and 1 and represents a scale
    // applied to all samples before interpretation.
    // the default is .5;
    void SetVolumeMultiplier(float);
    float GetVolumeMultiplier();


    float GetCurrentOutputSample();

    std::string Name() {return "Android OpenSL";}
    std::string Version() {return "1.0";}
    bool Valid() {return true;}
  private:
    ABAudioData * ABA;
};
}
#endif
#endif
