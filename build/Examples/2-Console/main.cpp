
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


#include <Dynacoe/Library.h>



/*  An example showing how to
    interact with the console

    2014, Johnathan Corkery
*/

using namespace Dynacoe;


// This entity simply posts messages to the console
class ConsoleExample : public Entity {
  public:
    ConsoleExample() {
        SetName("ConsoleEx");

        // Since we are only going to be working in the console,
        // lets have the Entity switch to the console right away for us.
        // This can be done manually by pressing Shift + TAB at any time.
        // While on the console, Shift+TAB will bring you back to the scene.
        // Note that brining up the console does not stop or pause the scene.
        Console::Show(true);

        // Lets output a simple message: "Greetings"
        Console::Info() << "Greetings";


        // The Info() function informs Dynacoe to print output to the console. 
        // To mark the end of the line, the message must end in a Console::End().
        Console::Info() << " and Salutations!" << Console::End(); // now the line reads: Greetings and Salutations!



        // we can also chain pipe commands indefinitely:
        Console::Info() << "How are you?" << Console::End()
                        << "What is new?" << Console::End();
        // For readability, usually lines of output are given their own line
        // when piping multiple lines under one call.


        // there are other line types to try:
        Console::Info()    << "This is system info." << Console::End(); 
        Console::Warning() << "This is a warning. Usually for things "
                           << "that are non-fatal." << Console::End();
        Console::Error()   << "This is an error. Usually for things "
                           << "that aren't recoverable." << Console::End();


        // The console piping command also accepts numbers:
        Console::Info() << "My favorite flavor of pi is " << 3.14 << Console::End();

    }


};




// Just as in 1-Rectagles example, this section contains just the driving code
// just contains driving code. 

int main() {
    Engine::Startup();    

    ViewManager::NewMain("Console Example");
    
    
    Engine::Root() = Entity::Create<ConsoleExample>(); 
    Engine::Run();
    
    
    return 0;
}

