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

#ifndef H_DC_SEQUENCE_INCLUDED
#define H_DC_SEQUENCE_INCLUDED

#include <Dynacoe/Modules/Assets.h>


namespace Dynacoe {

/// \brief Assets that contains information to play a song made of loaded audio samples.
///
/// Unless specified otherwise, all subclass Add/Get/Remove functions that take an
/// index i adhere to the following behavior: If i is less than 0 or is greater 
/// than the number of elements in the container,
/// the i is wrapped around. Thus, as long as at least one element exists, a
/// valid element will always be returned.
class Sequence : public Asset{
  public:

    /// \brief A discreet sound that plays for a specified amount of time.
    ///
    struct Note {
        Note();
        
        
        /// \brief Convenience constructor.
        ///
        /// @param duration_ Duration attribute in beats.
        /// @param sound_ AssetID refering to a loaded sound.
        /// @param volume_ Volume of the note. 255 is max volume while 0 is no ouptu sound.
        /// @param panning_ Panning of the note. 0 is output on only the left channel and 255 is only on the right channel.
        Note(AssetID sound_, float duration_ = 4.f, uint8_t volume_ = 128, uint8_t panning_ = 128) :
            duration(duration_),
            sound(sound_),
            volume(volume_),
            panning(panning_){}

        Note(const Note & n) :
            duration(n.duration),
            sound(n.sound),
            volume(n.volume),
            panning(n.panning){}

        /// \brief Duration of the sound in beats.
        ///
        float duration;

        /// \brief AssetID of the sound to be played.  
        ///
        AssetID sound;    
        
        /// \brief Volume of the note. 0 - 255: 0 is no sound while 255 is max volume.
        ///
        uint8_t volume;

        /// \brief Panning of the note. 
        ///
        uint8_t panning;
    };


    /// \brief A collection of notes that begin playing at the same time.
    ///
    class Chord {
      public:
        Chord(uint32_t channel = 0);
        Chord(const std::vector<Note>&, float startTime, uint32_t channel = 0);        

        /// \brief Returns the specified note.
        ///
        Note & GetNote(int n);
        
        /// \brief Returns the number of notes that belong to this chord.
        ///
        int GetNumNotes();

        /// \brief Adds a new note to be part of this chord.
        ///
        void AddNote(const Note &);

        /// \brief Removes the specified note from the chord.
        ///
        void RemoveNote(int n);

        /// \brief Sets when this chord is played. Measured in beats.
        ///
        void SetStartTime(float t);

        /// \brief Returns the start time.
        ///
        float GetStartTime();


        /// \breif Returns the Sound channel that this chord is set to 
        /// play on.
        uint32_t GetChannel();

        /// \brief Sets the Sound channel that this chord will play on.
        ///
        void SetChannel(uint32_t);







      private:
        float setTime;
        std::vector<Note> nList;
        int32_t channel;
    };


    /// \brief A collection of chords. All chord start times are offset
    /// from the start of this measure.
    class Measure {
        std::vector<Chord> cList;
        int index;
      public:
        Measure();
        Measure(const std::vector<Chord>&);

        /// \brief Returns the specified chord.
        ///
        Chord & GetChord(int i);
        
        /// \brief Returns the number of chords part of this measure.
        ///        
        int GetNumChords();
        
        /// \brief Adds the specified chord to this Measure.
        ///      
        void AddChord(const Chord &);

        /// \brief Removes the specified Chord.
        ///
        void RemoveChord(int i);


    };



    Sequence(const std::string & n);
    
    /// \brief The playback speed of the sequence.
    ///
    float beatsPerMinute;
    
    /// \brief The number of beats that each measure should be given.
    ///
    int beatsPerMeasure;
    
    /// \brief Defines what represents the beat. This is a fraction of the 
    /// while measure. For example, in 4/4 time, the beatDuration would be 1.f / 4.f
    /// because the quarter note would get the beat.    
    float beatDuration;





    /// \brief Returns the duration of the Sequence in seconds.
    ///
    float GetLength();

    /// \brief Returns the number of physical measures stored in the sequence
    ///
    int GetNumMeasures();

    /// \brief Returns the number of measures up to the last measure actully being utilized.
    /// i.e. the last measure that contains a note.
    ///    
    int GetNumMeasuresUsed();

    

    /// \brief Adds measures to the sequence.
    ///
    /// @param count Number of measures to add.
    void AddMeasures(const std::vector<Measure> &);

    /// \brief Returns the specified measure.
    ///
    Measure & GetMeasure(int i);

    /// \brief Remove the specified measure.
    ///
    void RemoveMeasure(int i);


  private:
    std::vector<Measure> mList;

};


}



#endif
