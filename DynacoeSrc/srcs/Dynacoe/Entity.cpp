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


#include <Dynacoe/Entity.h>

#include <Dynacoe/Util/Time.h>
#include <Dynacoe/Component.h>
#include <Dynacoe/Modules/Console.h>
#include <Dynacoe/Backends/Backend.h>
#include <Dynacoe/Modules/Graphics.h>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace std;
using namespace Dynacoe;



static std::unordered_map<void *, bool> * entityIDtableRef = nullptr;
class EntityIDTable {
  public:
    EntityIDTable(Entity * e) {
        if (!entityIDtableRef) entityIDtableRef = new std::unordered_map<void *, bool>;
        refCount = 1;
        source = e;
        (*entityIDtableRef)[this] = true;
        char selfstr[32];
        selfstr[0] = 0;
        snprintf(selfstr, 32, "%llx", this);
        idStr = selfstr;
    }
    ~EntityIDTable() {
        if (!entityIDtableRef) return;
        entityIDtableRef->erase(entityIDtableRef->find(this));
        if (entityIDtableRef->empty())
            delete entityIDtableRef;
    }

    void EntityDestroyed() {
        source = nullptr;
        Remove();
    }

    Entity * GetInstance() const {
        return source;
    }

    void Add() {refCount++;}
    void Remove() {refCount--;
        if (!refCount) {
            delete this;
        }
    }

    const std::string & ToString() {
        return idStr;
    }

    static EntityIDTable * FromString(const std::string & str) {
        void * out = nullptr;
        sscanf(str.c_str(), "%llx", &out);
        return FromAddress((uint64_t)out);
    }

    static EntityIDTable * FromAddress(uint64_t id) {
        if (entityIDtableRef->find((void*)id) == entityIDtableRef->end()) {
            return nullptr;
        }
        return (EntityIDTable*)id;
    }



  private:
    int refCount;
    Entity * source;
    std::string idStr;
};


void Dynacoe::EntityErase(Entity * e) {
    delete e;
}


class EntityLimbo {
  public:
    void SendToOblivion(Entity * e) {
        lostSouls.push(e);
    }

    void OnStep() {
        Entity * soul;

        // pass judgement
        // (protip: everyone gets recycled)
        while(!lostSouls.empty()) {
            soul = lostSouls.top();
            lostSouls.pop();
            EntityErase(soul);
        }


    }

  private:
    std::stack<Entity *> lostSouls;
};

static EntityLimbo * limbo = nullptr;


template<typename T>
class Before {
    public:
        bool operator()(const T a, const T b) const {
            return (a.Identify()->GetPriority() < b.Identify()->GetPriority());
        }

};






void Entity::initBase(){
    static bool spawned = false;

    if (spawned) return;
    spawned = true;


    PriorityList.clear();

}








void Entity::Attach(Entity::ID NewEntID) {
    Entity * NewEnt = NewEntID.Identify();
    if (!NewEnt) return;
    if (NewEnt == this) return;
    if (NewEnt->world == this) return;
    if (NewEnt->world)
        NewEnt->world->Detach(NewEntID);
    //priorityListQueueAdd(NewEnt);
    priorityListAdd(NewEntID);
    NewEnt->world = this;

    NewEnt->OnEnter();
}

void Entity::Attach(Entity & ent) {
    Attach(ent.GetID());
}

void Entity::Detach(Entity::ID entID) {
    Entity * ent = entID.Identify();
    if (!ent) return;
    if (!(ent->GetID()).Valid()) return;
    ent->world = nullptr;

    // Try to find the index of the entity
    auto it = lower_bound(PriorityList.begin(), PriorityList.end(), entID, Before<Entity::ID>{});
    size_t i;
    for(i = distance(PriorityList.begin(), it); i < PriorityList.size(); ++i) {
        if (PriorityList[i] == ent->GetID()) break;
    }
    assert(i != PriorityList.size());
    priorityListRemove(i);
    ent->OnDepart();
}










int Entity::GetNumChildren() {return PriorityList.size();}

const std::vector<Entity::ID> & Entity::GetChildren() const {
    return PriorityList;
}

std::vector<Entity::ID> Entity::GetAllSubEntities() const {
    std::vector<Entity::ID> out;
    for(uint32_t i = 0; i < PriorityList.size(); ++i) {
        out.push_back(PriorityList[i]);
        auto sub = PriorityList[i].Identify()->GetAllSubEntities();
        for(uint32_t n = 0; n < sub.size(); ++n) {
            out.push_back(sub[n]);
        }
    }
    return out;

}

