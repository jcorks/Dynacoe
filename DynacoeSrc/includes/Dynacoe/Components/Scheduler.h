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

#ifndef H_DC_COMPONENT_SCHEDULER_INCLUDED
#define H_DC_COMPONENT_SCHEDULER_INCLUDED

#include <Dynacoe/Component.h>



namespace Dynacoe {
struct TaskM;


/// \ingroup Components
/// \{

/// \brief A Scheduler allows for automatic running 
/// of logic at certain intervals.
class Scheduler : public Dynacoe::Component {
  public:
    Scheduler();
    ~Scheduler();
    void OnStep();



    /// \brief Begins a new task. A task will run 
    /// once per every announced millisecond interval.
    /// The resolution is only as good as the number of times the
    /// host's Run() is called. In an Engine context, this is usually
    /// locked to the Engine's max FPS.
    ///
    /// @param name The name of the task.
    /// @param intervalMS The interval to run the task in milliseconds.
    /// @param task The task to add.
    /// @param initialDelay Specifies an amount of time in milliseconds to wait before starting to manage this task.
    void StartTask(
        const std::string &     name, 
        uint32_t                intervalMS, 
        Component::EventHandler task,
        uint32_t                initialDelay=0,
        void *                  data = nullptr
    );


    /// \brief Halts the task with the given name.
    ///
    /// If there is no task with the given name, no action is taken.
    void EndTask(const std::string & name);

    /// \brief Returns the number of milliseconds that the given 
    /// task is set to.
    uint32_t GetTaskInterval(const std::string & name);

    /// \brief Returns a vector of the names of currently running tasks.
    ///
    std::vector<std::string> GetTasks();

    /// \brief Stops running tasks until Resume() is run.
    ///
    /// All Task's intervals are halted as well
    void Pause();
    
    /// \brief Resumes all tasks.
    ///
    void Resume();


    std::string GetInfo();
  private:

    std::set<TaskM*, bool(*)(const TaskM *, const TaskM *)> * tasks;
};
/// \}

}




#endif
