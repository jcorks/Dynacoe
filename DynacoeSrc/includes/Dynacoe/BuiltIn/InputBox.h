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

#ifndef INPUT_BOX_H_INCLUDED
#define INPUT_BOX_H_INCLUDED

#include <Dynacoe/Entity.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Components/GUI.h>
#include <Dynacoe/Components/Text2D.h>

namespace Dynacoe {


/// \brief Allows a user to type inside of it.
///
class InputBox : public Dynacoe::Entity {
    public:

        InputBox();

        /// \brief Resizes the InputBox region.
        ///
        void Resize(int, int);
        
        /// \brief Sets the text within the InputBox.
        ///
        void SetText(const std::string &);
    
        /// \brief Reutrns the text currently within the InputBox.
        ///
        std::string GetText();
        
        /// \brief Sets whether to activate the InputBox.
        /// When activated, the InputBox will accept input.
        ///
        void Activate(bool act=true);


    protected:
        void OnStep();
        void OnDraw();

    private:

        static DynacoeEvent(Event_OnClick);

        GUI gui;
        Shape2D bg;
        Text2D text2D;
        std::string text;
        std::string visibleText;
        Color textColor;
};
}


#endif //INPUT_BOX_H_INCLUDED