bool Entity::Contains(Entity::ID ID) {
    Entity * other = ID.Identify();
    if (!other) return false;
    if (!other->HasParent()) return false;
    return &other->GetParent() == this;
}



std::vector<Entity::ID> Entity::FindChildByName(const std::string & str) {
    std::vector<Entity::ID> out;
    std::vector<Entity::ID> iter;
    Entity * w;
    Entity::ID id;
    Entity * ent;
    for(int i = 0; i < PriorityList.size(); ++i) {
        id = PriorityList[i];
        if (!id.Valid()) continue;
        ent = id.Identify();

        if(ent->GetName() == str)
            out.push_back(id);

        // recursively look.
        if (w = dynamic_cast<Entity*>(ent)) {
            iter = w->FindChildByName(str);
            for(int n = 0; n < iter.size(); ++n) {
                out.push_back(iter[n]);
            }
        }

    }
    return out;
}


void Entity::SetPriorityLast() {
    if (!HasParent()) return;

    Entity & parent = GetParent();
    if (!parent.PriorityList[parent.PriorityList.size() - 1].Valid()) return;
    double best = parent.PriorityList[parent.PriorityList.size() - 1].Identify()->priority;
    SetPriority(best + 1);

}

void Entity::SetPriorityFirst() {
    if (!HasParent()) return;

    Entity & parent = GetParent();
    if (!parent.PriorityList[0].Valid()) return;
    double best = parent.PriorityList[0].Identify()->priority;
    SetPriority(best - 1);


}


void Entity::Step() {
    if (!step) return;
    Entity::ID idSelf = GetID();

    OnPreStep();
    if (!idSelf.Valid()) return;

    // take out the TRASH
    if (limbo) {
        limbo->OnStep();
    }
    Entity * curEnt;



    vector<Entity::ID> priorityListCopy = PriorityList;
    uint32_t n, numEnts = priorityListCopy.size();
    double recordTime;

    stepTime = 0;
    recordTime = Time::MsSinceStartup();
    for(n = 0; n < componentsBefore.size(); ++n) {
        if (!idSelf.Valid()) return;
        if (componentsBefore[n]->step)
            componentsBefore[n]->Step();

    }


    for(int i = 0; i < numEnts; i++) {
        if (!priorityListCopy[i].Valid()) {
            //TODO: remove dead?
            continue;
        }
        curEnt = priorityListCopy[i].Identify();
        if (!curEnt) continue;
        // if we have destroyed / detached entities native to this Entity
        // within this loop, so we aren't going to run it
        curEnt->Step();


    }
    if (!idSelf.Valid()) return;

    // actual running of self
    OnStep();
    for(n = 0; n < componentsAfter.size(); ++n) {
        if (!idSelf.Valid()) return;
        if (componentsAfter[n]->step)
            componentsAfter[n]->Step();
    }
    if (!idSelf.Valid()) return;
    stepTime = Time::MsSinceStartup()-recordTime;


}



void Entity::Draw() {
    if (!draw) return;
    Entity::ID idSelf = GetID();

    OnPreDraw();
    if (!idSelf.Valid()) return;


    size_t compInd;

    // take out the TRASH
    if (limbo) {
        limbo->OnStep();
    }

    drawTime = 0;
    double recordTime = Time::MsSinceStartup();
    for(compInd = 0; compInd < componentsBefore.size(); ++compInd) {
        if (!idSelf.Valid()) return;
        if (componentsBefore[compInd]->draw)
            componentsBefore[compInd]->Draw();
    }

    // safely use a copy of priorityList

    Entity * e;
    vector<Entity::ID> priorityListCopy = PriorityList;
    bool lost = false;
    for(size_t entIndex = 0; entIndex < priorityListCopy.size(); entIndex++) {
        if (!priorityListCopy[entIndex].Valid()) continue;

        e = priorityListCopy[entIndex].Identify();
        if (!e) continue;
        // if we have destroyed / detached entities native to this Entity
        // within this loop, skip


        e->Draw();
        if (!idSelf.Valid()) return;

    }
    if (!idSelf.Valid()) return;



    OnDraw();
    for(compInd = 0; compInd < componentsAfter.size(); ++compInd) {
        if (!idSelf.Valid()) return;
        if (componentsAfter[compInd]->draw)
            componentsAfter[compInd]->Draw();

    }
    if (!idSelf.Valid()) return;
    drawTime = Time::MsSinceStartup()-recordTime;
}

