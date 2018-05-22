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


#ifndef DWORLD_H_INCLUDED
#define DWORLD_H_INCLUDED

#include <Dynacoe/AssetID.h>
#include <Dynacoe/Variable.h>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>

namespace Dynacoe {
class Component;
class UintToID;
class Node;
/// \brief Basic interactive object.
///
/// Entity s are the main classes that are extended to meet abstractions for your 
/// multimedia project. In conjunction with functionality you write in the 
/// OnStep(), OnDraw(), etc. virtual functions, you can dynamically extend 
/// Entity s using Component s and attaching them as you see fit.
///
/// Entity::ID s may also be used. Entity::ID s allow you to safely work 
/// with references of Entities. Using Entity::ID::Valid() calls can also 
/// safely and reliable tell you whether an Entity is still accessible.
///
/// Entity s are also inherently hierarchical. By Attach()ing other Entity s,
/// you can create trees of self-managed Entities.
class Entity {

  public:
      /// \brief Uniquely identifies an Entity 
      ///
      class ID {
        public:
          ~ID();
          ID() : id(nullptr){};
          ID(const std::string & str);
          ID(uint64_t value);
          ID(const ID &);
          ID & operator=(const ID & other);

          bool operator==(const ID & other) const {
              return other.id == id;
          }
          bool operator!=(const ID & other) const {
              return other.id != id;
          }
          /// \brief Returns a value that unique ientifies the Entity.
          ///
          uint64_t Value() const { return (uint64_t)id; }

          /// \brief Returns the entity referred to by this ID
          ///
          Entity * Identify() const;
          
          /// \brief Returns the ID in string form 
          ///
          const std::string & String() const;

          /// \brief Convenience function for .Identify()->QueryComponent<T>()
          ///
          /// Returns nullptr if there is no component or if this ID does not point to a valid
          template<typename T>
          T * Query() {
              if (!Valid()) return nullptr;
              return Identify()->QueryComponent<T>();
          }


          friend bool operator<(const Entity::ID & l, const Entity::ID & r) {
              return l.id < r.id;
          }
          /// \brief Same as Find, but casts it to the type you care about.
          ///
          /// If the given type doesnt actually inherit from Entity, nullptr is returned.
          template<typename T>
          T * IdentifyAs() const { return dynamic_cast<T*>(Identify()); }

          bool Valid() const;

        private:
          void * id;

          friend class Entity;

      };


    using   Priority    = int64_t;


    /// \brief The Node of the Entity, holding info such as the position and rotation.
    ///
    /// By default, every Entity has a Node Component attached to it.
    /// If your Entity does not require a Node, you can simply call
    /// Detach(&node) to not update any node logic so save CPU cycles.
    Node & node;


    /// \brief Returns the i'th Entity starting at 0.
    ///
    /// Entities are ordered by priority, where the first entity
    /// is guaranteed to be the one with the lowest Priority value.
    /// If unsuccessful, the EntityID will be invalid.
    const std::vector<Entity::ID> & GetChildren() const;


    /// \brief Returns all Entities that are within this Entity's hierarchy.
    std::vector<Entity::ID> GetAllSubEntities() const;

    /// \brief Returns the Entity with the given ID.
    ///
    /// If unsuccessful returns nullptr.
    bool Contains(Entity::ID);


    /// \brief Returns all bound Entities with the name equivalent to the one given within 
    /// this Entity's hierarchy.
    /// The Entity's IDs are returned.
    std::vector<Entity::ID> FindChildByName(const std::string &);

    /// \brief Updates all attached Entities.
    ///
    void Step();

    /// \brief Draws all attached Entities.
    ///
    void Draw();

    /// \brief Binds an entity to the World. If bound, an Entity will be
    /// updated when the World is updated.
    ///
    void Attach(Entity::ID);
    void Attach(Entity &);

    /// \brief Unbinds an Entity.
    ///
    void Detach(Entity::ID);






    /// \brief Allocates an Entity and Attaches it for you.
    ///
    template<typename T>
    T * CreateChild() {
        T * out = new T();
        Attach(out->GetID());
        return out;
    }

    /// \brief Creates a new Entity and returns its ID.
    ///
    template<typename T>
    static Entity::ID Create() {
        T * out = new T();
        return out->GetID();
    }
    
