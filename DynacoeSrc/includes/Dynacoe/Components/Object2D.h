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

#ifndef H_OBJECT2D_INCLUDED
#define H_OBJECT2D_INCLUDED

#include <Dynacoe/Component.h>
#include <Dynacoe/Color.h>
#include <Dynacoe/Util/BoundingBox.h>




namespace Dynacoe {

/// \ingroup Components
/// \{

/// \brief A component that gives movement and collision-detection
/// related attributes to an Entity in 2D space
///
/// Known events:
/// "on-move" - called BEFORE applying the new position of the Entity.
/// "on-collide" - source will always be what caused the collision

class Object2D : public Component {
    public:
        
 
 
        class Collider {
          public:
            class Line {
              public:
                Line(){};
                Line(const Dynacoe::Vector & a, const Dynacoe::Vector & b);
                bool Intersects(const Line & other) const;
                Dynacoe::Vector a;
                Dynacoe::Vector b;
            };
            Collider(){};
            Collider(const std::vector<Dynacoe::Vector> &);
            Collider(float radius, uint32_t numPts=8);
            Collider(const std::vector<Collider> &);
            
            bool WillContainPoint(const Dynacoe::Vector & other) const;
            
            void UpdateTransition(const Dynacoe::Vector & after);
            
            bool CollidesWith(const Collider & otherd) const;



            Entity::ID lastCollided;
            
            const BoundingBox & GetMomentBounds() {return smearBounds;}
          private:
            void SetFromPoints(const std::vector<Dynacoe::Vector> &);
  
            std::vector<Line> boundingBox;
            std::vector<Line> staticPoints;
            std::vector<Line> smear;

            BoundingBox smearBounds;
            Vector oldPosition;
            bool stationary;
            bool isSet;

        };


        Object2D();
        ~Object2D();
    
        void OnAttach();


        /// \brief Compounds a velocity vector with the current velocity
        ///
        /// @param factor The velocity amount to add. Can be negative.
        /// @param direction The direction to apply the velocity in degrees.
        void AddVelocity(float factor, float direction);
        void AddVelocityTowards(float factor, const Dynacoe::Vector &, float directionOffset = 0.f);

        /// \brief Sets the velocity vector of an object.
        /// 
        /// @param factor The velocity to set.
        /// @param direction The direction to set the velocity in degrees.
        void SetVelocity(float factor, float direction);
        void SetVelocityTowards(float factor, const Dynacoe::Vector &, float directionOffset = 0.f);


        /// \brief Sets the amount of resistance to speed measured as a percentile.
        /// 
        /// Every frame, only the amount percent of the host Entity's velocity is retained. For example,
        /// if amt is specified to be .95, every frame 5% of the Entity's speed will be lost.
        /// This is applied in the horizontal direction.
        void SetFrictionX(float amt);

        /// \brief Same as SetFrictionX(), but in the vertical direction.
        ///
        void SetFrictionY(float amt);

        /// \brief Returns the current direction that the entity is traveling in degrees.
        ///
        float GetDirection();

        /// \brief Reduces the speed of the Entity to 0.
        ///
        void Halt();

        /// \brief Returns the horizontal component of the current velocity.
        ///
        float GetVelocityX() const;

        /// \brief Returns the vertical component of the current velocity.
        ///
        float GetVelocityY() const;

        /// \brief Sets the vertical velocity for the component.
        ///
        void SetVelocityX(float);

        /// \brief Sets the vertical velocity for the component.
        ///
        void SetVelocityY(float);



        /// \brief Returns the speed of the object.
        ///
        float GetSpeed();


        /// \brief Sets the speed of the object in the current direction
        ///
        void SetSpeed(float);

        /// \brief Returns the position of the Entity after another frame of movement.
        /// 
        Dynacoe::Vector GetNextPosition();

        /// \brief Returns the previous position of the entity
        const Dynacoe::Vector & GetLastPosition() const;


        Collider collider;


        static void DrawColliders(const Color & c);

        std::string GetInfo();

        void Update();   
    private:
        friend class CollisionManager;
        float speedX;
        float speedY;
        float frictionX;
        float frictionY;
        Dynacoe::Vector last;

};
/// \}

}

#endif
