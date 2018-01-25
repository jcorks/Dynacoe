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

#ifndef H_DCOMPONENT_INCLUDED
#define H_DCOMPONENT_INCLUDED

#include <Dynacoe/Entity.h>


namespace Dynacoe {
class Entity;

/**
 * \brief Class that extends the functionality of an Entity, but as a removable
 * and addable object.
 *
 * To work, all Component s have hosts; when a host updates, all components do as well.
 * Like Entities, Components cna have a Draw and a Step function with an overloadable
 * OnStep / OnDraw. In addition, all Component's also have event handling capabilities.
 * Setable with string, it is possible to maintain and assign functions to run on 
 * certain events and even define your own events for custom Components.
 * 
 */
class Component {
  public:

    void Step();
    void Draw();

    bool draw;
    bool step;

    //virtual Component * Clone() {return nullptr;}

    /// \brief Returns a string identifier belonging to the component.
    /// Each component implementation should have a unique name.
    ///
    std::string GetTag() { return tag; };

    /// \brief Sets the current host of the component.
    ///
    void SetHost(Entity *);

    /// \brief Returns the set host of the component. If no host is set,
    /// nullptr is returned.
    ///
    Entity * GetHost() const {return host; };

    /// \brief Convenience function. Equivalent to dynamic_cast<T*>(GetHost())
    ///
    template<typename T>
    T * GetHostAs() const {return dynamic_cast<T*>(GetHost());}

    /// \brief Returns a ID of the host. If there is non, an empty ID is returned.
    ///
    Entity::ID GetHostID() const {return host ? host->GetID() : Entity::ID();}

    /// \brief Returns a string containing information on the state of the
    /// component.
    ///
    virtual std::string GetInfo(){return "";}




    /// \name Event control
    ///
    /// Events allow for an efficient option to have logic run in reaction 
    /// to a trigger. When construction your own Component, you can install 
    /// any number of Events and call EmitEvent() when you'd like the user's
    /// logic to run.
    ///
    ///\{

    /// \brief A handler for an event.
    ///
    /// The return value tells the event system to propogate the event
    /// thats currently being processed.
    using EventHandler = bool (*)(Component * component, Entity::ID self, Entity::ID source, const std::vector<std::string> & args);



    /// \brief Triggers the specified event for this EventSystem
    ///
    /// Returns whether the event was allowed to propogate all the
    /// way to the main handler (in other words, lets you know whether
    /// all the handlers allowed the event to happen)
    /// Source is passed to the handler and is usually the source of the event (but does not have to be)
    bool EmitEvent(const std::string &, Entity::ID source = Entity::ID(), const std::vector<std::string> & args = {});

    /// \brief Returns whether there exists at least one handler for the given event
    ///
    bool CanHandleEvent(const std::string &);


    /// \brief Adds a hook to the event. 
    ///
    /// A hook happens at the end of a given event after all the
    /// handlers have been run. Hooks occur regardless of event handler propogation.
    /// (the return value is ignored for all hooks)
    void InstallHook(const std::string &, EventHandler);
    
    /// \brief Removes a hook added with InstallHook()
    ///
    void UninstallHook(const std::string &, EventHandler);

    /// Adds a handler to an event. 
    ///
    /// Handlers that are added are run in LIFO order
    /// and their return values dictate whether the event should propogate.
    /// the last handler run for an event is always the main handler of the event.
    void InstallHandler(const std::string &, EventHandler);
    
    /// \brief Removes a handler added with InstallHandler()
    ///
    void UninstallHandler(const std::string &, EventHandler);


    /// \breif Returns a list of events that this eventsystem is able to process
    ///
    std::vector<std::string> GetKnownEvents() const;
    ///\}

  protected:
    Component(const std::string &);


    /// \brief Function that is called upon each Run iteration.
    /// Component Run()s are run before the entity's run function.
    ///
    virtual void OnStep(){};


    /// \brief Function that is called upon each Draw iteration.
    /// Component Draw()s are run before the entity's draw function.
    ///
    virtual void OnDraw(){};

    /// \brief Function that is called upon a host entity's Attach() function
    /// calling. It is run right after the host is set.
    ///
    virtual void OnAttach(){};


    /// \breif Adds a new event to be recognized by the EventSystem.
    ///
    /// if mainHandler is nullptr, the event is still added, but has no default
    /// handler
    void InstallEvent(const std::string &, EventHandler mainHandler = nullptr);

    /// \brief removes a handler of an event
    ///
    void UninstallEvent(const std::string &);



  private:
    friend class Entity;
    std::string tag;
    Entity * host;

    struct EventSet {
        std::vector<EventHandler> hooks;
        std::vector<EventHandler> handlers;
    };
    std::unordered_map<std::string, EventSet> handlers;
};
}

/// \brief Convenience wrapper for defining event functions
///
/// A VERY simple macro that avoids the need to have to write
/// the whole function header for a new event handler. You simply pass
/// in the name of your handler, and a function declaration will be
/// put in its place. The arguments are named as follows:
///   argument 1 (Component *) -> "component"
///   argument 2 (Entity::ID)  -> "self"
///   argument 3 (Entity::ID)  -> "source"
///
#define DynacoeEvent(_Name_) bool _Name_(Dynacoe::Component * component, Dynacoe::Entity::ID self, Dynacoe::Entity::ID source, const std::vector<std::string> & args)



#endif
