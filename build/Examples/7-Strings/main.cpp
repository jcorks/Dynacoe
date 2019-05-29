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





// This example will involve 2 classes.
//
// The TextHandler here actually handles incoming input from the keyboard.
// Instead of actually looking for specific keys, the Dynacoe::Input has an 
// alternative for text: UnicodeListeners. These objects, once customized, 
// respond to actual text from a keyboard and allow for custom 
// behavior after specific text is detected.
class TextHandler : public Dynacoe::UnicodeListener {
  public:
    // The TextHandler holds a copy to the main Entity's Text2D
    Text2D * textObject ;

    // The full string displayed.
    std::string text;



    TextHandler(Text2D * ref) {
        textObject = ref;

        // Lets load in the font. By default, Dynacoe 
        // can read OTF and TTF fonts.
        textObject->SetFont    (Assets::Load("ttf", "monospace.ttf"));
        textObject->SetFontSize(24);
        textObject->text =     "Try typing!";
        
        // There exist different spacing modes, discussed below.
        textObject->SetSpacingMode(Text2D::SpacingMode::Monospace);


    }


    // This function is a built-in function called when the UnicodeListener 
    // detects a new character is entered in the keyboard.
    // Note the special characters.
    void OnNewUnicode(int character) {

        // 17, 18, 19, and 20 are special characters that represent the arrow keys.
        if (character == 17) {
            Console::Info() << "Left arrow!\n";
            return;
        }

        if (character == 18) {
            Console::Info() << "Up arrow!\n";
            return;
        }

        if (character == 19) {
            Console::Info() << "Right arrow!\n";
            return;
        }

        if (character == 20) {
            Console::Info() << "Down arrow!\n";
            return;
        }


        // \b is the backspace character!
        if (character == '\b') {
            text = text.substr(0, text.size()-1);
        } else {
            text += (char)character;
        }

        textObject->text =  text + '|';
    }


    // If an input is held on a keyboard, it may send a "repeat request"
    // for quicker input. This usually only makes sense for text input.
    // If we get a request to repeat, we will just run the normal text addition 
    // function.
    void OnRepeatUnicode(int character) {
        OnNewUnicode(character);
    }
};


// StringExample allows you to type in a text editor-like fashion, entering 
// new lines with the "enter" key and removing characters with "backspace"
class StringExample : public Dynacoe::Entity {

  public:



    StringExample() {
        SetName("Typer");
        textObject = AddComponent<Text2D>();

        // Here we instantiate the text handler. At 
        // this point, it isnt activated and wont actually listen 
        // to keys yet.
        handler = new TextHandler(textObject);


        // This call activates the listener        
        Dynacoe::Input::AddUnicodeListener(handler);

    }



    void OnStep() {

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
    TextHandler * handler;
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

