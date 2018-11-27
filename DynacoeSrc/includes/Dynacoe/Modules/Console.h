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
#ifndef H_DC_CONSOLE_INCLUDED
#define H_DC_CONSOLE_INCLUDED

#include <Dynacoe/Color.h>
#include <Dynacoe/Entity.h>
#include <Dynacoe/Interpreter.h>
#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Component.h>
#include <map>

class ConsoleInputStream;
namespace Dynacoe {

class DebugMessage;
class Shape2D;



/// \brief Streams output to the console. Normally not directly needed.
class ConsoleStream {
  public:
  
    ///\brief Message classifications. For now, just determines the color of the message.
    ///
    enum class MessageType{
        Normal, ///< A normal message
        Warning,///< A warning
        Severe, ///< A severe error
        Fatal,  ///< A fatal warning.
    };

    ConsoleStream & operator=(const ConsoleStream &);

    using FinishedCallback = void (*)(const std::string & text, ConsoleStream::MessageType i); 

    ConsoleStream(const ConsoleStream &);
    ConsoleStream(FinishedCallback);
    ~ConsoleStream(); // calls finished callback with the formatted string
    /// \name Stream Output
    /// 
    /// \{
    ConsoleStream operator<<(const Chain&);
    ConsoleStream operator<<(MessageType);
    ///\}

  private:

    static MessageType type;
    std::string str;
    FinishedCallback finish;
};


/// \brief A debugging utility used to view output from the 
/// Engine and interpret text commands from the user.
///
/// The console a useful utility on the user side when running your program.
/// All Engine messages displayed are recorded in the console. While you can 
/// view output, you can also submit additional commands for the Engine 
/// to run. There exist a number of built-in commands along with the 
/// ability to add your own commands. And, due to the structure of adding 
/// commands, they will all have some sort of help documentation for usage.
namespace Console {



    /// \brief Types of message postings.
    ///
    enum class MessageMode{
        Standard, ///< Every console line added results in a message that briefly appears on screen. This is the default.
        Disabled,     ///< Console output is only added to the console log.
    };

    /// \brief Marks the ending of the line.
    ///
    const char * End();




    /// \name Special streams
    ///\{
    ConsoleStream System();
    ConsoleStream Info  ();
    ConsoleStream Error ();
    ConsoleStream Warning();
    ///\}

    // State

    /// \brief Returns whether or not the console is showing.
    /// 
    bool IsVisible() ;

    /// \brief Activates/deactivates the console.
    ///
    /// @param doIt If true, the console will be shown. If false, the console will be hidden.
    void Show(bool doIt) ;

    /// \brief Returns whether or not console has been locked.
    ///
    /// When the console is locked, the console may not be toggled.
    bool IsLocked() ;

    /// \brief Locks the console.
    ///
    /// @param lock If true, Show() will no longer have an effect.
    /// Calling the function again with false re-enables the toggling.
    void Lock(bool);

    /// \brief Sets whether to pause the engine when showing the console.
    ///
    /// The default is true.
    void PauseOnShow(bool);

    /// \brief Adds an additional command to be 
    /// recognized by the interactive interpreter.
    /// @param name The base name of the command.
    /// @param command The actual command logic.
    void AddCommand(const std::string & name, Interpreter::Command * command);


    /// \brief Returns the number of lines of output
    /// currently held by the console
    ///
    uint32_t GetNumLines();

    /// \brief Returns the i'th most recent message, where 0 is the
    /// earliest line and GetNumLines()-1 is the most recent line.
    ///
    /// @param i The line to retrieve.
    std::string GetLine(uint32_t i);

    /// \brief Clears all recorded messages in the Console.
    ///
    void Clear();


    /// \brief Sets the mode by which to display incoming Console messages.
    ///
    void OverlayMessageMode(MessageMode);
    
    /// \brief Returns the current message mode. MessageMode::Standard is the default.
    ///
    MessageMode GetOverlayMessageMode();

    /// \brief Sets a callback to be run for all user-entered commands, regardless of 
    /// command issued. Whether to process the command normally is returned.
    ///
    /// If specified, the console, before normally processing the command, 
    /// will call this function first to determine whether to continue.
    /// The compiled input string is given as the first argument of args.
    /// If the callback returns false, no further action is taken.
    void SetCommandCallback(DynacoeEvent((*)));

    void Init();
    void InitAfter();
    void RunBefore();
    void RunAfter();
    void DrawAfter();









};
}


#endif
