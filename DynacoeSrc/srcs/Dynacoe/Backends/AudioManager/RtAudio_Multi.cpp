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

#if defined(DC_BACKENDS_RTAUDIO_WIN32) || defined(DC_BACKENDS_RTAUDIO_ALSA) || defined(DC_BACKENDS_RTAUDIO_OSS)

#include <Dynacoe/Backends/AudioManager/RtAudio_Multi.h>
#include <Dynacoe/Util/Math.h>
#include <iostream>

using namespace Dynacoe;
using std::vector;
using std::string;
using std::queue;

using std::cout;
using std::endl;

const int BufferCount_c = 4;





RtAudioManager::~RtAudioManager() {
}


bool RtAudioManager::Connect(AudioStreamHandler * handler) {
    handlerMain = handler;
    OpenStream();        
    EnableOutput(true);
    return connected;
}

void RtAudioManager::SetSampleRate(uint32_t) {
    sampleRate = 44100;
    CloseStream();
    OpenStream();
     // reopen stream with new sample rate
}

uint32_t RtAudioManager::GetSampleRate() {
    return sampleRate;
}






bool RtAudioManager::Underrun() {
    //if (isUnderrun)
        //cout << "[Dynacoe::RtAudio]: " << "Underrun detected." << endl;
    return isUnderrun;
}

void RtAudioManager::EnableOutput(bool doIt) {
    doIt?StartStream():StopStream();
}

void RtAudioManager::SetVolumeMultiplier(float f) {
    volume = f;
    Math::Clamp(f, 0.f, 1.f);
}

float RtAudioManager::GetVolumeMultiplier() {
    return volume;
}






float RtAudioManager::GetCurrentOutputSample() {
    return lastSample;
}






/* internal */

const int RtAudio_n_internal_buffers_c = 1;
#ifndef DC_BACKENDS_RTAUDIO_WIN32
    const int RtAudio_n_internal_buffer_size_c = 256;
#else   
    // not sure what to do here. 256 seems fine on some machines, while 
    // on others it induces some kind of desync clicking? This seems more like a 
    // math problem more than enything else. For now, 512 it is
    const int RtAudio_n_internal_buffer_size_c = 512;
#endif
RtAudioManager::RtAudioManager() {

    rtParams.nChannels = 2;
    rtParams.firstChannel =  0;    

    rtOpts.numberOfBuffers = RtAudio_n_internal_buffers_c;
    rtOpts.streamName = "Dynacoe-RtAudioStream";


    
    sampleRate = 44100;
    volume = 1.f;
    enabled = false;

    #ifdef DC_BACKENDS_RTAUDIO_WIN32
        rtAudio = new RtAudio(RtAudio::WINDOWS_DS);
        rtOpts.flags = RTAUDIO_SCHEDULE_REALTIME;
        rtOpts.priority = 0;
    #elif defined(DC_BACKENDS_RTAUDIO_ALSA)
        rtAudio = new RtAudio(RtAudio::LINUX_ALSA);
        rtOpts.flags = RTAUDIO_SCHEDULE_REALTIME | RTAUDIO_ALSA_USE_DEFAULT;
        rtOpts.priority = 0;

    #elif defined(DC_BACKENDS_RTAUDIO_OSS)
        rtAudio = new RtAudio(RtAudio::LINUX_OSS);
    #endif
    rtParams.deviceId = rtAudio->getDefaultOutputDevice();
    connected = false;

    
}



void RtAudioManager::OpenStream() {
    if (connected) {
        return;

    }
    try {
        unsigned int bufferSize = RtAudio_n_internal_buffer_size_c;
        cout << "[Dynacoe::RtAudio]: " << "Attempting to open stream..." << endl;

        rtAudio->openStream(
            &rtParams,
            nullptr,
            RTAUDIO_FLOAT32, // yeah floating points!
            sampleRate,
            &bufferSize, 
            StreamCallback,
            (void *)this,
            &rtOpts
        );
           
        connected = true;  
        //cout << "[Dynacoe::RtAudio]: " << "Stream opened" << endl;       
    } catch(RtAudioError & e) {
        connected = false;
        cout << "[Dynacoe::RtAudio]: " << "Stream open failed:" << e.getMessage() << endl;
        //rtAudio->closeStream();
    }
}

void RtAudioManager::CloseStream() {
    try {
        if (connected) {
            rtAudio->closeStream();
            //cout << "[Dynacoe::RtAudio]: " << "Stream closed successfully" << endl;
        }
    } catch(RtAudioError & e) {
        cout << "[Dynacoe::RtAudio]: " << "Stream close failed!" << endl;
    }
    connected = false;
}

void RtAudioManager::StartStream() {
    if (enabled) return;

    
    rtAudio->startStream();
    cout << "[Dynacoe::RtAudio]: " << "Stream started" << endl;
    enabled = true;
}

void RtAudioManager::StopStream() {
    if (!enabled) return;

    cout << "[Dynacoe::RtAudio]: " << "Stream stopped" << endl;
    rtAudio->stopStream();
    enabled = false;
}







float RtAudioManager::GetUnderrunSample() {
    return 0.f;
}

int RtAudioManager::StreamCallback(
  void * oBuffer,
  void * iBuffer,
  unsigned int nBufferFrames,
  double streamTime,
  RtAudioStreamStatus status,
  void * userData) {
    

    if (status & RTAUDIO_INPUT_OVERFLOW) {
        cout << "Audio manager: internal overflow!" << endl;
        return 0;
    } else if (status & RTAUDIO_OUTPUT_UNDERFLOW) {
        cout << "Audio manager: output underflow! (framerate too low to compensate?): " << nBufferFrames << endl;
    }


    RtAudioManager * audio = (RtAudioManager*) userData;
    (*audio->handlerMain)(
        nBufferFrames*2,
        (float*)oBuffer
    );
    
    /*
    float * targetBuffer = (float*)oBuffer;   
    uint32_t underrunFloats = nBufferFrames*2 - 
        audio->streamQueue->PopData(
            targetBuffer,
            nBufferFrames*2
        )
    ;
    

    if (underrunFloats) {
        for(int i = nBufferFrames*2-underrunFloats; i < nBufferFrames*2; ++i) {
            targetBuffer[i]   = audio->GetUnderrunSample();
        }
        //cout << "UNDERRUNs!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

    }

    audio->isUnderrun = underrunFloats;  
    */

    return 0;
  
}

std::string RtAudioManager::Name() {return "RtAudio (c) 2001-2014 Gary P. Scavone";}
std::string RtAudioManager::Version() {return "v1.0 (RtAudio ver. 4.1.1)";}
bool RtAudioManager::Valid() {return true;}




#endif