    /// \brief Creates a new Entity of no type and returns its ID.
    ///
    static Entity::ID Create() {
        return (new Entity)->GetID();
    }
    template<typename T>
    
    /// \brief Creates a new Entity of the given type.
    ///
    static T * CreateReference() {
        return new T;
    }


    /// \brief Returns number of active Entities.
    ///
    int GetNumChildren();






    /// \brief Returns the last recorded amount of milliseconds it took the
    /// Entity, iedentified by id, to complete its run cycle. A run cycle consists of the Entities
    /// Run function and any attached components' run.
    ///
    double StepDuration();

    /// \brief Returns the last recorded amount of milliseconds it took the
    /// Entity, iedentified by id, to complete its draw cycle. A draw cycle consists of the Entities
    /// Draw function and any attached components' draw.
    ///
    double DrawDuration();













    /// \name Variable control
    ///
    /// When debugging, it is often convenient to view the value of something
    /// while the program runs normally. This interface allows you to connect specific values
    /// to be viewed in the Debugger.
    ///
    ///\{


    /// \brief Allows you to monitor a Variable
    /// through the debugger.
    ///
    /// @param v The variable to watch.
    void Watch(Variable v);

    /// \brief Returns a watched Variable of the given name.
    ///
    /// @param name The name of the Variable to ask for.
    Variable GetWatched(const std::string & name);

    /// \brief Returns all watched variables associated with this Entity.
    ///
    std::vector<Variable> GetWatchedVars();

    /// \brief Stops watching a variable of the given name.
    ///
    /// @param varName The variable to stop watching.
    void Unwatch(const std::string & varName);
    ///\}




    /// \brief Alters the priority of this entity.
    ///
    /// Priorty determines the order in which this
    /// entity is updated. A lower priority means it will be drawn and updated earlier.
    /// It is undefined which entity is updated first if both have the same priority.
    /// @param p The new priority.
    void SetPriority(Priority p);

    /// \brief Sends the entity of the ID to
    /// be the last of in line for drawing and updates.
    ///
    void SetPriorityLast();

    /// \brief Sends the entity to be drawn and updated as the last in the queue
    /// causing it to be on top when drawing.
    ///
    void SetPriorityFirst();

    /// \brief Returns the priority of the this entity.
    ///
    Priority GetPriority();





    /// \name Update Handling
    ///
    /// When using the Engine, Entities are utomatically updated
    /// based on your specified framerate. However, it is
    /// useful at times to control when you update the Entity manually.
    ///\{


    /// \brief Whether the engine should call Step() automatically for this entity.
    /// Note that Step() calls also manage components and child entities.
    /// The default is true.
    ///
    bool step;

    /// \brief Whether the engine should call Draw() automatically for this entity.
    /// Note that Draw() calls also manage components and child entities.
    /// The default is true.
    ///
    bool draw;


    /// \brief Returns wether or not the Engine is handling calling Step() automatically,
    /// taking into account the Entity's hierarchy.
    ///
    bool IsStepping();

    /// \brief Returns wether or not the Engine is handling calling Draw() automatically,
    /// taking into account the Entity's hierarchy.
    ///
    bool IsDrawing();
    ///\}



    /// \brief  Returns the World that the Entity belongs to.
    ///
    /// *Every Entity is guaranteed to return a valid reference*. In the case that
    /// no actual world owns the Entity,
    Entity & GetParent();

    /// \brief Returns whether or not the Entity belongs to a world.
    ///
    bool HasParent();



    /// \brief When Components should be updated.
    ///
    enum class UpdateClass {
        Before, ///< Before Entity base logic is run
        After   ///< After Entity base logic is run
    };

    /// \name Component Control
    ///
    /// Components offer additional standard sets of functionality
    /// that extend the capabilities of the Entity it is attached to.
    ///
    ///\{


    /// \brief Attaches a component to this entity.
    ///
    /// Once attached, the component's
    /// Run and Draw functions will be called before this entity's Run and Draw.
    /// @param c The new compontent to add.
    Component * AddComponent(Component * c, UpdateClass when = UpdateClass::Before);

    /// \brief Same as AddComponent, but the component is destroyed with the Entity.
    ///
    template<typename T>
    T * BindComponent(UpdateClass when = UpdateClass::Before) {
        T * component = new T;
        if (!dynamic_cast<Component *>(component)) {
            // not a component...
            delete component;
            return nullptr;
        }
        AddComponent(component, when);
        componentsBound.push_back(component);
        return component;
    }

