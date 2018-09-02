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


#include <Dynacoe/Modules/Sound.h>

#include <Dynacoe/Components/Clock.h>
#include <Dynacoe/Util/Filesys.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Util/Time.h>
#include <Dynacoe/AudioBlock.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Components/Mutator.h>

#include <set>
#include <cassert>
#include <cmath>
#include <sstream>
#include <cstring>

#define WAV_HEADER_SIZE 44
#define AudioSample_LIMIT 32767
#define BUFFER_SIZE 2048
#define CHANNEL_COUNT 32
#include <cstdio>


using namespace Dynacoe;
using namespace std;

static float panning_to_multiplier_l(float panning) {
    float leftPan = (2 - panning*2);
    Math::Clamp(leftPan, 0.f, 1.f);
    return leftPan;
}

static float panning_to_multiplier_r(float panning) {
    float rightPan = panning*2;
    Math::Clamp(rightPan, 0.f, 1.f);
    return rightPan;
}


#include "Sound_StateArray.hpp"
#include "Sound_AudioStreamObject.hpp"
#include "Sound_AudioEffectChannel.hpp"
#include "Sound_AudioProcessorIO.hpp"
#include "Sound_AudioProcessor.hpp"
#include "Sound_AudioClient.hpp"















AudioClient *      Sound::a;





static const int DYNACOE_SAMPLE_RATE =        44100;       // (Samples processed per second)












ActiveSound::ActiveSound() {
    state = LookupID();
}

ActiveSound::ActiveSound(const ActiveSound & other) {
    state = other.state;
}

ActiveSound & ActiveSound::operator=(const ActiveSound & other) {
    state = other.state;
    return *this;
}

ActiveSound::ActiveSound(LookupID id) {
    state = id;
    Updated();
}

void ActiveSound::SetVolume(float v) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    Math::Clamp(v, 0.f, 1.f);
    s->client.volume = v;
}

void ActiveSound::SetPanning(float p) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.panning = p;
}

void ActiveSound::SetRepeat(bool b) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.repeat = b;
}

void ActiveSound::Seek(float b) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    Math::Clamp(b, 0.f, 1.f);
    s->client.seek = b;
}

void ActiveSound::SetChannel(uint32_t t) {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.channel = t;
}

void ActiveSound::Stop() {
     AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    // TODO: client side stop
    s->client.volume = 0.f;
}

void ActiveSound::Pause() {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.pause = true;
}

void ActiveSound::Resume() {
    AudioStreamObject * s;
    if (!(s = (AudioStreamObject*)Updated())) return;
    s->client.pause = false;
}

bool ActiveSound::Valid() {
    return activeSounds.Query(state);
}

uint8_t * ActiveSound::Updated() {
    if (!state.Valid()) return nullptr;
    AudioStreamObject * out = activeSounds.Find(state);

    // could use an invalidation system in the future maybe.
    //out->needsUpdate = true;
    return (uint8_t*)out;
}







void Sound::Init() {

    static bool spawned = false;

    if (spawned) return;
    spawned = true;

    a = new AudioClient();

}

void Sound::InitAfter(){}

Backend * Sound::GetBackend() {
        return a->GetManager();
}

void Sound::RunBefore() {}
void Sound::RunAfter()  {
    a->UpdateMain();
}
void Sound::DrawBefore(){}
void Sound::DrawAfter() {}


















ActiveSound Sound::PlayAudio(AssetID i, uint8_t effectChannel, float volume, float panning) {
    if (!a) return ActiveSound(LookupID());
    AudioBlock * target = &Assets::Get<AudioBlock>(i);
    Math::Clamp(volume, 0.f, 1.f);
    Math::Clamp(panning, 0.f, 1.f);
    if (volume == 0.f) return ActiveSound(LookupID());
    if (target) {
        // play a chunk, any chunk
        return ActiveSound(a->PlayPCM(i, volume, panning, effectChannel));
    } else {
        Console::Error()<<("Request to play sound failed!")<< Console::End;
        return ActiveSound(LookupID());

    }


}


void Sound::ChannelSetVolume(uint8_t channel, float v) {
    a->ChannelSetVolume(channel, v);
}

void Sound::ChannelSetPanning(uint8_t channel, float v) {
    a->ChannelSetPanning(channel, v);
}



void Sound::ChannelAddEffect(const AudioEffect * ef, uint8_t ch) {
    a->AddEffect((AudioEffect *)ef, ch);
}

void Sound::ChannelRemoveEffect(const AudioEffect * ef, uint8_t ch) {
    a->RemoveEffect((AudioEffect *)ef, ch);
}

void Sound::ChannelReset(uint8_t ch) {
    a->Reset(ch);
}

void Sound::ChannelKeepAwake(uint8_t ch, bool ka) {
    a->KeepAwake(ch, ka);
}

























AudioManager * Sound::GetManager() {
    return a->GetManager();
}


uint32_t AudioClient::GetSampleRate() {
    uint32_t sampleRate = processor->manager->GetSampleRate();
    return sampleRate;
}


bool Sound::IsPlaying(AssetID index) {
    if (!a) return false;
    AudioBlock * addr = &Assets::Get<AudioBlock>(index);
    return a->IsSampleActive(addr);
}
