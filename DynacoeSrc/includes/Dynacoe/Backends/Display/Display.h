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

#ifndef DC_DISPLAY_H_INCLUDED
#define DC_DISPLAY_H_INCLUDED

/* Display 

   An interface for displaying rendered data

*/
#include <string>
#include <Dynacoe/Backends/Backend.h>
#include <Dynacoe/Backends/Framebuffer/Framebuffer.h>

namespace Dynacoe {



/// \brief Backend that controls how data is displayed to the user.
///
/// The display is abstracted as a rectangle, with a width, height, and position, that
/// consumes a rendered visual of data from a renderer. 
class Display : public Backend {
  public:

    /// \brief Base class for resize callbacks.
    ///
    class ResizeCallback {
        public:
        /// \brief Called upon resizing.
        ///
        /// @param newWidth The new width of the Display.
        /// @param newHeight The new height of the Display. 
        virtual void operator()(int newWidth, int newHeight) = 0;
    };

    /// \brief Base class for the trigger of a close signal from the user.
    ///
    /// Usually this is akin to the user pressing the "close" button on a window
    /// in their desktop environment.
    class CloseCallback {
        public:

        /// \brief The actual callback.
        ///
        virtual void operator()(void) = 0;
    };

    virtual ~Display(){}




    /// \brief The standard functional capabilities of a Display.
    ///
    enum class Capability {
        CanResize,      ///< The Display can be resized.
        CanMove,        ///< The Display's position can be moved.
        CanFullscreen,  ///< The Display's size can consume the entire physical device, often in a special state.
        CanHide,        ///< The Display can be hidden.
        CanLockSize     ///< The Display can prevent the user from changing the dimensions of the Display.
    };

    /// \brief Controls how the Display displays Rendered data.
    ///
    enum class ViewPolicy {
        NoPolicy,       ///< The Display will show the attached Framebuffer's contents with no transformation
        MatchSize,      ///< The Display will stretch the attached Framebuffer's contents to match the windows dimensions
    };



    
    /// \brief The variety of system handle types that Display can represent.
    ///
    enum class DisplayHandleType {
        X11Display,     ///< The display handle is an X11Display instance. In this case, the display is a window in an X11 environment
        WINAPIHandle,   ///< The display handle is a MS Windows HWND instance.
        Unknown         ///< The handle's type is not known and should not be relied on.
    };

    /// \brief The varienty of system event types that Display can give.
    enum class DisplayEventType {
        X11Event,      ///< The event is a pointer to a std::vector<XEvent>.
        WINAPIMsg,     ///< The event is a pointer to a std::vector<MSG>.
        Unknown        ///< The event's type is not known and should not be relied on.
        
    };
    
    /// \brief Resizes the display. If the display does not support resizing, no action is taken.
    ///    
    virtual void Resize(int w, int h) = 0;

    /// \brief Sets the position of the display. 
    ///
    /// Usually, this is relative to whatever environment
    /// the display exists in. For example, in a desktop environment, this could be
    /// an offset from the DE's origin. If the display does not support moving,
    /// no action is taken.
    virtual void SetPosition(int x, int y) = 0;

    /// \brief Set the display into a fullscreen context. If fullscreen is not supported,
    /// no action is taken.
    ///
    virtual void Fullscreen(bool) = 0;

    /// \brief Attempts to hide the display. If hiding is not supported, no action is taken.
    ///
    virtual void Hide(bool) = 0;

    /// \brief Returns whether the display has user input focus. On display implementations
    /// where this doesnt apply, i.e. where there is only one logical display available,,
    /// this will always return true. 
    virtual bool HasInputFocus() = 0; 

    /// \brief Attempts to prevent resizing on the user's side. 
    ///
    /// For example,
    /// in a desktop environment, this would disable the feature of resizing
    /// the window.
    virtual void LockClientResize(bool) = 0;

    /// \brief Attempts to prevent moving on the user's side. 
    /// 
    virtual void LockClientPosition(bool) = 0;

    /// \brief Controls how the Renderer's information is displayed. The default policy is "MatchSize"
    /// See ViewPolicy for more information. 
    ///
    virtual void SetViewPolicy(ViewPolicy) = 0;

    /// \brief Returns the width of the display.
    ///
    virtual int Width() = 0;

    /// \brief Returns the height of the display.
    ///
    virtual int Height() = 0;
    
    /// \brief Returns the X position of the display.
    ///
    virtual int X() = 0;

    /// \brief Returns the Y position of the display.
    ///
    virtual int Y() = 0;


   
    /// \brief Sets the name of the display. On some systems, this can, for example,
    /// set the title bar of the application to the specified name.
    virtual void SetName(const std::string &) = 0;

    /// \brief Adds an additional callback function to be be called after
    /// the occurance of a resize event.Callbacks are run in the order that they
    /// were added in.
    virtual void AddResizeCallback(ResizeCallback *) = 0;

    /// \brief Removes the callback of the same instance as one given via
    /// AddResizeCallback.
    virtual void RemoveResizeCallback(ResizeCallback *) = 0;

    /// \brief Adds an additional callback function to be be called after
    /// the occurance of a closing event.
    ///
    /// Typically on desktop systems,
    /// this is triggered by pressing the close button on the window
    /// associated with the Display. Callbacks are run in the order that they
    /// were added in.
    virtual void AddCloseCallback(CloseCallback *) = 0;
    /// \brief Removes the callback of the same instance as one given via
    /// AddCloseCallback.
    virtual void RemoveCloseCallback(CloseCallback *) = 0;



    /// \brief Returns whether or not the Display is able to 
    /// perform the requested capability.
    virtual bool IsCapable(Capability) = 0; 



    // Interfacing with others


    /// \brief Updates display with input visual data that is 
    /// populated in the internal framebuffer. THe framebuffer's
    /// data canbe populated by modifying the framebuffer in GetSource()
    virtual void Update(Framebuffer *) = 0;    

    // Returns the framebuffer types that this renderer supports.
    // Should the framebuffer not match one of the given types, the framebuffer 
    // attachment will fail
    virtual std::vector<Dynacoe::Framebuffer::Type> SupportedFramebuffers() = 0;

    /// \brief Returns the type associated with data returned by 
    /// GetSystemHandle. Meant for internal use, but can be handy when
    /// doing weird things.
    virtual DisplayHandleType GetSystemHandleType() = 0;

    /// \brief Returns an implementation-specific handle that represents this
    /// Display or this Display's properties. Meant for internal use, but can 
    /// be handy when doing weird things.
    virtual void * GetSystemHandle() = 0;
    
    /// \brief Returns the type of the system events returned by 
    /// GetLastSystemEvent(). Like getting the system handle, it's mostly meant for internal 
    /// use, but can come in handy when doing weird things.
    virtual DisplayEventType GetSystemEventType() = 0;

    /// \brief Returns an implementation-specific value that represents the 
    /// last processed event generated form the display. It is guaranteed to be updated
    /// after Update() has been called and is valid until the next Update() call.
    virtual void * GetLastSystemEvent() = 0;
    
    

};






}

#endif
