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

#if(defined DC_BACKENDS_RTAUDIO_ALSA || defined DC_BACKENDS_RTAUDIO_OSS || defined DC_BACKENDS_RTAUDIO_WIN32)

#ifndef H_DC_RTAUDIO_MULTI_INCLUDED
#define H_DC_RTAUDIO_MULTI_INCLUDED


#include <Dynacoe/Backends/AudioManager/AudioManager.h>
#include <../srcs/Dynacoe/Backends/AudioManager/RtAudio/RtAudio.h>
#include <queue>
#ifdef DC_OS_WINDOWS
    #include <mutex>
#else
    #include <mutex>
#endif
#include <cstring>

namespace Dynacoe {
class StreamBuffer;
class RtAudioManager : public AudioManager {
  public:
    // standard interface
    bool Connect(AudioStreamHandler *);
    void SetSampleRate(uint32_t);
    uint32_t GetSampleRate();
    bool Underrun();
    void EnableOutput(bool doIt);
    void SetVolumeMultiplier(float);
    float GetVolumeMultiplier();
    float GetCurrentOutputSample();

    std::string Name();
    std::string Version();
    bool Valid();


    RtAudioManager();
    ~RtAudioManager();

  private:




    int inputBuffer;


    StreamBuffer * streamQueue;
    


    RtAudio * rtAudio;
    RtAudio::StreamParameters   rtParams;
    RtAudio::StreamOptions      rtOpts;
    AudioStreamHandler * handlerMain;

    /* Attributes */
    uint32_t sampleRate;
    bool enabled;
    bool connected;
    bool isUnderrun;
    float volume;
    float lastSample;


    
    float GetUnderrunSample();
    void OpenStream();
    void CloseStream();
    void StartStream();
    void StopStream();
    static int StreamCallback(void * buffer, void *, unsigned int, double, RtAudioStreamStatus, void *);


    std::recursive_mutex bufferMutex; // attempt to stop the bad
    

};
}

#endif
#endif
