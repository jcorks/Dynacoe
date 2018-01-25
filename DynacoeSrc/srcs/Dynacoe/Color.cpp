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


#include <Dynacoe/Color.h>
#include <cmath>
#include <cstdio>

using namespace Dynacoe;

static std::unordered_map<std::string, Color>  * dictionary = nullptr;;
static void LoadColors();
Color::Color() {
    r = 0;
    g = 0;
    b = 0;
    a = 255;
}

Color::Color(const Color & c) {
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
}
Color::Color(int red, int green, int blue, int alpha) {
    r = red;
    g = green;
    b = blue;
    a = alpha;
}

Color::Color(float red, float green, float blue, float alpha) {
    r = red;
    g = green;
    b = blue;
    a = alpha;
}

Color::Color(const std::vector<Component> & c) {
    for(int i = 0; i < (int)c.size() && i < 4; ++i) {
        switch(i) {
          case 0: r = c[i]; break;
          case 1: g = c[i]; break;
          case 2: b = c[i]; break;
          case 3: a = c[i]; break;
        }
    }
}
Color::Color(const char * str) {
    *this = std::string(str);
}

Color::Color(const std::string & src) {
    if (!dictionary) {
        LoadColors();
    }
    // names are not case sensitive;
    std::string str(src);
    for(int i = 0; i < str.length(); ++i) {
        str[i] = toupper(str[i]);
    }
    a = 255;
    if (str[0] == '#' && str.size() == 9 || str.size() == 7) {
        str = str.substr(1, str.length());
        uint8_t cl = 0;
        uint8_t val = 0;
        for(int i = 0; i < str.size(); ++i) {
            val = str[i];
            val = (val >= '0' && val <= '9' ? val - '0' : (val - 'A' + 10));
            cl += val*pow(16, i%2==0? 1 : 0);

            switch(i) {
              case 1: r = cl; cl = 0; break;
              case 3: g = cl; cl = 0; break;
              case 5: b = cl; cl = 0; break;
              case 7: a = cl; cl = 0; break;
              default:;
            }

        }
    } else {
        *(this) = ((dictionary->count(str))? dictionary->find(str)->second : Color("white"));
    }
    if (a.Byte() != 255)
        printf("%d\n", a.Byte());
}

Color::Color(uint32_t clr) {
    uint8_t * data;
    data = (uint8_t *) &clr;

    r = data[0];
    g = data[1];
    b = data[2];
    a = data[3];
}

Color & Color::Define(uint32_t c) {
    (*this) = Color(c);
    return (*this);
}

Color Color::operator+ (const Color & c) const {
    Color out = (*this);
    out.r += c.r;
    out.g += c.g;
    out.b += c.b;
    out.a += c.a;
    return out;
}

Color Color::operator- (const Color & c) const {
    Color out = (*this);
    out.r -= c.r;
    out.g -= c.g;
    out.b -= c.b;
    out.a -= c.a;
    return out;
}

Color Color::operator* (const Color & c) const {
    Color out = (*this);
    out.r *= c.r;
    out.g *= c.g;
    out.b *= c.b;
    out.a *= c.a;
    return out;
}

Color Color::operator/ (float f) const {
    Color out = (*this);
    out.r /= f;
    out.g /= f;
    out.b /= f;
    out.a /= f;
    return out;
}

Color Color::operator* (float f) const {
    Color out = (*this);
    out.r *= f;
    out.g *= f;
    out.b *= f;
    out.a *= f;
    return out;
}

Color & Color::operator+= (const Color & c) {
    (*this) = (*this) + c;
    return (*this);
}

Color & Color::operator-= (const Color & c) {
    (*this) = (*this) - c;
    return (*this);
}

Color & Color::operator=(const Color & c) {
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
    return (*this);
}




uint32_t Color::Int() const {

    uint8_t data[4];

    data[0] = r.Byte();
    data[1] = g.Byte();
    data[2] = b.Byte();
    data[3] = a.Byte();

    return *((uint32_t *) data);
}





Color &  Color::Define(int red, int green, int blue, int alpha) {
    r = red;
    g = green;
    b = blue;
    a = alpha;
    return (*this);
}

bool Color::operator==(const Color & c) const {
    return (r.Byte() == c.r.Byte() &&
            b.Byte() == c.b.Byte() &&
            g.Byte() == c.g.Byte() &&
            a.Byte() == c.a.Byte());
}

bool Color::operator!=(const Color & c) const {
    return !(r.Byte() == c.r.Byte() &&
             b.Byte() == c.b.Byte() &&
             g.Byte() == c.g.Byte() &&
             a.Byte() == c.a.Byte());
}



void Color::Define(const std::string & src, const Color & c) {
    if (!dictionary) {
        LoadColors();
    }
    std::string str = src;
    for(int i = 0; i < str.length(); ++i) {
        str[i] = toupper(str[i]);
    }
    (*dictionary)[str] = c;



}

