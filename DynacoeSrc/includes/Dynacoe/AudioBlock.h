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

#ifndef H_DC_AUDIOBLOCk_INCLUDED
#define H_DC_AUDIOBLOCk_INCLUDED


#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Modules/Sound.h>
#include <string>
#include <cstdint>

namespace Dynacoe {







/**  
 * \brief A generalized structure representing an audio waveform.
 *
 *   An audio waveform is defined in Dynacoe as a stream of samples,
 *   where samples are typically 2 to 4 bytes of
 *   information representing the amplitude of the
 *   waveform at that point. In Dynacoe,
 *   samples are 16-bit signed integers.
 *  Though, it is important to note that AudioBlocks represent stereo
 *  waveforms, where per sample, there exist two datum values, one for each channel. 
 *  As such, data for the waveform is
 *   interleaved to qualify both channels, where the subsequent sample
 *  corresponds to the channel opposite of the current one.
 */
class AudioBlock : public Asset {


  public:

    AudioBlock(const std::string &);
    ~AudioBlock();

    /// \brief  Clears the current data store of the AudioBlock and
    /// replaces it with the given data.
    ///
    /// @param data The audio samples that the AudioBlock will now consist of.
    /// @param numSamples The number of samples contained by data.
    void Define(AudioSample * data, uint32_t numSamples);

    /// \brief Sets volume according to a fraction.
    ///
    /// 0 corresponds to silence and
    /// 1.0 correspond to maximum volume. 
    /// Values outside this range are clamped.
    /// @param fraction The fraction of volume that AudioBlock
    /// will use. Clamped to the range 0.f - 1.f, where 0.f is 
    /// no sound and 1.f is original sample's volume.
    void SetVolume(float fraction);

    /// \brief Sets panning according to a fraction.
    ///
    /// 0 corresponds to left output channel only
    /// and 1.0 denotes right output channel only.
    /// Values outside this range are clamped.
    /// @param fraction The fraction of panning to use. Clamped to the range 0
    /// to 1, where .5 is even splitting between both channels.
    void SetPanning(float fraction);

    /// \brief Returns the current volume.
    ///
    float GetVolume();

    /// \brief Returns the current panning
    ///
    float GetPanning();
    
    

    /// \brief Returns the size in bytes stored in the AudioBlock.
    ///
    uint32_t NumBytes();

    /// \brief Similar to numBytes; returns the number of samples
    /// that the AudioBlock consists of.
    ///
    uint32_t NumSamples();

    /// \brief Returns a subsection of the AudioBlock as
    /// an independent AudioBlock.
    ///
    /// The new AudioBlock consists of all samples from firstSample to endSample.
    /// @param The first sample of the new block.
    /// @param The last sample of the new block.
    AudioBlock * GetSubBlock(uint32_t  firstSample, uint32_t endSample);


    /// \brief Appends the specified AudioBlock to the end
    /// of this one.
    ///
    /// @param block The block to have appended at the end of this block.
    void AppendBlock(AudioBlock * block);


    /// \brief Removes a section of samples.
    /// 
    /// @param beginSample The first sample to begin removal.
    /// @param endSample The last sample to remove. All in between will be removed.
    void RemoveBlock(uint32_t beginSample, uint32_t endSample);


    /// \brief Inserts the collection of samples at the specified sample.
    ///
    /// @param beginSample The sample of this AudioBlock to begin insertion at.
    /// @param block The AudioBlock to copy data from.
    void InsertBlock(uint32_t beginSample, AudioBlock * block);

    /// \brief Convert to seconds
    /// of the sample. 
    ///
    /// If the queried sample is beyond the extent of
    /// the AudioBlock, -1 is returned.
    /// @param i The sample index to retreive the time from.
    float GetSecondsFromSamples(uint32_t i);

    /// \brief Same as GetSecondsFromSamples, but the other way.
    /// @param f The time in seconds to ask about.
    uint32_t GetSampleFromSeconds(float f);


    /// \brief Changes the value of the specified sample.
    ///
    /// If the sample index given is invalid,
    /// no change to the waveform is made.
    /// @param sample The sample to edit.
    /// @param sampleValue The new value of the sample.
    void EditSample(uint32_t sample, AudioSample sampleValue);



    /// \brief Retrieves the current value of the waveform
    /// at the given sample index. 
    ///
    ///If the sample is
    /// invalid, no error is returned, but the value returned
    /// will always be 0 for this case.
    /// @param sample The sample to ask about.
    AudioSample GetSample(uint32_t sample);


    /// \brief Adds the specified samples to the wave form
    ///
    /// @param data The raw data to append.
    /// @param size The number of samples to append.
    void Append(AudioSample * data, uint32_t size);

    /// \brief Creates a new AudioBlock as a basic sine wave with the given parameters.
    /// Refer to PitchReference for hertz values.
    /// @param pitch The pitch of the sine wave to add in hertz.
    /// @param roughness The roughness of the sine wave. 
    /// @param volume The volume of the size wave.
    /// @param duration The duration of the AdioBlock in seconds.
    /// @param compound The compounding of the sine wave.
    static AudioBlock * CreateSine(float pitch, float roughness, uint8_t volume, float duration, int compound);


    /// \brief Creates a new AudioBlock as a basic noise distribution with the given parameters.
    ///
    /// @param pitch The pitch of the sine wave to add in hertz.
    /// @param roughness The roughness of the sine wave. 
    /// @param volume The volume of the size wave.
    /// @param duration The duration of the AdioBlock in seconds.
    /// @param compound The compounding of the sine wave.
    static AudioBlock * CreateHit(float pitch, float impact, float roughness, uint8_t volume, int compound);

  private:



    std::string path;
    AssetID index;
    char * data;
    uint32_t size;
    uint8_t volume;
    float leftPanning;
    float rightPanning;

};
}

#endif
