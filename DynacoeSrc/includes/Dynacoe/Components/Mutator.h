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

#ifndef H_DC_COMPONENT_MUTATOR_INCLUDED
#define H_DC_COMPONENT_MUTATOR_INCLUDED




#include <Dynacoe/Component.h>
#include <Dynacoe/Components/Clock.h>
#include <set>


struct MutationState;
class StateContainer;
namespace Dynacoe {


/// \ingroup Components
/// \{
/** \brief A piecewise function manager that changes inputs
 *  over time.
 *
 *  There are some instances where it is useful to model
 *  the transformation of a value in a complex manner. The Mutator component
 *  provides functionality for simple and complex easing. You can string easing functions
 *  together to achieve more complex behavior in one instance of a Mutator through
 *  the mutation interface.
 *  All times are delt with in seconds and
 *  the Mutator always begins at time 0.
*/
class Mutator : public Component {
  public:

    /// \brief For constructing the Mutator's mutations, you must specify
    /// a mutation function. These are the various available
    ///
    enum class Function {
        Constant,       ///< The state instantaneously changes to the next value
        Linear,         ///< The state linearly changes to the next value
        Quadratic,      ///< The state quadratically transforms to the next value
        SquareRoot,     ///< The state transforms in a reverse-quadratic manner to the next value
        Cubic,          ///< The state alters in a cubic manner to the next value
        Sinusoidal,     ///< The state sinusoidally changes to the next value
        Logarithmic,    ///< The state changes in a logarithmic manner to the next value
        RandomDistribution  ///< The state changes to random values between the state at the start and the end state.
    };

    Mutator();
    /// \brief Convenience constructor to set up the Mutator with one function.
    ///
    /// @param beginValue The value that the Mutator should start at.
    /// @param endValue The value that the Mutator should end at.
    /// @param f The mutation function to use.
    /// @param duration The duration of the mutation in seconds.
    Mutator(float beginValue,float endValue, Function f, float duration);
    ~Mutator();


    /// \brief Resets all mutations
    ///
    void Clear(float beginValue);

    /// \brief Sets a new point of mutation.
    ///
    /// When denotes when the Mutator will reach the given
    /// value, and travelFunction defines how this value is reached.
    void NewMutation(float when, float value, Function travelFunction);

    /// \brief Like NewMutation(), except the destination value is random chosen
    /// upon start(). min / max bound the random value.
    void NewRandomMutation(float when, float minValue, float MaxValue, Function travelFunction);


    /// \brief Sets a new destination value for the mutation.
    ///
    void SetMutationDestination(int mutationIndex, float newValue);



    /// \brief Begins the mutation lifetime.
    ///
    /// If it was already started, this
    /// function starts the simulation from the beginning.
    void Start();
    
    /// \brief Stops the mutation. 
    ///
    /// Stopping the mutation brings it to its end value.
    void Stop();

    /// \brief Sets whether or not to automatically restart
    /// after the final value has been reached.
    ///
    void Loop(bool);

    /// \brief Returns whether or not the terminal value has been reached.
    ///
    bool Expired();

    /// \brief Returns the mutator's progress in seconds.
    ///
    float GetCurrentTime();

    /// \brief Returns the duration of the mutation.
    ///
    float GetEnd();

    /// \brief Returns the Mutator's mutation value at a specified time.
    ///
    float GetAt(float time);

    /// \brief Returns the current mutation value result.
    ///
    float Value();
    
    
    /// \brief Tells how values should change when bound.
    ///
    enum class BindFunction {
        Set, ///< Updates the value with the current Value(), replacing the old one
        Add, ///< Adds the Value()
        Subtract, ///< Subtracts the Value()
        Multiply, ///< Multiplies by the Value()
        Divide ///< Divides by the Value()
    };
    
    /// \brief Binds a reference to the Mutator.
    ///
    /// Once a value is bound, it will be accessed and updated on 
    /// the Step of the playing Mutation with the current Value()
    /// How the value will be updated depends on the BindFunction set 
    /// The defualt is to "Set" which just replaces the value with 
    /// the mutation value.
    ///\{
    void Bind(float &,    BindFunction f = BindFunction::Set);
    void Bind(int &,      BindFunction f = BindFunction::Set);
    void Bind(uint8_t &,  BindFunction f = BindFunction::Set);
    void Bind(uint32_t &, BindFunction f = BindFunction::Set);
    void Bind(uint64_t &, BindFunction f = BindFunction::Set);
    
    void Unbind(float &);
    void Unbind(int &);
    void Unbind(uint8_t &);
    void Unbind(uint32_t &);
    void Unbind(uint64_t &);
    ///\}
    
    
    
    

    /// \brief Iteratively eases a value to another and returns the stepped value.
    /// This is known in other toolkits as the 'lerp' function.
    ///
    static float StepTowards(float current,
                             float destination,
                             float step = .5f,
                             Function f = Function::Linear);
    static Vector StepTowards(Vector current,
                             Vector destination,
                             float step = .5f,
                             Function f = Function::Linear);



    void OnStep();




    std::string GetInfo();








  private:
    struct MutationState {
        MutationState(float t, Mutator::Function f, float val, float min, float max, bool ran) :
            time(t),
            func(f),
            value(val),
            minValue(min),
            maxValue(max),
            isRandom(ran) {}
        float time;
        Mutator::Function func;
        float value;
        float minValue;
        float maxValue;
        bool isRandom;

        bool operator<(const MutationState & m) const {
            return time < m.time;
        }
    };
    class CompareMutationState {
      public:
        bool operator()(const MutationState * a, const MutationState *b) const {
            return (*a < *b);
        }
    };

    std::set<MutationState*, CompareMutationState> states;
    float begin;
    Dynacoe::Clock timer;
    bool isLooping;
    bool hasBinds;
    bool accountForLastStep;
    MutationState * registr;
    
    typedef void (*BindFunctionBase)(void *, int type, float val);
    
    std::vector<std::pair<void*, BindFunctionBase>> * bindVectors;
    
    void Bind(void *, int, BindFunction);
    void Unbind(void*, int);
    static float compute(float first, float end, Function f, float ratio);

};

///\}




}


#endif
