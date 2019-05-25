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


#include <Dynacoe/Modules/ViewManager.h>
#include <Dynacoe/Util/Time.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Util/Random.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <ctime>
#include <cassert>
#include <cstring>
#include <set>


//#include <arpa/inet.h>
//for mem status.
//#include <windows.h>
//#include <psapi.h>
    
#define RESIZE_GRACE_PERIOD 20


using namespace std;
using namespace Dynacoe;




struct LogicalDisplay {
    Display * display;
};

static Dynacoe::Table<LogicalDisplay> displays;
static ViewID currentDisplay;






static std::set<LookupID> ids;
#ifdef _WIN32
    #include <direct.h>
#endif






ViewID ViewManager::New(const std::string & name, int w, int h) {
    LogicalDisplay out;   
    out.display = (Display*) Backend::CreateDefaultDisplay();

    if (!out.display->Valid()) {
        Console::Error()  << "No valid Display could be created!"<< Console::End();
        return ViewID();
    }   
    out.display->SetName(name);
    out.display->Resize(w, h);

    /*
    out.fb = (Framebuffer*) Backend::CreateDefaultFramebuffer();
    out.display->AttachSource(out.fb);
    */

    ViewID id = displays.Insert(out);
    ids.insert(id);
    return id;

}


Dynacoe::Display * ViewManager::NewMain(const std::string & name, int w, int h) {
    auto id = New(name, w, h);
    SetMain(id);
    
    Graphics::GetRenderCamera().SetRenderResolution(w, h);
    return Get(id);
}

std::string ViewManager::GetCurrentClipboardAsString() {
    auto disp = Get(currentDisplay);
    if (!disp) return "";

    // usually the clipboard will give raw data, so 
    // the data buffer might not be null-terminated.
    auto data = disp->GetCurrentClipboard();
    char * dataBuffered = new char[data.size()+1];
    memcpy(dataBuffered, &data[0], data.size());
    dataBuffered[data.size()] = 0;
    
    std::string out(dataBuffered);
    delete[] dataBuffered;
    return out;
}

void ViewManager::SetCurrentClipboardAsString(const std::string & str) {
    auto disp = Get(currentDisplay);
    if (!disp) return;

    std::vector<uint8_t> dataReal;
    dataReal.resize(str.size()+1);
    dataReal[str.size()] = 0;
    memcpy(&dataReal[0], str.c_str(), str.size());

    disp->SetCurrentClipboard(dataReal);
}


void ViewManager::Destroy(ViewID id) {
    if (!displays.Query(id)) return;

    ids.erase(id);
    displays.Remove(id);


    if (id == currentDisplay) {
        if (ids.empty()) {
            Graphics::GetRenderer()->AttachTarget(nullptr);
        } else {
            SetMain(*ids.begin());
        }
    }
    
}


Display * ViewManager::Get(ViewID id) {
    return (displays.Query(id) ? displays.Find(id).display : nullptr);
}


void ViewManager::SetMain(ViewID id) {
    LogicalDisplay disp;
    if (!displays.Query(id)) {
        Console::Error()  << "Could not set main Display: invalid display given!"<< Console::End();
        return;
    }
    disp = displays.Find(id);
    if (!Graphics::GetRenderer()->Valid()) {
        Console::Error()  << "A fatal error occurred while trying to attach the display: the renderer is not in a usable state. Exiting."<< Console::End();
        Engine::Quit();        
        return;
    }
    
    currentDisplay = id;
}


ViewID ViewManager::GetCurrent() { return currentDisplay; }
int ViewManager::GetViewHeight() {
    if (!displays.Query(currentDisplay)) return -1;
    LogicalDisplay d = displays.Find(currentDisplay);
    return d.display->Height();
}
int ViewManager::GetViewWidth()  {
    if (!displays.Query(currentDisplay)) return -1;
    LogicalDisplay d = displays.Find(currentDisplay);
    return d.display->Width();
}


std::vector<ViewID> ViewManager::ListViews() {
    return std::vector<ViewID> (
        ids.begin(),
        ids.end()
    );
}




















    


