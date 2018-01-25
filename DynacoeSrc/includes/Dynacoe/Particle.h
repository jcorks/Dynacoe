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

#ifndef H_DPARTICLE_INCLUDED
#define H_DPARTICLE_INCLUDED

#include <string>
#include <stack>
#include <Dynacoe/Modules/Assets.h>
#include <Dynacoe/Components/Shape2D.h>

class EParticle;

/* Basic structures representing particle data. */
/* Johnathan Corkery, 2014 */
namespace Dynacoe {

/// \name Particles
///
/// Particles are 1 or more images that transform over a duration and eventually
/// destruct. To make a particle, you must load a particle
/// script during runtime either by using the storeParticle() function
/// or the built-in storeParticles() which is called upon initializing the
/// entire application. Once stored, you can then emit the particle effect.
///
///\{



/// \brief A structure that holds the specification of a particle
///
class Particle : public Asset {
  public:

    Particle(const std::string & n);
    std::string Image_name;



    float duration_max;
    int alpha_max;
    float xScale_max;
    float yScale_max;
    float multiScale_max;
    float rotation_max;
    float direction_max;
    float speed_max;

    int red_max;
    int blue_max;
    int green_max;

    float duration_min;
    int alpha_min;
    float xScale_min;
    float yScale_min;
    float multiScale_min;
    float rotation_min;
    float direction_min;
    float speed_min;

    int red_min;
    int blue_min;
    int green_min;

    int alpha_delta_min;
    float xScale_delta_min;
    float yScale_delta_min;
    float multiScale_delta_min;
    float rotation_delta_min;
    float direction_delta_min;
    float speed_delta_min;

    int red_delta_min;
    int blue_delta_min;
    int green_delta_min;

    int alpha_delta_max;
    float xScale_delta_max;
    float yScale_delta_max;
    float multiScale_delta_max;
    float rotation_delta_max;
    float direction_delta_max;
    float speed_delta_max;

    int red_delta_max;
    int blue_delta_max;
    int green_delta_max;
};


/// \brief Emits particles.
///
/// Once a particle is loaded, it can then be drawn from a ParticleEmitter.
/// Particle emitters serve as the context by which emitted particles exist.
/// Thus, the particles emitted are affected by the ParticleEmitter's Position,
/// ordering, updating, etc.
///
class ParticleEmitter2D : public Entity {
  public:
    ParticleEmitter2D();

    /// \brief Enable texture filtering for each particle emitted.
    /// It is enabled by default.
    ///
    void EnableFiltering(bool doIt);

    /// \brief Enable translucency for each each particle drawn.
    /// It is enabled by default.
    ///
    void EnableTranslucency(bool doIt);

    /// \brief Instantiates a particle in 2D space based on a stored design.
    ///
    /// The function returns a reference to the instantated particle.
    /// Changes made to the particle after emission are reflected on the
    /// emitted particle.
    /// \param pos  The point of origin to emit the particle.
    /// \param p    The name of the particle design to emit. If p does
    ///             not refer to a stored particle, then no particle is emitted
    ///             and the reference returned is invalid.
    void EmitParticle(AssetID p, int n = 1);





    void OnDraw();
  private:
    Shape2D shape;
    void drawParticle(EParticle *);
    EParticle * instantiateParticle(const Particle *);


    bool filter;
    bool translucent;

    std::vector<Entity::ID> particleActiveList;

    uint32_t numParticles;
};


///\}


}

#endif
