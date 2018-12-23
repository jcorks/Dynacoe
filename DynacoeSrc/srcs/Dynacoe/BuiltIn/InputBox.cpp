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

#include <Dynacoe/BuiltIn/InputBox.h>
#include <Dynacoe/Modules/Input.h>

#include <cstdlib>
#include <sstream>
//#include <ioutils>

using namespace Dynacoe;

// focus event
DynacoeEvent(InputBox::Event_OnClick) {
    InputBox * host = self.IdentifyAs<InputBox>();
    host->Activate(true);
    return true;
}




static Color activeText (240,240,240,255);
static Color activeBox  (30,30,30,255);
static Color dormantText(128,128,128,255);
static Color dormantBox (10, 10, 10, 255);







void InputBox::Activate(bool act) {
    if (act) {
        gui->SetFocus();
    } else {
        gui->Unfocus();
    }        
}

void InputBox::Resize(int w, int h) {
    bg->FormRectangle(w, h);
    gui->DefineRegion(w, h);
}

std::string InputBox::GetText() {
    return text;
}

void InputBox::SetText(const std::string & str) {
    text = str;
    visibleText = str;
}










InputBox::InputBox() : Entity("InputBox") {
    text   = ""; 
    visibleText = "";

    gui = AddComponent<GUI>();
    bg = AddComponent<Shape2D>();
    text2D = AddComponent<Text2D>();
    Resize(50, 12);
    
    

    gui->InstallHandler("on-click", Event_OnClick);
}



void InputBox::OnStep() {

    if (gui->IsFocused()) {
        char last = 0;
        if ((last = Input::GetLastUnicode())) {
            if (last == '\n') {
                Activate(false);
                return;
            } else if (last == '\b') {
                text = text.substr(0, text.size()-1); 
            } else {
                text+=last;
            }


            int maxLen = gui->GetRegionW() / 6 - 1; // div by char widget
            if (text.size() >= maxLen) {
                visibleText = text.substr(text.size() - maxLen, maxLen);
            } else {
                visibleText = text;
            }
        }

        textColor = activeText;
        bg->color = activeBox;
    } else if (gui->IsHovered())  {
        bg->color = (dormantBox + Color(10, 40, 10, 0));
    } else {
        textColor = dormantText;
        bg->color = (dormantBox);
    }
}


void InputBox::OnDraw() {
    int w = gui->GetRegionW();
    int h = gui->GetRegionH();

    std::string drawnText = visibleText;
    if (gui->IsFocused())
        drawnText += "|";

    text2D->text = drawnText;
    text2D->SetTextColor(textColor);
}



