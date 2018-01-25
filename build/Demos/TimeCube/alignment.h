/*

Copyright (c) 2016, Johnathan Corkery. (jcorkery@umich.edu)
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


#ifndef H_ALIGNMENT_INCLUDED
#define H_ALIGNMENT_INCLUDED

#include <Dynacoe/Library.h>
#include <cmath>

/* Contains information on who's who */

class Alignment : public Dynacoe::Component {
  public:

    using ID = Dynacoe::LookupID ;



    Alignment() : Component("Alignment"){}


    // Generates a new faction identified by ID
    static ID New() {
        AlignmentState * newState = new AlignmentState;
        newState->color = {
            (.2f + Dynacoe::Random::Value()*.8f),
            (.2f + Dynacoe::Random::Value()*.8f),
            (.2f + Dynacoe::Random::Value()*.8f),
            1.f
        };



        newState->pts = GenerateFactionShape();

        return table.Insert(newState);

    }



    Dynacoe::Color Color() {
        return table.Find(id)->color;
    }

    ID GetID() {
        return id;
    }


    std::vector<Dynacoe::Point> GetShape() {
        return table.Find(id)->pts;
    }


    void Change(ID f) {
        id = f;
    }





    void OnAttach() {
        // always have an alignment in a valid state
        if (defaultID == ID())
            defaultID = New();
        id = defaultID;
    }

  private:

    // unit shape
    static std::vector<Dynacoe::Point> GenerateFactionShape() {
        std::vector<Dynacoe::Point> pts;


        // symmetrical, facing 0-degrees
        Dynacoe::Point anchorBottom = RandomSrcPoint();
        Dynacoe::Point anchorTop    = RandomSrcPoint();

        anchorBottom.y = 0;
        anchorTop.y = 0;


        pts.push_back(anchorBottom);
        pts.push_back(RandomSrcPoint());
        for(int i = 0; i < 3; ++i) {
            pts.push_back(pts[pts.size()-1]);
            pts.push_back(RandomSrcPoint());
        }
        pts.push_back(pts[pts.size()-1]);
        pts.push_back(RandomSrcPoint());




        // then reflect over y axis
        int origNum = pts.size();
        Dynacoe::Point curPt;
        for(int i = 0; i < origNum; ++i) {
            curPt = pts[i];
            curPt.y = -1*curPt.y;
            pts.push_back(curPt);
        }
        return pts;

    }


    static Dynacoe::Point RandomSrcPoint() {
        Dynacoe::Point out;
        out.x =      Dynacoe::Random::Value() *10;
        out.y = fabs(Dynacoe::Random::Value())*10;
        out.z = 0.f;
        return out;
    }


    ID id;
    struct AlignmentState {
        Dynacoe::Color color;
        std::vector<Dynacoe::Point> pts;
    };


    static Dynacoe::Table<AlignmentState*> table;
    static ID defaultID;
};

Dynacoe::Table<Alignment::AlignmentState*> Alignment::table;
Alignment::ID Alignment::defaultID;
#endif