void Entity::priorityListAdd(Entity::ID curEnt) {
    auto it = lower_bound(PriorityList.begin(), PriorityList.end(), curEnt, Before<Entity::ID>{});
    PriorityList.insert(it, curEnt);
}



void Entity::priorityListRemove(int i) {
    auto it = PriorityList.begin() + i;
    PriorityList.erase(it);
}


double Entity::StepDuration() {
    return stepTime;
}

double Entity::DrawDuration() {
    return drawTime;
}





uint64_t Entity::idPool = 1;
static std::unordered_map<std::string, Entity *> masterEntNameMap;
static std::unordered_map<EntityIDTable*,    Entity *> masterEntIDMap;





const char * unused_name_c = "<No Name>";

Entity * Entity::ID::Identify() const {
    if (!id) return nullptr;
    EntityIDTable * table = (EntityIDTable*)id;
    return table->GetInstance();
}



Entity::ID::ID(const ID & other) {
    id = nullptr;
    *this = other;
}

Entity::ID & Entity::ID::operator=(const Entity::ID & other) {
    if (other.id) {
        EntityIDTable * table = (EntityIDTable*)other.id;
        table->Add();
    }
    if (id) {
        EntityIDTable * table = (EntityIDTable*)id;
        table->Remove();
    }
    id = other.id;

    return *this;
}
Entity::ID::~ID() {
    if (id) {
        EntityIDTable * table = (EntityIDTable*)id;
        table->Remove();
    }
}

Entity::ID::ID(const std::string & str) {
    id = EntityIDTable::FromString(str);
    EntityIDTable * table = (EntityIDTable*)id;
    if (id) table->Add();
}

Entity::ID::ID(uint64_t data) {
    id = EntityIDTable::FromAddress(data);
    EntityIDTable * table = (EntityIDTable*)id;
    if (id) table->Add();
}

const std::string & Entity::ID::String() const {
    EntityIDTable * table = (EntityIDTable*)id;
    return table->ToString();
}




bool Entity::ID::Valid() const{
    return Identify();
}
Entity::Entity(const std::string & str) : Entity(){
    SetName(str);
}
Entity::Entity() : node(*(new Node)) {
    if (!limbo) {
        limbo = new EntityLimbo();
        masterEntIDMap[0] = nullptr;

    }


    name = unused_name_c;
	priority = 0;
    world = nullptr;

	step = true;
	draw = true;
	protectd = false;
    removed = false;

    EntityIDTable * table = new EntityIDTable(this);
    idTable = (void*) table;
    id.id = (void*) table;
    masterEntIDMap[table] = this;


    Watch(Variable("draw", draw));
    Watch(Variable("step", step));
    AddComponent(&node);

}






void Entity::Remove() {
    if (removed) return;
    OnRemove();

    auto children = PriorityList;
    for(uint32_t i = 0; i < children.size(); ++i) {
        if (children[i].Valid())
            children[i].Identify()->Remove();
    }

    if (HasParent()) {
        GetParent().Detach(GetID());
    }
    masterEntIDMap.erase((EntityIDTable*)idTable);
    if (masterEntNameMap.find(name) != masterEntNameMap.end())
        masterEntNameMap.erase(name);

    (limbo)->SendToOblivion(this);
    world = nullptr; // hide limbo's existence from itself? this is getting weird
    removed = true;

    id.id = nullptr;
    EntityIDTable * table = (EntityIDTable*)idTable;
    table->EntityDestroyed();


}

Entity::~Entity() {
    for(uint32_t i = 0; i < componentsBound.size(); ++i) {
        delete (componentsBound[i]);
    }
    delete &node;
}








Entity::ID Entity::GetID() {
    //ID amme(id);
    //assert(amme.Identify() == this);
    return id;
}

string Entity::GetName() {
    return name;
}

void Entity::SetName(const string & str) {
    if (name == unused_name_c) {
        name = str;
        masterEntNameMap[str] = this;

    }
}





void Entity::SetPriority(Priority p) {
    if (world) {
        Entity::ID worldID = world->GetID();
        world->Detach(id);
        priority = p;
        worldID.Identify()->Attach(id);
    } else {
        priority = p;
    }
}

