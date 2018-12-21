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

#include <Dynacoe/Components/Mutator.h>
#include <Dynacoe/Util/Random.h>
#include <Dynacoe/Util/Math.h>
#include <cmath>
#include <algorithm>

using namespace Dynacoe;


enum BindType {
    MTBind_Float,
    MTBind_Int,
    MTBind_UInt8,
    MTBind_UInt32,
    MTBind_UInt64
};

void Mutator::Bind(float & f, BindFunction n)    {Bind(&f, MTBind_Float, n);}
void Mutator::Bind(int & f, BindFunction n)      {Bind(&f, MTBind_Int, n);}
void Mutator::Bind(uint8_t & f, BindFunction n)  {Bind(&f, MTBind_UInt8, n);}
void Mutator::Bind(uint32_t & f, BindFunction n) {Bind(&f, MTBind_UInt32, n);}
void Mutator::Bind(uint64_t & f, BindFunction n) {Bind(&f, MTBind_UInt64, n);}
    
    
static void bind_fn_set(void * v, int type, float val) {
    switch(type) {
      case MTBind_Float:  *((float*)   v) = val; break;
      case MTBind_Int:    *((int*)     v) = val; break;
      case MTBind_UInt8:  *((uint8_t*) v) = val; break;
      case MTBind_UInt32: *((uint32_t*)v) = val; break;
      case MTBind_UInt64: *((uint64_t*)v) = val; break;
    }
}

static void bind_fn_add(void * v, int type, float val) {
    switch(type) {
      case MTBind_Float:  *((float*)   v) += val; break;
      case MTBind_Int:    *((int*)     v) += val; break;
      case MTBind_UInt8:  *((uint8_t*) v) += val; break;
      case MTBind_UInt32: *((uint32_t*)v) += val; break;
      case MTBind_UInt64: *((uint64_t*)v) += val; break;
    }
}

static void bind_fn_subtract(void * v, int type, float val) {
    switch(type) {
      case MTBind_Float:  *((float*)   v) -= val; break;
      case MTBind_Int:    *((int*)     v) -= val; break;
      case MTBind_UInt8:  *((uint8_t*) v) -= val; break;
      case MTBind_UInt32: *((uint32_t*)v) -= val; break;
      case MTBind_UInt64: *((uint64_t*)v) -= val; break;
    }
}

static void bind_fn_multiply(void * v, int type, float val) {
    switch(type) {
      case MTBind_Float:  *((float*)   v) *= val; break;
      case MTBind_Int:    *((int*)     v) *= val; break;
      case MTBind_UInt8:  *((uint8_t*) v) *= val; break;
      case MTBind_UInt32: *((uint32_t*)v) *= val; break;
      case MTBind_UInt64: *((uint64_t*)v) *= val; break;
    }
}

static void bind_fn_divide(void * v, int type, float val) {
    switch(type) {
      case MTBind_Float:  *((float*)   v) /= val; break;
      case MTBind_Int:    *((int*)     v) /= val; break;
      case MTBind_UInt8:  *((uint8_t*) v) /= val; break;
      case MTBind_UInt32: *((uint32_t*)v) /= val; break;
      case MTBind_UInt64: *((uint64_t*)v) /= val; break;
    }
}

    
void Mutator::Bind(void * v, int t, BindFunction fnChoice) {
    BindFunctionBase fn;
    switch(fnChoice) {
      case BindFunction::Set: fn = bind_fn_set; break;
      case BindFunction::Add: fn = bind_fn_set; break;
      case BindFunction::Multiply: fn = bind_fn_set; break;
      case BindFunction::Divide: fn = bind_fn_set; break;
    }
    bindVectors[t].push_back(std::pair<void*, BindFunctionBase>(v, fn));
    hasBinds = true;
}


void Mutator::Unbind(float & f)    {Unbind(&f, MTBind_Float);}
void Mutator::Unbind(int & f)      {Unbind(&f, MTBind_Int);}
void Mutator::Unbind(uint8_t & f)  {Unbind(&f, MTBind_UInt8);}
void Mutator::Unbind(uint32_t & f) {Unbind(&f, MTBind_UInt32);}
void Mutator::Unbind(uint64_t & f) {Unbind(&f, MTBind_UInt64);}

    
void Mutator::Unbind(void * v, int t) {
    std::vector<std::pair<void *, BindFunctionBase>> & b = bindVectors[t];
    for(uint32_t i = 0; i < b.size(); +i) {
        if (b[i].first == v) {
            b.erase(b.begin()+i);
            return;
        }
    }
}
    
Mutator::Mutator() : Component(),
      begin(0),
      isLooping(false),
      registr(new MutationState(0, Function::Constant, 0, 0, 0, false))
{
    SetTag("Mutator");
    bindVectors = new std::vector<std::pair<void*, BindFunctionBase>>[MTBind_UInt64+1];
    hasBinds = false;
    accountForLastStep = true;

}