    /// \brief Returns whether or not there is currently an attached component of the
    /// type given.
    ///
    /// If there is, a reference to the first one is returned.
    /// If not, nullptr is returned.
    template<typename T>
    T * QueryComponent();


    /// \brief Returns all components that belong to the Entity.
    ///
    const std::vector<Component *> & GetComponents() const;

    /// \brief Removes the first occurrance of a component
    /// with the tag given.
    ///
    /// No action is taken on the data
    /// associated with the component.
    /// @param tag The tag that identifies the component to remove.
    void RemoveComponent(const std::string & tag);

    /// \brief Removes the attached component with the given pointer
    ///
    void RemoveComponent(const Component *);
    ///\}




    /// \brief Detaches and marks this entity for deletion. After this is called, all references to
    ///
    void Remove();





    /// \brief Sets an optional name to further identify the entity. Tools such as the debugger
    ///
    /// use this to further identify entities.
    /// @param name The new name of the Entity. This name will also be displayed in the Debugger.
    void SetName(const std::string & name);

    /// \brief Returns the Entity's unique ID.
    ///
    /// The ID uniquely identifies an Entity. If the source Entity is removed or freed, the
    /// ID will return false for a Valid call and Entity() will return NULL.
    ID GetID();

    /// \brief Returns the name identifier of the Entity.
    ///
    std::string GetName();



    /// \brief Returns a list of all Entitys. Note that this list is
    /// generated every time
    static std::vector<Entity::ID> GetAll();



    virtual ~Entity();













  protected:

    Entity();
    Entity(const std::string &);



    /// \name Base Functionality
    ///
    /// When defining your Entity, these are the basic functions
    /// that allow the Entity to express logic.
    /// \{


    /// \brief The Entry function is called upon each attachment to a world.
    ///
    /// On the first attachment, Init is called before Enter.
    virtual void OnEnter(){}

    /// \brief The Depart function is called upon each detachment from a world.
    ///
    virtual void OnDepart(){}


    /// \brief The Destruct function is called just before removal of the object.
    ///
    /// It should be used like a destructor to free and resources associated with the
    /// entity.
    virtual void OnRemove(){}


    /// \brief Called when Step() is called.
    ///
    virtual void OnPreStep(){}

    /// \brief Called when Step() is called.
    ///
    virtual void OnStep(){}


    /// \brief Called when Draw() is called.
    ///
    virtual void OnPreDraw(){}

    /// \brief Called when Draw() is called.
    ///
    virtual void OnDraw(){}

    ///\}

    void * operator new(std::size_t);
    void * operator new[](std::size_t);
    void operator delete(void * ptr);
    void operator delete[](void * ptr);


  private:





    friend class Engine;

     bool WasDetachedMidExecution(Entity::ID id);
     void priorityListQueueAdd(Entity::ID);
     void priorityListQueueRemove(Entity::ID);
     void priorityListAdd(Entity::ID);
     void priorityListRemove(int);
     void initBase();
     int getNewEntID();



     std::vector<Entity::ID> PriorityList;     // normal entity list




     std::set<Variable> watchList;

     Entity::ID id;
     std::string name;
     Entity * world;




     int64_t priority;



     void * idTable;
     bool protectd;
     bool removed;

     double stepTime;
     double drawTime;
     static uint64_t idPool;


     std::vector<Component *> componentsBefore;
     std::vector<Component *> componentsAfter;
     std::vector<Component *> components;
     std::vector<Component *> componentsBound;
     friend void EntityErase(Entity * e);
};




template<typename T>
T * Entity::QueryComponent() {
    for(int i = 0; i < componentsBefore.size(); ++i) {
        if (dynamic_cast<T *>(componentsBefore[i])) {
            return static_cast<T *>(componentsBefore[i]);
        }
    }

    for(int i = 0; i < componentsAfter.size(); ++i) {
        if (dynamic_cast<T *>(componentsAfter[i])) {
            return static_cast<T *>(componentsAfter[i]);
        }
    }

    return nullptr;
}
void EntityErase(Entity * e);

};


#endif // DSPAWNED_H_INCLUDED
