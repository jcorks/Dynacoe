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


#ifndef H_SEQUENCE_EXAMPLE_INCLUDED
#define H_SEQUENCE_EXAMPLE_INCLUDED

#include <Dynacoe/Library.h>
#include <Dynacoe/Sequence.h>
#include <Dynacoe/Components/Sequencer.h>
using namespace Dynacoe;

class SequenceExample : public Dynacoe::Entity {
  public:
    SequenceExample() {
        AddComponent(&sequencer);

        // Load the sounds as before
        kick  = Assets::Load("wav", "kick.wav");
        snare = Assets::Load("wav", "snare.wav"); 
        hat   = Assets::Load("wav", "hat.wav");
        clave = Assets::Load("wav", "clave.wav");
                
        
        // Create a sequence.
        // A sequence is essentially a song.
        // Sequences are broken up in to measures, which 
        // are consist of Chords made up of individual Notes, and notes 
        // actually are the sounds that will play.
        // This allows for flexible and general music creation with individual sounds.
        Sequence s("Example");

        
        Sequence::Measure beat = 
            Dynacoe::Sequence::Measure({
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(kick)}, 4*0/16.f)},
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(kick)}, 4*5/16.f)},
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(kick)}, 4*6/16.f)},
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(kick)}, 4*14/16.f)},

                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(snare)}, 4*4/16.f)},
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(snare)}, 4*12/16.f)},
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(snare)}, 4*13/16.f)},
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(snare)}, 4*15/16.f)},

                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(clave)}, 4*2/16.f)},
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(clave)}, 4*7/16.f)},
                {Dynacoe::Sequence::Chord({Dynacoe::Sequence::Note(clave)}, 4*9/16.f)},

            });

        
        // we going to repeat the beat a few times
        s.AddMeasures({beat});
        s.AddMeasures({beat});
        s.AddMeasures({beat});
        s.AddMeasures({beat});

        
        // You need to actually queue the sequence before starting.
        // You can queue any number of sequences and they will play in succession.
        sequencer.Queue(s);
        sequencer.Start();

    }



  private:
    Dynacoe::Sequencer sequencer;

    Dynacoe::AssetID kick;
    Dynacoe::AssetID snare;
    Dynacoe::AssetID hat;
    Dynacoe::AssetID clave;
};

#endif