Mutator::Mutator(float b, float end, Function f, float dur) : Component() {
    SetTag("Mutator");
    begin = b;
    isLooping = false;
    registr = new MutationState(0, Function::Constant, 0, 0, 0, false);
    states.insert(
        new MutationState(dur, f, end, 0, 0, false)
    );
    bindVectors = new std::vector<std::pair<void*, BindFunctionBase>>[MTBind_UInt64+1];
    hasBinds = false;
    accountForLastStep = true;

}

Mutator::~Mutator() {
    for(const MutationState * m : states) {
        delete m;
    }
    delete registr;
    delete[] bindVectors;
}




void Mutator::Clear(float beg) {
    begin = beg;
    states.clear();
    timer.Pause();

}

void Mutator::NewMutation(float when, float value, Function travelFunction) {
    states.insert(new MutationState(when, travelFunction, value, 0, 0, false));
}

void Mutator::NewRandomMutation(float when, float min, float max, Function travelFunction) {
    states.insert(new MutationState(when, travelFunction, 0, min, max, true));
}



void Mutator::SetMutationDestination(int index, float newVal) {
    auto it = (states.begin());
    for(int i = 0; i < index && it != states.end(); ++i, ++it){};
    if (it == states.end() || (*it)->isRandom) return;
    (*it)->value = newVal;
}






void Mutator::Start() {
    timer.Resume();
    timer.Set();
    accountForLastStep = false;
    /*
    for(auto iter = states.begin(); iter != states.end(); ++iter) {
        if (iter->isRandom
    }
    */
}

void Mutator::Stop() {
    timer.Set();
    timer.Pause();
    accountForLastStep = true;
}

void Mutator::Loop(bool b) { isLooping = b; }



bool Mutator::Expired() {

    return timer.GetTimeSince() >= (GetEnd() * 1000);
}

float Mutator::GetEnd() {
    if (states.empty()) return 0;
    auto iter = states.end();
    --iter;
    return (*iter)->time;
}

float Mutator::GetCurrentTime() {
    return timer.GetTimeSince() / 1000.f;
}


float Mutator::GetAt(float atTime) {

    if (atTime <= 0)
        return begin;

    registr->time = atTime;

    auto iter = lower_bound(states.begin(), states.end(), registr, CompareMutationState());
    auto prev = iter;
    if (prev != states.begin()) prev--;




    // breof
    if (iter == states.begin() && (*iter)->time >= atTime) {
        return compute(
            begin,
            (*iter)->value,
            (*iter)->func,
            ((atTime) / ((*iter)->time))
        );


    } else if (iter != states.end()) { // avg case

        return compute(
            (*prev)->value,
            (*iter)->value,
            (*iter)->func,
            ((atTime - (*prev)->time) / ((*iter)->time - (*prev)->time))
        );
    }


    return (*prev)->value;
}

float Mutator::Value() {
    return GetAt(GetCurrentTime());
}


void Mutator::OnStep() {
    if (states.empty()) return;
    bool exp = Expired();
    
    if (!exp || !accountForLastStep) {
        if (hasBinds) {
            float val = Value();
            for(int n = 0; n <= MTBind_UInt64; ++n) {
                std::vector<std::pair<void*, BindFunctionBase>> & binds = bindVectors[n];
                for(uint32_t i = 0; i < binds.size(); ++i) {
                    binds[i].second(binds[i].first, n, val);
                }
            }
        }
        if (exp)
            accountForLastStep = true;
    }
    if (exp && isLooping) {
        timer.Set();
        accountForLastStep = false;
    }
}



float Mutator::StepTowards(float current,
                           float dest,
                           float step,
                           Function f) {
    return compute(current, dest, f, step);
}


Vector Mutator::StepTowards(Vector current, Vector dest, float step, Function f) {
    return Vector(StepTowards(current.x, dest.x, step, f),
                 StepTowards(current.y, dest.y, step, f),
                 StepTowards(current.z, dest.z, step, f));
}


std::string Mutator::GetInfo() {
    return (Chain() << "Elapsed time:  " << GetCurrentTime() << "\n"
                    << "Current Value: " << Value() << "\n");
}



// progress is from 0 to 1, where 0 is begining, and 1 is finished
float Mutator::compute(float first, float end, Function func, float progress){

    switch(func) {
        case Function::Constant:          return end;
        case Function::Linear:            return first + (end - first)*progress;
        case Function::Quadratic:         return first + (end - first)*progress*progress;
        case Function::Cubic:             return first + (end - first)*.5f + (end - first)*(.5f * powf(progress*2.f - 1.f, 3));
        case Function::Sinusoidal:        return first + (end - first)*sin(progress * ((2.5f)*Math::Pi()));
        case Function::RandomDistribution:return first + (
                    (end-first>0?
                        Random::Spread(0, end - first) :
                        Random::Spread(end - first, 0)
            ));
        case Function::Logarithmic:       return first + (end - first)*((log2(1+(progress*63))) / (6.0));
    }
    return 0;
}
