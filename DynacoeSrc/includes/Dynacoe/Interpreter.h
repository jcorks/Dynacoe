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

#ifndef H_DC_INTERPRETER_INCLUDED
#define H_DC_INTERPRETER_INCLUDED

#include <string>
#include <vector>
#include <map>

namespace Dynacoe {


/// \brief The interpreter provides a means to 
/// execute dynamic runtime behavior through
/// text. This is most conventionally useful via
/// the Console.

class Interpreter {
  public:

    /// \brief Functor that represents a run command.
    ///
    class Command { 
      public:
        /// \brief The callback for the command invocation.
        ///
        /// @param argvec A vector of arguments. The first string is always 
        /// the command name. 
        virtual std::string operator()(const std::vector<std::string> & argvec)= 0;

        /// \brief Function that contains text to be accessed when 
        /// requesting documentation on the command.
        virtual std::string Help() const = 0;
    };




    Interpreter();




    /// \brief Runs a console command.
    ///
    /// The first token of the command is the name of the command.
    /// Any additional tokens are parsed as arguments to the ConsoleCommand.
    /// The result is returned.
    std::string RunCommand(const std::string &);


    /// \brief Sets a new console command.
    ///
    /// @param cmd The string that identifies the command.
    /// @param consoleCommand A pointer to a console command.
    void AddCommand(const std::string & cmd, Command * consoleCommand);
    


  private:

    struct CommandResult {
        Interpreter::Command * f;
        std::vector<std::string> args;
    };

    std::map<std::string, Command*>        commands;
    std::string ReduceEvaluators(const std::string &);
    CommandResult RunCommandString(const std::string & in);

    


};
}


#endif