std::string Color::ToString() const {
    char cstr[30];
    sprintf(cstr, "#%02x%02x%02x%02x",
        r.Byte(),
        g.Byte(),
        b.Byte(),
        a.Byte()
    );
    return std::string(cstr);
}


void LoadColors() {
    dictionary = new std::unordered_map<std::string, Color>;

    Color::Define("Alice Blue",     Color("#f0f8ff"));
    Color::Define("Antique white",  Color("#faebd7"));
    Color::Define("Aqua",           Color("#00ffff"));
    Color::Define("Aquamarine",     Color("#7fffd4"));
    Color::Define("Azure",          Color("#f0ffff"));


    Color::Define("Beige",          Color("#f5f5dc"));
    Color::Define("Bisque",         Color("#FFe4c4"));
    Color::Define("Black",          Color("#000000"));
    Color::Define("Blanched almond",Color("#ffebcd"));
    Color::Define("Blue",           Color("#0000ff"));
    Color::Define("Blue Violet",    Color("#8a2be2"));
    Color::Define("Brown",          Color("#a52a2a"));
    Color::Define("Burlywood",      Color("#deb887"));


    Color::Define("Cadet blue",     Color("#5F9EA0"));
    Color::Define("Chartreuse",     Color("#7fff00"));
    Color::Define("Chocolate",      Color("#d2691E"));
    Color::Define("Coral",          Color("#FF7F50"));
    Color::Define("Cornflower",     Color("#6495ed"));
    Color::Define("Cornsilk",       Color("#fff8dc"));
    Color::Define("Crimson",        Color("#dc143c"));
    Color::Define("Cyan",           Color("#00ffff"));


    Color::Define("Dark Blue",      Color("#00008b"));
    Color::Define("Dark Cyan",      Color("#008b8b"));
    Color::Define("Dark Goldenrod", Color("#B8860b"));
    Color::Define("Dark Gray",      Color("#a9a9a9"));
    Color::Define("Dark Green",     Color("#006400"));
    Color::Define("Dark Khaki",     Color("#BdB76B"));
    Color::Define("Dark Magenta",   Color("#8B008B"));
    Color::Define("Dark Olive Green",Color("#556b2f"));
    Color::Define("Dark Orange",    Color("#FF8c00"));
    Color::Define("Dark Orchid",    Color("#9932cc"));
    Color::Define("Dark Red",       Color("#8B0000"));
    Color::Define("Dark Salmon",    Color("#e9967a"));
    Color::Define("Dark Sea Green", Color("#8FBc8f"));
    Color::Define("Dark Slate Blue",Color("#483d8b"));
    Color::Define("Dark Slate Gray",Color("#2f4f4f"));
    Color::Define("Dark Turquoise", Color("#00ced1"));
    Color::Define("Dark Violet",    Color("#9400d3"));
    Color::Define("Deep Pink",      Color("#ff1493"));
    Color::Define("Deep Sky Blue",  Color("#00bfff"));
    Color::Define("Dim Gray",       Color("#696969"));
    Color::Define("Dodger Blue",    Color("#1e90ff"));


    Color::Define("Firebrick",      Color("#822222"));
    Color::Define("Floral White",   Color("#FFFAF0"));
    Color::Define("Forest green",   Color("#228b22"));
    Color::Define("Fuchsia",        Color("#ff00ff"));



    Color::Define("Gainsboro",      Color("#dcdcdc"));
    Color::Define("Ghost white",    Color("#f8f8ff"));
    Color::Define("Gold",           Color("#FFd700"));
    Color::Define("Goldenrod",      Color("#daa520"));
    Color::Define("Gray",           Color("#bebebe"));
    Color::Define("Web Gray",       Color("#808080"));
    Color::Define("Green",          Color("#00FF00"));
    Color::Define("Web Green",      Color("#008000"));
    Color::Define("Green Yellow",   Color("#adff2f"));


    Color::Define("Honeydew",       Color("#f0fff0"));
    Color::Define("Hot Pink",       Color("#ff69bf"));

    Color::Define("Indian Red",     Color("#cd5c5c"));
    Color::Define("Indigo",         Color("#4b0082"));
    Color::Define("Ivory",          Color("#fffff0"));

    Color::Define("Khaki",          Color("#f0e68c"));

    Color::Define("Lavender",       Color("#e6e6fa"));
    Color::Define("Lavender Blush", Color("#fff0f5"));
    Color::Define("Lawn Green",     Color("#7cfc00"));
    Color::Define("Lemon Chiffon",  Color("#fffacd"));
    Color::Define("Light Blue",     Color("#add8e6"));
    Color::Define("Light Coral",    Color("#f08080"));
    Color::Define("Light Cyan",     Color("#eoffff"));
    Color::Define("Light Goldenrod",Color("#fafad2"));
    Color::Define("Light Gray",     Color("#d3d3d3"));
    Color::Define("Light Green",    Color("#90ee90"));
    Color::Define("Light Pink",     Color("#ffb6c1"));
    Color::Define("Light Salmon",   Color("#ffa07a"));
    Color::Define("Light Sea Green",Color("#20b2aa"));
    Color::Define("Light Sky Blue", Color("#87cefa"));
    Color::Define("Light Slate Gray",Color("#778899"));
    Color::Define("Light Steel Blue",Color("#b0c6de"));
    Color::Define("Light Yellow",   Color("#ffffe0"));
    Color::Define("Lime",           Color("#00FF00"));
    Color::Define("Lime Green",     Color("#3dcd32"));
    Color::Define("Linen",          Color("#faf0e6"));



    Color::Define("Mangenta",       Color("#FF00FF"));
    Color::Define("Maroon",         Color("#bo3060"));
    Color::Define("Web Maroon",     Color("#7f0000"));
    Color::Define("Medium Aquamarine", Color("#66cdaa"));
    Color::Define("Medium Blue",    Color("#0000cd"));
    Color::Define("Medium Orchid",  Color("#ba55d3"));
    Color::Define("Medium Purple",  Color("#9370d8"));
    Color::Define("Medium Sea Green", Color("#3cb371"));
    Color::Define("Medium Slate Blue",Color("#7b68ee"));
    Color::Define("Medium Spring Green",Color("#00fa9a"));
    Color::Define("Medium Turquoise", Color("#48d1cc"));
    Color::Define("Medium Violet Red",Color("#c71585"));
    Color::Define("Midnight Blue",  Color("#191970"));
    Color::Define("Mint Cream",     Color("#F5fffa"));
    Color::Define("Misty Rose",     Color("#ffe4e1"));
    Color::Define("Moccasin",       Color("#ffe4b5"));


    Color::Define("Navajo White",   Color("#ffdead"));
    Color::Define("Navy Blue",      Color("#000080"));


    Color::Define("Old Lace",       Color("#fdf5e6"));
    Color::Define("Olive",          Color("#808000"));
    Color::Define("Olive Drab",     Color("#6b8e23"));
    Color::Define("Orange",         Color("#FFa500"));
    Color::Define("Orange Red",     Color("#ff4500"));
    Color::Define("Orchid",         Color("#da70d6"));


    Color::Define("Pale Goldenrod", Color("#eee8aa"));
    Color::Define("Pale Green",     Color("#98fb98"));
    Color::Define("Pale Turquoise", Color("#afeeee"));
    Color::Define("Pale Violet Red",Color("#db7093"));
    Color::Define("Papaya Whip",    Color("#ffefd5"));
    Color::Define("Peach Puff",     Color("#ffdab9"));
    Color::Define("Peru",           Color("#cd853f"));
    Color::Define("Pink",           Color("#ffc0cb"));
    Color::Define("Plum",           Color("#ddaodd"));
    Color::Define("Powder Blue",    Color("#boe0e6"));
    Color::Define("Purple",         Color("#a020f0"));
    Color::Define("Web Purple",     Color("#7f007f"));

    Color::Define("Rebecca Purple", Color("#663399"));
    Color::Define("Red",            Color("#FF0000"));
    Color::Define("Rosy Brown",     Color("#BC8F8F"));
    Color::Define("Royal Blue",     Color("#4169E1"));


    Color::Define("Saddle Brown",   Color("#8B4513"));
    Color::Define("Salmon",         Color("#FA8072"));
    Color::Define("Sandy Brown",    Color("#F4A460"));
    Color::Define("Sea Green",      Color("#2e8b57"));
    Color::Define("Seashell",       Color("#fff5ee"));
    Color::Define("Sienna",         Color("#A0522d"));
    Color::Define("Silver",         Color("#c0c0c0"));
    Color::Define("Sky Blue",       Color("#87ceeb"));
    Color::Define("Slate Blue",     Color("#6A5ACD"));
    Color::Define("Slate Gray",     Color("#708090"));
    Color::Define("Snow",           Color("#FFFAFA"));
    Color::Define("Spring Green",   Color("#00FF7f"));
    Color::Define("Steel blue",     Color("#4682b4"));

    Color::Define("Tan",            Color("#D2B48c"));
    Color::Define("Teal",           Color("#008080"));
    Color::Define("Thistle",        Color("#D8BFD8"));
    Color::Define("Tomato",         Color("#FF6347"));
    Color::Define("Turquoise",      Color("#40E0D0"));
    Color::Define("Violet",         Color("#ee82ee"));
    Color::Define("Wheat",          Color("#F5DEB3"));
    Color::Define("White",          Color("#ffffff"));
    Color::Define("White Smoke",    Color("#f5f5f5"));
    Color::Define("Yellow",         Color("#ffff00"));
    Color::Define("Yellow Green",   Color("#9acd32"));

}
