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


#include <Dynacoe/Components/Scheduler.h>
#include <Dynacoe/Components/Clock.h>
using namespace Dynacoe;



// A full task
struct Dynacoe::TaskM {
    Component::EventHandler task;
    std::string name;
    uint32_t interval;
    Clock timer;
    void * data;
};


bool TaskMCompare(const TaskM* a, const TaskM* b) {
    return a->name < b->name;
}

Scheduler::Scheduler() : Component("Scheduler") {
    tasks = new std::set<TaskM*, bool(*)(const TaskM *, const TaskM *)>(TaskMCompare);
}
Scheduler::~Scheduler() {
    auto iter = tasks->begin();
    while(iter != tasks->end()) {
        delete (*iter++);
    }

    delete tasks;
}




void Scheduler::StartTask(const std::string & name, uint32_t intervalMS, Component::EventHandler task, uint32_t initialDelay, void * data) {
    TaskM * t = new TaskM();
    t->name = name;
    t->timer.Set(intervalMS+initialDelay);
    t->task = task;
    t->interval = intervalMS;
    t->data = data;
    tasks->insert(t);
}


void Scheduler::EndTask(const std::string & name) {
    TaskM task;
    task.name = name;
    auto iter = tasks->find(&task);
    if (iter == tasks->end()) return;

    delete (*iter);
    tasks->erase(iter);
}


uint32_t Scheduler::GetTaskInterval(const std::string & name) {
    TaskM task;
    task.name = name;
    auto iter = tasks->find(&task);
    if (iter == tasks->end()) return 0;

    return (*iter)->interval;
}

std::vector<std::string> Scheduler::GetTasks() {
    std::vector<std::string> out;
    auto iter = tasks->begin();
    while(iter != tasks->end()) {
        out.push_back((*iter++)->name);
    }
    return out;
}


void Scheduler::Pause() {
    auto iter = tasks->begin();
    while(iter != tasks->end()) {
        (*iter++)->timer.Pause();
    }
}


void Scheduler::Resume() {
    auto iter = tasks->begin();
    while(iter != tasks->end()) {
        (*iter++)->timer.Resume();
    }
}





void Scheduler::OnStep() {
    Entity * host;
    if (!(host = GetHost())) return;

    TaskM * task;
    bool repost;
    auto iter = tasks->begin();
    std::vector<std::string> toBeRemoved;

    while(iter != tasks->end()) {
        task = *iter++;

        if (task->timer.IsExpired()) {
            if(task->task(task->data, this, host->GetID(), Entity::ID(), {})) {
                task->timer.Set(task->interval);
            } else {
                toBeRemoved.push_back(task->name);
            }
        }
    }

    for(uint32_t i = 0; i < toBeRemoved.size(); ++i) {
        EndTask(toBeRemoved[i]);
    }

}

std::string Scheduler::GetInfo() {
    return (Chain() << "Active task count: " << (int) tasks->size());
}
