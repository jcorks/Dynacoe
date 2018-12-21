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


#include <Dynacoe/Component.h>
#include <Dynacoe/Modules/Console.h>

using namespace Dynacoe;

Component::Component() {
    host = nullptr;
    draw = true;
    step = true;
    
    InstallEvent("on-attach");
    InstallEvent("on-detach");
}

void Component::SetHost(Entity * h) {
    host = h;
}

void Component::Draw() {
    OnDraw();
}

void Component::Step() {
    OnStep();
}







bool Component::EmitEvent(const std::string & ev, Entity::ID source, const std::vector<std::string> & args) {

    auto eventHandlers = handlers.find(ev);
    if (eventHandlers == handlers.end()) {
        Dynacoe::Console::Error() << "Component " << GetTag() << " error: cannot perform unknown signal \"" << ev << "\"\n";
        return false;
    }

    EventSet fns = eventHandlers->second;
    bool retval = true;
    int size = fns.handlers.size();
    if (size) { 
        for(int i = size-1; i >= 0; --i) {
            if (!fns.handlers[i].first(fns.handlers[i].second, this, GetHostID(), source, args)) {
                retval = false;
                break;
            }
        }
    }
    for(size_t i = 0; i < fns.hooks.size(); ++i) {
       fns.hooks[i].first(fns.hooks[i].second, this, GetHostID(), source, args);
    }

    return retval;
}


bool Component::CanHandleEvent(const std::string & s) {
    return handlers.find(s) != handlers.end();
}





void Component::InstallEvent(const std::string & name, EventHandler h, void * data) {
    if (handlers.find(name) != handlers.end()) {
        Dynacoe::Console::Error() << "Component " << GetTag() << " error: cannot install \"" << name << "\", event is already installed.\n";
        return;
    }
    if (h) 
        handlers[name].handlers.push_back({h, data});
    else 
        handlers[name] = EventSet();
}



void Component::UninstallEvent(const std::string & name) {
    handlers.erase(name);
}



std::vector<std::string> Component::GetKnownEvents() const {
    std::vector<std::string> out;
    for(auto i = handlers.begin(); i != handlers.end(); ++i) {
        out.push_back(i->first);
    }
    return out;
}


void Component::InstallHandler(const std::string & name, EventHandler h, void * data) {
    auto fns = handlers.find(name);
    if (fns == handlers.end()) {
        Dynacoe::Console::Error() << "Component " << GetTag() << " error: cannot add handler for \"" << name << "\": event is not installed.\n";
        return;
    }
    fns->second.handlers.push_back({h, data});
}

void Component::UninstallHandler(const std::string & name, EventHandler h) {
    auto localHandlers = handlers.find(name);
    if (localHandlers == handlers.end()) return;

    for(size_t i = 0; i < localHandlers->second.handlers.size(); ++i) {
        if (localHandlers->second.handlers[i].first == h) {
            localHandlers->second.handlers.erase(localHandlers->second.handlers.begin()+i);
        }
    }
}





void Component::InstallHook(const std::string & name, EventHandler h, void * data) {
    auto fns = handlers.find(name);
    if (fns == handlers.end()) {
        Dynacoe::Console::Error() << "Component: cannot add hook for \"" << name << "\": event is not installed.\n";
        return;
    }
    fns->second.hooks.push_back({h, data});
}

void Component::UninstallHook(const std::string & name, EventHandler h) {
    auto localHandlers = handlers.find(name);
    if (localHandlers == handlers.end()) return;

    for(size_t i = 0; i < localHandlers->second.hooks.size(); ++i) {
        if (localHandlers->second.hooks[i].first == h) {
            localHandlers->second.hooks.erase(localHandlers->second.hooks.begin()+i);
        }
    }
}

void * Component::operator new(std::size_t size) {
    return new char[size];
}



void Component::operator delete(void * ptr)  {
    delete[] (char*) ptr;
}


