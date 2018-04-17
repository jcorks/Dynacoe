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





namespace Dynacoe {

class CollisionManager; 
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
        
        /// \brief Defines a collidable region.
        ///
        class ContactPoly {
            public:
                /// \brief Returns the number of points that make up the polygon.
                ///
                int GetNumVectors() const;
        
                /// \brief Returns the i'th point.
                ///
                Dynacoe::Vector GetVector(int i) const;

                /// \brief Updates the value of the i'th point.
                ///
                void SetVector(int, const Dynacoe::Vector &);

                
                ContactPoly(int i) : numPts(i) {pts = new Dynacoe::Vector[i];}
                ~ContactPoly() { delete[] pts;}

            private:
                friend Object2D;
                friend CollisionManager;
                Dynacoe::Vector * pts;
                int numPts;

        };


 



        Object2D();
        ~Object2D();
    
        void OnAttach();

        void OnStep();

        /// \brief Compounds a velocity vector with the current velocity
        ///
        /// @param factor The velocity amount to add. Can be negative.
        /// @param direction The direction to apply the velocity in degrees.
        void AddVelocity(double factor, double direction);
        void AddVelocityTowards(double factor, const Dynacoe::Vector &);


        /// \brief Sets the velocity vector of an object.
        /// 
        /// @param factor The velocity to set.
        /// @param direction The direction to set the velocity in degrees.
        void SetVelocity(double factor, double direction);
        void SetVelocityTowards(double factor, const Dynacoe::Vector &);

        /// \brief Sets the amount of resistance to speed measured as a percentile.
        /// 
        /// Every frame, only the amount percent of the host Entity's velocity is retained. For example,
        /// if amt is specified to be .95, every frame 5% of the Entity's speed will be lost.
        /// This is applied in the horizontal direction.
        void SetFrictionX(double amt);

        /// \brief Same as SetFrictionX(), but in the vertical direction.
        ///
        void SetFrictionY(double amt);

        /// \brief Returns the current direction that the entity is traveling in degrees.
        ///
        double GetDirection();

        /// \brief Reduces the speed of the Entity to 0.
        ///
        void Halt();

        /// \brief Returns the horizontal component of the current velocity.
        ///
        double GetVelocityX();

        /// \brief Returns the vertical component of the current velocity.
        ///
        double GetVelocityY();


        /// \brief Returns the speed of the object.
        ///
        double GetSpeed();


        /// \brief Sets the speed of the object in the current direction
        ///
        void SetSpeed(double);

        /// \brief Returns the position of the Entity after another frame of movement.
        /// 
        Dynacoe::Vector GetNextPosition();



        /// \brief Adds a collision box relative to the origin of the host Entity.
        ///
        ContactPoly & AddContactBox(const Dynacoe::Vector & offset,
                           int w, int h);



        /// \brief Adds a polygon defined by 2D points in space to act as a collision bound.
        ///
        ContactPoly & AddContactPolygon(const std::vector<Dynacoe::Vector> &);



        /// \brief Removes the specified contact polygon
        void ClearAllContacts();

        /// \brief  Sets whether or not this collider should detect collisions.
        ///        
        /// If inactive, other active colliders can still detect this collider.
        /// In general, setting an Entity to be collision-inactve makes sense
        /// for permanently stationary objects.
        void SeekCollisions(bool doIt) { collisionActive = doIt; }



        /// \brief Returns true if the given line segment defined by two points
        /// instersects any point in any contact polygons.
        ///
        bool IsLineCollided(const Dynacoe::Vector &,
                            const Dynacoe::Vector &);

        static void DrawColliders(const Color & c);

        std::string GetInfo();

    private:
        friend class CollisionManager;

        static DynacoeEvent(NullCollision);
        static CollisionManager * collisionManager;
        double speedX;
        double speedY;
        double frictionX;
        double frictionY;
        float greatestColliderSpan;


        void runCollisions();
        void Update();
        



        bool willCollide(Object2D * other);

        bool collisionActive;
        //vector<ContactBox> colliders;
        std::vector<ContactPoly *> colliders;
        static bool isCollided(const ContactPoly * self, const ContactPoly * other,
                        const Dynacoe::Vector & selfOrigin, const Dynacoe::Vector & otherOrigin);
        static bool areIntersected(const Dynacoe::Vector &, const Dynacoe::Vector &,
                            const Dynacoe::Vector &, const Dynacoe::Vector &);
        bool isCollidedState;

};
/// \}

}

#endif
