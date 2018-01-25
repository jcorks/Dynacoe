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


#ifndef CONSOLE_H_INCLUDED
#define CONSOLE_H_INCLUDED

#include <Dynacoe/Library.h>

class ConsoleExample : public Dynacoe::Entity {
  public:
    ConsoleExample() {
        SetName("ConsoleEx");

        // Since we are only going to be working in the console,
        // lets have the Entity switch to the console right away for us.
        // This can be done manually by pressing Shift + TAB at any time.
        // While on the console, Shift+TAB will bring you back to the scene.
        // Note that brining up the console does not stop or pause the scene.
        Dynacoe::Console::Show(true);

        // Lets output a simple message: "Greetings"
        Dynacoe::Console::Info() << "Greetings";


        // The Info() function informs Dynacoe to print output to the console. 
        // To mark the end of the line, the message must end in a Console::End.
        Dynacoe::Console::Info() << " and Salutations!" << Dynacoe::Console::End; // now the line reads: Greetings and Salutations!



        // we can also chain pipe commands indefinitely:
        Dynacoe::Console::Info() << "How are you?" << Dynacoe::Console::End
                                 << "What is new?" << Dynacoe::Console::End;
        // For readability, usually lines of output are given their own line
        // when piping multiple lines under one call.


        // there are other line types to try:
        Dynacoe::Console::Info()    << "This is system info." << Dynacoe::Console::End; 
        Dynacoe::Console::Warning() << "This is a warning. Usually for things "
                                    << "that are non-fatal." << Dynacoe::Console::End;
        Dynacoe::Console::Error()   << "This is an error. Usually for things "
                                    << "that aren't recoverable." << Dynacoe::Console::End;


        // The console piping command also accepts numbers:
        float number = 3.14;

        Dynacoe::Console::Info() << "My favorite flavor of pi is " << number << Dynacoe::Console::End;

    }


};

#endif // CONSOLE_H_INCLUDED
