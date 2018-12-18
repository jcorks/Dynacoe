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

#define ANDROID
#ifndef H_DC_DISPLAY_AndroidBridge_Display
#define H_DC_DISPLAY_AndroidBridge_Display

#include <Dynacoe/Backend/Display/Display.h>


namespace Dynacoe {


// Serves as a bridge to the 
class AndroidBridge_Display_Data;
class AndroidBridge_Display : public Display {
  public:
    AndroidBridge_Display();
    ~AndroidBridge_Display();
 
    
    /// \brief Resizes the display. If the display does not support resizing, no action is taken.
    ///    
    void Resize(int w, int h){}

    /// \brief Sets the position of the display. 
    ///
    /// Usually, this is relative to whatever environment
    /// the display exists in. For example, in a desktop environment, this could be
    /// an offset from the DE's origin. If the display does not support moving,
    /// no action is taken.
    void SetPosition(int x, int y){}

    /// \brief Set the display into a fullscreen context. If fullscreen is not supported,
    /// no action is taken.
    ///
    void Fullscreen(bool);

    /// \brief Attempts to hide the display. If hiding is not supported, no action is taken.
    ///
    void Hide(bool){}

    /// \brief Returns whether the display has user input focus. On display implementations
    /// where this doesnt apply, i.e. where there is only one logical display available,,
    /// this will always return true. 
    bool HasInputFocus();

    /// \brief Attempts to prevent resizing on the user's side. 
    ///
    /// For example,
    /// in a desktop environment, this would disable the feature of resizing
    /// the window.
    void LockClientResize(bool);

    /// \brief Attempts to prevent moving on the user's side. 
    /// 
    void LockClientPosition(bool);

    /// \brief Controls how the Renderer's information is displayed. The default policy is "MatchSize"
    /// See ViewPolicy for more information. 
    ///
    void SetViewPolicy(ViewPolicy);

    /// \brief Returns the width of the display.
    ///
    int Width();

    /// \brief Returns the height of the display.
    ///
    int Height();
    
    /// \brief Returns the X position of the display.
    ///
    int X() {return 0;}

    /// \brief Returns the Y position of the display.
    ///
    int Y() {return 0;}


   
    /// \brief Sets the name of the display. On some systems, this can, for example,
    /// set the title bar of the application to the specified name.
    void SetName(const std::string &){}

    /// \brief Adds an additional callback function to be be called after
    /// the occurance of a resize event.Callbacks are run in the order that they
    /// were added in.
    void AddResizeCallback(ResizeCallback *);

    /// \brief Removes the callback of the same instance as one given via
    /// AddResizeCallback.
    void RemoveResizeCallback(ResizeCallback *);

    /// \brief Adds an additional callback function to be be called after
    /// the occurance of a closing event.
    ///
    /// Typically on desktop systems,
    /// this is triggered by pressing the close button on the window
    /// associated with the Display. Callbacks are run in the order that they
    /// were added in.
    void AddCloseCallback(CloseCallback *);
    /// \brief Removes the callback of the same instance as one given via
    /// AddCloseCallback.
    void RemoveCloseCallback(CloseCallback *);



    /// \brief Returns whether or not the Display is able to 
    /// perform the requested capability.
    bool IsCapable(Capability capability); {
        switch(capability) {
          case Dynacoe::Capability::CanResize:     return false;
          case Dynacoe::Capability::CanMove:       return false;
          case Dynacoe::Capability::CanFullscreen: return true;
          case Dynacoe::Capability::CanHide:       return false;
          case Dynacoe::Capability::CanLockSize:   return false;
        }
    }



    // Interfacing with others


    /// \brief Updates display with input visual data that is 
    /// populated in the internal framebuffer. THe framebuffer's
    /// data canbe populated by modifying the framebuffer in GetSource()
    void Update(Framebuffer *); 

    // Returns the framebuffer types that this renderer supports.
    // Should the framebuffer not match one of the given types, the framebuffer 
    // attachment will fail
    std::vector<Dynacoe::Framebuffer::Type> SupportedFramebuffers() {
        return {Dynacoe::Framebuffer::Type::GLFBPacket};
    }

    /// \brief Returns the type associated with data returned by 
    /// GetSystemHandle. Meant for internal use, but can be handy when
    /// doing weird things.
    DisplayHandleType GetSystemHandleType() {
        return {Dynacoe::Display::DisplayHandleType::Unknown};
    }

    /// \brief Returns an implementation-specific handle that represents this
    /// Display or this Display's properties. Meant for internal use, but can 
    /// be handy when doing weird things.
    void * GetSystemHandle() {return nullptr;}
    
    /// \brief Returns the type of the system events returned by 
    /// GetLastSystemEvent(). Like getting the system handle, it's mostly meant for internal 
    /// use, but can come in handy when doing weird things.
    DisplayEventType GetSystemEventType() {return Display::DisplayEventType::Unknown;}

    /// \brief Returns an implementation-specific value that represents the 
    /// last processed event generated form the display. It is guaranteed to be updated
    /// after Update() has been called and is valid until the next Update() call.
    void * GetLastSystemEvent() {return nullptr;}
    
  private:
    AndroidBridge_Display_Data * ABD;

};


}


#endif
#endif
#endif
