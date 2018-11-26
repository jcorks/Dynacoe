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

#ifndef H_SOUND_INCLUDED
#define H_SOUND_INCLUDED



#include <Dynacoe/Backends/AudioManager/AudioManager.h>
#include <Dynacoe/AudioSample.h>
#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/AudioEffect.h>
#include <Dynacoe/Sequence.h>
#include <Dynacoe/Util/Table.h>


class AudioClient;

namespace Dynacoe {
class AudioManager;
class AudioBlock;



/// \brief Allows for interaction with a playing sound.
///
/// An ActiveSound represents a sample of audio that is currently
/// undergoing playback. As such, you can use the ActiveSound to modify how the
/// corresponding audio is playing. Because typical playback systems are done on
/// separate threads, the reflection of changes caused by the calling of
/// these functions is not immediately guaranteed.
///
/// ActiveSound objects are produced by Sound::PlayAudio
class ActiveSound {
  public:
    ActiveSound();
    ActiveSound(const ActiveSound &);
    ActiveSound & operator=(const ActiveSound &);
    ActiveSound(LookupID stateID);

    /// \brief Sets the volume of the ActiveSound.
    ///
    /// @param v The new volume to use. The value is clamped to the range 0.f and 1.f.
    void SetVolume(float v);

    /// \brief Sets the panning.
    ///
    /// @param v The new panning. The value is clamped to the range 0.f and 1.f.
    void SetPanning(float v);

    /// \brief Sets whether the sound should be replayed once it finishes.
    ///
    /// @param b If true, the sound will repeat. If false, the sound will not repeat.
    /// The default is not to repeat.
    void SetRepeat(bool b);

    /// \brief Sets the progress in the sound to play from.
    ///
    /// @param f The place in the sound to continue playback from.
    /// f is interpreted as a fraction of the progress of the sound,
    /// where 0 is the beginning of the sound and 1 is the end of the sound.
    /// The value is clamped to this range.
    void Seek(float f);

    /// \brief Alters the channel that the sound plays on.
    ///
    /// @param i The new channel to play from.
    void SetChannel(uint32_t i);

    /// \brief Halts the sound.
    ///
    void Stop();

    /// \brief Pauses the sound if it was playing.
    ///
    void Pause();

    /// \brief Resumes the sound's playback if it was Pause()ed.
    ///
    void Resume();

    /// \brief Returns whether this ActiveSound actually refers to something that is playing back.
    ///
    /// If the sound has Stop()ed or finished on its own,
    /// the corresponding ActiveSound will no longer have an effect
    /// on anything. It is not an error to use an ActiveSound if
    /// it does not refer to an actual sound.
    bool Valid();

  private:

    uint8_t * Updated();

    LookupID state;
};


/// \brief Module that handles all audio related functionality.
///
namespace Sound {




    /// \brief Returns whether or not an instance of the sample is playing
    ///
    bool IsPlaying(AssetID index);


    /// \brief Queues audio for immediate playback
    ///
    /// @param effectChannel The channel to send the audio block to play on.
    /// @param panning The panning for the sound. 0.f is all the way to the left, 1.f is all the way to the right
    /// @param volume The volume the sound should play at.
    ActiveSound PlayAudio(AssetID, uint8_t effectChannel=0, float volume = 1.f, float panning = .5f);







    /* Channels */
    /**
      * \name Channels
      * All audio plays through a number of Channels.
      * Think of a channel as a set of independent audio systems:
      * each channel can have its own set of effects applied to it, can
      * have a specific volume applied, or panning applied. A common set up
      * would be to have a separate channel for Sound Effects,
      * a separate channel for "Background Music", and perhaps another for "Voice Audio".
      *
      * AudioEffects are functors that process and modify raw
      * audio data. AudioEffects are ran on each sample that is played on
      * its owning channel. AudioEffects are ran on samples before it is
      * passed to the mixer for processing.
      *
      * In the case that effect processing is desired for all samples,
      * It is possible to add / remove effects to / from the mixer directly via
      * the *MasterEffect* functions.
    **/
    //{

    /// \brief Registers an AudioEffect instance with an associated
    /// effect channel. All further samples that play on the given channel
    /// will have this effect applied.
    ///
    void ChannelAddEffect(const AudioEffect *, uint8_t channel);

    /// \brief Unregisters an AudioEffect instance.
    ///
    void ChannelRemoveEffect(const AudioEffect *, uint8_t channel);

    /// \brief Removes all effects from a channel.
    /// This does not delete the AudioEffects given to the channel.
    void ChannelReset(uint8_t channel);
    
    /// \brief Keeps the stream active for the given channel.
    /// By default, channels will only be active if there are samples playing through it 
    /// This is normally fine, execpt this means effects will stop once no samples are 
    /// being processed through a channel. For something like reverberation, this 
    /// would cut off meaningful information from being output. Keeping the channel 
    /// awake sacrifices performance for making more quality  
    void ChannelKeepAwake(uint8_t, bool doIt);

    /// \brief Sets the volume for the given channel
    /// 0.f denotes minimum volume and 1.f maximum.
    /// The values are clipped if they are beyond these bounds.
    ///
    void ChannelSetVolume(uint8_t channel, float);

    /// \brief Sets the panning for the entire channel.
    /// 0.f denotes all the way to the left and 1.f all the way to the right.
    /// The values are clipped if they are beyond these bounds.
    ///
    void ChannelSetPanning(uint8_t channel, float);
    ///}


    AudioManager * GetManager();


    void Init();
    void InitAfter();
    void RunAfter();




};





};











#endif