Entity::Priority Entity::GetPriority() {
    return priority;
}


Entity & Entity::GetParent() {
    return *world;
}

bool Entity::HasParent() {
    return world;
}



Component * Entity::AddComponent(Component * c, UpdateClass timeC) {
    if (timeC == UpdateClass::Before)
        componentsBefore.push_back(c);
    else
        componentsAfter.push_back(c);
    components.push_back(c);
    c->SetHost(this);
    c->OnAttach();
    return c;
}


void Entity::RemoveComponent(const string & tag) {
    for(size_t i = 0; i < components.size(); ++i) {
        if (components[i]->GetTag() == tag) {
            components[i]->SetHost(nullptr);
            components.erase(components.begin() + i);
            break;
        }
    }

    for(size_t i = 0; i < componentsBefore.size(); ++i) {
        if (componentsBefore[i]->GetTag() == tag) {
            componentsBefore[i]->SetHost(nullptr);
            componentsBefore.erase(componentsBefore.begin() + i);
            return;
        }
    }


    for(size_t i = 0; i < componentsAfter.size(); ++i) {
        if (componentsAfter[i]->GetTag() == tag) {
            componentsAfter[i]->SetHost(nullptr);
            componentsAfter.erase(componentsAfter.begin() + i);
            return;
        }
    }



}

void Entity::RemoveComponent(const Component * c) {
    for(size_t i = 0; i < components.size(); ++i) {
        if (components[i] == c) {
            components[i]->SetHost(nullptr);
            components.erase(components.begin() + i);
            break;
        }
    }


    for(size_t i = 0; i < componentsBefore.size(); ++i) {
        if (componentsBefore[i] == c) {
            componentsBefore[i]->SetHost(nullptr);
            componentsBefore.erase(componentsBefore.begin() + i);
            break;
        }
    }

    for(size_t i = 0; i < componentsAfter.size(); ++i) {
        if (componentsAfter[i] == c) {
            componentsAfter[i]->SetHost(nullptr);
            componentsAfter.erase(componentsAfter.begin() + i);
            break;
        }
    }



}

const std::vector<Component *> & Entity::GetComponents() const {
    return components;
}






void Entity::Watch(Variable v) {
    watchList.insert(v);
}


Variable Entity::GetWatched(const std::string & name) {
    int i;
    auto iter = watchList.find(Variable(name, i));
    if (iter != watchList.end())
        return *iter;
    return Variable();
}



std::vector<Variable> Entity::GetWatchedVars() {
    std::vector<Variable> out;
    for(auto iter = watchList.begin(); iter != watchList.end(); ++iter) {
        out.push_back(*iter);
    }
    return out;
}


void Entity::Unwatch(const std::string & varName) {
    int i;
    auto iter = watchList.find(Variable(varName, i));
    if (iter != watchList.end())
        watchList.erase(iter);
}


bool Entity::IsStepping() {
    if (!step) return false;
    if (!HasParent()) return true;
    Entity * ent = this;
    do {
        ent = &ent->GetParent();
        if (!ent->step) return false;
    } while(ent->HasParent());
    return true;
}

bool Entity::IsDrawing() {
    if (!draw) return false;
    if (!HasParent()) return true;
    Entity * ent = this;
    do {
        ent = &ent->GetParent();
        if (!ent->draw) return false;
    } while(ent->HasParent());
    return true;
}


std::vector<Entity::ID> Entity::GetAll() {
    if (limbo) {
        limbo->OnStep();
    }
    std::vector<Entity::ID> out;
    Entity::ID id;
    for(auto it = masterEntIDMap.begin(); it != masterEntIDMap.end(); ++it) {
        id = Entity::ID((uint64_t)it->first);
        if (!id.Valid()) continue;
        out.push_back(id);
    }
    return out;
}



void * Entity::operator new(std::size_t size) throw(std::bad_alloc) {
    return new char[size];
}

void * Entity::operator new(std::size_t size, const std::nothrow_t &) throw() {
    return new char[size];
}

void * Entity::operator new(std::size_t size, void * p2) throw() {
    return p2;
}



void Entity::operator delete(void * ptr) throw() {
    delete[] (char*) ptr;
}

void Entity::operator delete(void * ptr, const std::nothrow_t &) throw() {
    delete[] (char*) ptr;
}

void Entity::operator delete(void * ptr, void * ptr2) throw() {
    delete[] (char*) ptr;
}
