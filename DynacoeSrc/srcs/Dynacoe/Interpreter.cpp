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


#include <Dynacoe/Interpreter.h>
#include <Dynacoe/Util/Chain.h>

using namespace Dynacoe;



void Interpreter::AddCommand(const std::string & cmd, Command * command) {
    commands[cmd] = command;
}


std::string Interpreter::RunCommand(const std::string & in_src) {
    std::string in = in_src;
    std::vector<std::string> args;

    // compute any evaluator blocks in full
    in = ReduceEvaluators(in_src);


    Chain str = in;
    std::string command = str.GetLink();




    // the help keyword is special.
    // if it is the first token in the command 
    // after evaluation, it will treat the following 
    // argument as the command to request help for
    if (command == "help") {
        str++;
        std::string helpPage = str.GetLink();
        if (helpPage == "") {
            return "Syntax: help [cmd]\n";
        }

        auto helpCmd = commands.find(helpPage.c_str());
        if (helpCmd == commands.end()) {
            Chain c = "";            
            c << "\"" << helpPage.c_str() << "\" does not exist as a command.\n";
            c << "Here are the list of available commands:\n";
            auto listIter = commands.begin();
            while(listIter != commands.end()) {
                c << "    " << (listIter++)->first.c_str() << "\n";
            }
            return c;
        }
        return (Chain() << helpPage << ": " << helpCmd->second->Help() << "\n");
    }






    auto iter = commands.find(command.c_str());

    if (iter == commands.end()) {
        return (Chain() << "Unknown command \"" << in << "\"");
    }

    // parse arguments
    while(str.LinksLeft()) {

        if (str.GetLink()[0] != '"') {
            // normal arg
            args.push_back(str.GetLink().c_str()); str++;
        } else {
            // double quote arg 
            Chain compoundArg;
            int iter = str.GetLinkPos();
            iter++;
            while(str.ToString()[iter] != '"' && iter < str.ToString().size()) {
                compoundArg<<str.ToString()[iter];
                iter++;
            }

            if (!(iter < str.ToString().size() && 
                  str.ToString()[iter] == '"'    && 
                  iter != str.GetLinkPos())) {
                return "Missing compound argument terminator \"...";
            }

            // push in arg minus quotes
            args.push_back(compoundArg);

            str = str.ToString().substr(iter+1, str.ToString().size()-1);
        }
    }

    return (*(iter->second))(args);
}

std::string Interpreter::ReduceEvaluators(const std::string & src) {
    std::string in = src;
    // first, scan for evaluation substrings
    // evalutaion commands cannot be embedded within the same calling
    // scope. However, additional produced evaluators after evaluation 
    // are parsed in their entirety (though, i cannot think of a
    // situation where that can actually occur outside of loading 
    // a script...)
    uint32_t firstIndex;
    uint32_t endIndex;
    std::string subCmd;
    std::string part1, part2;

    for(int i = 0; i < in.size(); ++i) {
        if (in[i] == '|') {
            subCmd = "";
            firstIndex = i;
            int n;
            for(n = i+1; in[n] != '|' && n < in.size(); ++n) {
                subCmd += in[n];
            }
            endIndex = n+1 ;
            if (in[n] != '|') {
                return in; // ignore if incomplete  
            }
            
            //TODO: handle bad subcall           

            // Now we can evaluate and replace the part edited with this.
            subCmd = RunCommand(subCmd);
            

            part1 = in.substr(0, firstIndex);
            part2 = in.substr(endIndex, in.size() - endIndex);


            // We also want to reset the evaulator in case any new pieces came in
            in = std::string(part1.c_str()) + subCmd + part2.c_str();
            i = 0;
        }
    }
    return in;
}

// Assigns subcommands. THis behavior is equivalent 
// to adding additional functions / scripts
class Command_Alias: public Interpreter::Command {
  public:
    Command_Alias(Interpreter * in) {
        interp = in;
    }

    std::string operator()(const std::vector<std::string> & argvec) {
        if (argvec.size() != 4) {
            return interp->RunCommand("help alias") + "\n(Please refer to the help documentation for proper invocation)";
        }


        // First get the alias name
        std::string cmd = argvec[1];
        std::vector<std::string> cmds;


        // Next get the arguments.
        Chain argSrc = argvec[2].c_str();
        std::vector<std::string> args;
        argSrc.SetDelimiters(",");
        while(argSrc.LinksLeft()) {
            args.push_back(argSrc.GetLink().c_str());
            argSrc++;
        } 
        
        

        Chain src = argvec[3].c_str();
        src.SetDelimiters(",");

        while(src.LinksLeft()) {
            cmds.push_back(src.GetLink().c_str());
            src++;
        }    
        interp->AddCommand(cmd, new SubCommand(cmds, args, interp));

        return std::string("Set new command: ") + cmd;
        
    }

    class SubCommand : public Interpreter::Command {
      public:
        SubCommand(const std::vector<std::string> & argvec, 
                   const std::vector<std::string> & argReplace,
                   Interpreter * in) {

            interp = in;
            for(auto iter = argvec.begin(); iter != argvec.end(); ++iter) {
                subs.push_back(*iter);
            }

            for(auto iter = argReplace.begin(); iter != argReplace.end(); ++iter) {
                args.push_back(*iter);
            } 
        }

        std::string operator()(const std::vector<std::string> & argvec) {
            if (argvec.size() != args.size()+1) {
                return (Chain("Error: alias \"") 
                    << argvec[0] << "\" expects " 
                    << (int)args.size() << " arguments, only " 
                    << (int)argvec.size()-1);
            }
            std::string out;

            //TODO: this is pretty terrible, make it faster please
            for(std::string cmd : subs) {
                size_t pos = 0;
                for(int i = 0; i < args.size(); ++i) {
                    std::string arg = args[i];

                    // for each occurrence of the argument to the alias 
                    // within the command, replace with the invoked argument.
                    while((pos = cmd.find(arg, pos)) != std::string::npos) {
                        cmd.replace(pos, arg.size(), argvec[i+1]);
                        pos+=argvec[i+1].size();
                    } 
                }
                out = interp->RunCommand(cmd);
            }

            return out;
        }

        std::string Help()const {
            std::string out;
            out = "Alias command. Runs the following:\n";
            for(int i = 0; i < subs.size(); ++i) {
                out += "    ";
                out += subs[i];
                out += "\n";
            }
            return out;
        }

      private:

        std::vector<std::string> subs;
        std::vector<std::string> args;
        Interpreter * interp;
    };

    std::string Help()const {
        return 
            "Registers a new command that runs the actions contained in\n"
            "either an external file or command list\n\n"
            "usage:\n"
            "    alias aliasName \"aliasArg1, aliasArg2, ...\" \"cmd1 arg arg, cmd2, ...\"\n"
            "\n\n"
            "Any commands or arguments in the command list\n"
            "that match aliasArgs will be replaced with that\n"
            "alias arg. The result of the last command is returned.\n";
            

    }
  private:
    Interpreter * interp;
};

class Command_Print: public Interpreter::Command {
  public:

    Command_Print(Interpreter * in) { interp = in; }

    std::string operator()(const std::vector<std::string> & argvec) {
        auto iter = argvec.begin();
        Chain out;
    
        for(iter++; iter != argvec.end(); ++iter) {
            out << iter->c_str() << " ";
        }
        return out;    
    }


    std::string Help()const {
        return 
            "Returns the arguments given. \nusage: echo \"stuff\"";

    }
  private:
    Interpreter * interp;
};


Interpreter::Interpreter() {
    AddCommand("echo", new Command_Print(this));
    AddCommand("alias", new Command_Alias(this));
}




