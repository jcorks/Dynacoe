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

#ifndef H_DC_RENDER_LIGHT
#define H_DC_RENDER_LIGHT

#include <Dynacoe/Component.h>
#include <Dynacoe/Color.h>
#include <Dynacoe/Backends/Renderer/Renderer.h>

namespace Dynacoe {

/** \brief A 3D aspect that signifies a light. 
 *
 */
class RenderLight : public Component {
  public:
    RenderLight();
    ~RenderLight();

    /// Types of lights that the RenderLight can assume.
    ///
    enum class Light {
        Point,        ///< Vector lights have a single location in space. The intensity of the light is dependent on how close other objects are.
        Directional,  ///< Directional lights have a constant effect on object regardless of the object's proximity.
    };




    /// \brief Defines the light.
    ///
    /// @param type The Light type that the light should be.
    void FormLight(Light type);

    /// \brief Resets the RenderLight.
    ///
    void Clear();

    /// \brief Enables or disables the light.
    ///
    void Enable(bool);
    
    /// \brief Attributes of a light.
    struct LightAttributes {
        bool operator==(const LightAttributes & other) const {
            return (position == other.position &&
                    color == other.color &&
                    intensity == other.intensity);
        }
        /// \brief Global / absolute position of the light.
        ///
        Vector position;
        
        /// \brief Intensity of the light.
        ///
        float intensity;

        /// \brief Color of the light.
        ///
        Color color;
    };

    /// \brief Attributes of the light. Edit these to 
    /// modify the light.
    ///
    LightAttributes state;
    

    void OnDraw();
    std::string GetInfo();
    
  private:

    LightID lightID;
    Light type;
    LightAttributes current;
    Vector currentPos;
    bool enabled;

};
}

#endif
