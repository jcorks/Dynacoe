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

/*  Example that acts as a basic text editor
    2014, Johnathan Corkery
 */
#include <Dynacoe/Library.h>
using namespace Dynacoe;


// StringExample allows you to type in a text editor-like fashion, entering 
// new lines with the "enter" key and removing characters with "backspace"
class StringExample : public Dynacoe::Entity {

  public:

    StringExample() {
        SetName("Typer");
        textObject = AddComponent<Text2D>();
        // Lets load in the font. By default, Dynacoe 
        // can read OTF and TTF fonts.
        textObject->SetFont    (Assets::Load("ttf", "monospace.ttf"));
        textObject->SetFontSize(24);

        textObject->text =     "Try typing!";
        
        // There exist different spacing modes, discussed below.
        textObject->SetSpacingMode(Text2D::SpacingMode::Monospace);



    }

    void OnStep() {
     
        // This controles when to add additional characters to 
        // this displayed string. GetLastUnicode() is a nifty function for 
        // retrieving the last character the user typed. This takes into 
        // account Shift.
        int character;
        if (character = Input::GetLastUnicode()) {
            if (character == '\b') {
                text = text.substr(0, text.size()-1);
            } else {
                text += (char)character;
            }
            textObject->text =  text + '|';
        }
        


        // Text rendering with Dynacoe gives you a few options to 
        // display character spacings. 'Kerned' text draws the text 
        // using the natural spacing determined by the font. This is the 
        // most aesthetically pleasing, but is more work to predict 
        // how much real estate the text will require.
        if (Input::IsPressed(Keyboard::Key_right)) {
            textObject->SetSpacingMode(Text2D::SpacingMode::Kerned);
            Console::Info() << "Changed spacing mode to: Kerned\n";
        } 

        // 'Bitmap' text will draw the text with spacing determined from 
        // just the visual. Rarely useful, but you never know!
        if (Input::IsPressed(Keyboard::Key_left)) {
            textObject->SetSpacingMode(Text2D::SpacingMode::Bitmap);
            Console::Info() << "Changed spacing mode to: Bitmap\n";
        } 

        // 'Monospace' text will draw the text with the same spacing 
        // regardless of the character. THis is very useful because, in this mode, 
        // you can predict the space usage of the text to be drawn just by 
        // the character count.
        if (Input::IsPressed(Keyboard::Key_up)) {
            textObject->SetSpacingMode(Text2D::SpacingMode::Monospace);
            Console::Info() << "Changed spacing mode to: Monospace\n";

        } 
        
     

    }
  private:

    Dynacoe::Text2D  * textObject;
    Dynacoe::Shape2D * cursor;
    
    std::string text;

};




// Usual driver as in previous examples
int main() {
    Engine::Startup();    

    ViewManager::NewMain("Text Example");

    Engine::Root() = Entity::Create<StringExample>();
    Engine::Run(); 
    return 0;
}

