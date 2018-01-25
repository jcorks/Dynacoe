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


#include <Dynacoe/AudioBlock.h>
#include <Dynacoe/Util/Math.h>


#include <cmath>
#include <cstring>

using namespace Dynacoe;
using std::string;
static const int DYNACOE_SAMPLE_RATE =        44100;       // (Samples processed per second)

AudioBlock::AudioBlock(const string & n) : Asset(n){
    data = NULL;
    index = AssetID();
    path = "";
    volume = 1.f;
    size = 0;
    leftPanning = .5;
    rightPanning = .5;

}



AudioBlock::~AudioBlock() {
    if (data)
    delete[] data;
}

void AudioBlock::Define(AudioSample * src, uint32_t numSamples) {
    

    if (data) delete[] data;
    data = (char *)(new AudioSample[numSamples]);
    memcpy(data, src, numSamples*sizeof(AudioSample));
    size = numSamples*sizeof(AudioSample);
}

void AudioBlock::SetVolume(float v) {
    Math::Clamp(v, 0.f, 1.f);
    volume = v;
}

void AudioBlock::SetPanning(float v) {
    Math::Clamp(v, 0.f, 1.f);
    leftPanning = 1 - v;
    rightPanning = v;
}

float AudioBlock::GetVolume() {
    return volume;
}

float AudioBlock::GetPanning() {
    return rightPanning;
}

uint32_t  AudioBlock::NumBytes() {
    return size;
}

uint32_t  AudioBlock::NumSamples() {
    return NumBytes() / sizeof(AudioSample);
}

AudioBlock * AudioBlock::GetSubBlock(uint32_t  begin, uint32_t end) {
    if (begin == end) return NULL;
    if (begin > end) {
        int temp = begin;
        begin = end;
        end = temp;
    }

    if (end >= NumSamples()) end = NumSamples() - 1;

    AudioBlock * out = new AudioBlock(GetAssetName() + "@SubBlock");
    out->size = (end-begin+1) * sizeof(AudioSample);
    char * tempData = new char[out->size];
    memcpy(tempData,
           data + sizeof(AudioSample) * begin,
           out->size);
    out->index = AssetID();
    out->path = "";
    return out;
}

void AudioBlock::AppendBlock(AudioBlock * block) {
    InsertBlock(NumSamples() - 1, block);
}

void AudioBlock::RemoveBlock(uint32_t begin, uint32_t end) {
    if (begin == end) return;
    if (begin > end) {
        int temp = begin;
        begin = end;
        end = temp;
    }
    if (begin < 0)        begin = 0;
    if (end >= NumSamples()) end = NumSamples() - 1;

    // the block size refers to the deadwight block's size;
    int blockSize = (end - begin) * sizeof(AudioSample);
    int newSize = NumBytes() - blockSize;




    char * newData = new char[newSize];

    memcpy(newData,
           data,
           (begin) * sizeof(AudioSample));
    // we copy all around the deadweight's block
    memcpy(newData + (begin) * sizeof(AudioSample),
           data    + (begin) * sizeof(AudioSample) + blockSize,
           newSize - (begin) * sizeof(AudioSample));

    delete[] data;
    data = newData;
    size = newSize;
}

void AudioBlock::InsertBlock(uint32_t begin, AudioBlock * block) {
    if (begin < 0)  begin = 0;
    if (begin >= NumSamples())  begin = NumSamples() - 1;

    int newSize = NumBytes() + block->NumBytes();
    char * newData = new char[NumBytes() + block->NumBytes()];
    memcpy(newData,
           data,
           NumBytes());
    memcpy(newData + NumBytes(),
           block->data,
           block->NumBytes());
    delete[] data;
    data = newData;
    size = newSize;


}


float AudioBlock::GetSecondsFromSamples(uint32_t  sample) {
    if (sample >= NumSamples()) return -1;
    return (sample / ((float) DYNACOE_SAMPLE_RATE)) / 2.f;
}

uint32_t AudioBlock::GetSampleFromSeconds(float seconds) {
    int sample = seconds * DYNACOE_SAMPLE_RATE * 2;
    if (sample < 0 || sample >= NumSamples()) return -1;
    return sample;
}

void AudioBlock::EditSample(uint32_t sample, AudioSample value) {
    if (sample >= NumSamples()) return;
    ((AudioSample *)data)[sample] = value;
}

AudioSample AudioBlock::GetSample(uint32_t sample) {
    if (sample >= NumSamples()) return AudioSample(0, 0);
    return ((AudioSample *)data)[sample];
}


//void AudioBlock::Append(vector<AudioSample> & nData) {
void AudioBlock::Append(AudioSample * nData, uint32_t nData_size) {
    uint32_t newSize = size + nData_size*sizeof(AudioSample);
    char *   newData = new char[newSize];
    if (size) {
        memcpy(newData, data, size);
        delete[] data;
    }
    memcpy(newData + size, &nData[0], nData_size*sizeof(AudioSample));
   
    data = newData;
    size = newSize;

}



AudioBlock * AudioBlock::CreateSine(float pitch, float roughness, uint8_t volume, float duration_sec, int compound) {
    uint32_t chunkSize = ceil(DYNACOE_SAMPLE_RATE * duration_sec);
    chunkSize += chunkSize % sizeof(AudioSample);
    AudioBlock * sample = new AudioBlock("Internal_AudioSample@??");
    sample->data = new char[chunkSize];
    sample->size = chunkSize;
    sample->volume = volume;





    for(uint32_t n = 0; n < chunkSize; n++) {
        sample->data[n] =  ((chunkSize - n) / (float) chunkSize) * pow(sin(Math::Pi() / 2 * n / (22050.0 / pitch)),compound)*40*roughness;
        //sample->data[n] = 0;
    }

    return sample;
}



AudioBlock * AudioBlock::CreateHit(float pitch, float impact, float roughness, uint8_t volume, int compound) {
    uint32_t chunkSize = ceil(DYNACOE_SAMPLE_RATE * 1);
    chunkSize += chunkSize % sizeof(AudioSample);
    AudioBlock * sample = new AudioBlock("Internal_AudioSample@??");
    sample->data = new char[chunkSize];
    sample->size = chunkSize;
    sample->volume = volume;





    for(uint32_t n = 0; n < chunkSize; n++) {
        sample->data[n] =  pow(chunkSize / (n + 0.0) * impact, compound) + (RAND_MAX / (float)rand()) * roughness;
    }

    return sample;
}







