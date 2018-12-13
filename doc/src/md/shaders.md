Using Shaders: GLSL {#shaders-glsl}
===================

Using the included OpenGL renderer, it is possible to include and run your own shader programs 
in the GLSL language with your project. Depending on what version of OpenGL you have running, Dynacoe will support 
- GLSL 1.2
- GLSL 1.3
- GLSL 1.4


For maximum portability, it's probably best to stick to GLSL 1.2 (yeah, I know it's old.. but it
should be good enough for most things!)

Besides, along with pure language support, 
Dynacoe provides interfacing and utility functions to help write your programs.

Uniforms
--------

These uniforms provide a mechanism to interface with static data provided 
when issuing a render. It allows you to access material data 
and calculated parameters from the camera.
 
```{.c}

//_________ Camera Matrices _________//

// 4x4 matrix containing view transform from the current Camera.
mat4 Dynacoe_ViewTransform;

// 4x4 matrix containing the projection matrix from the current Camera.
mat4 Dynacoe_ProjectionTransform;

// 4x4 matrix containing object-local transform from the current 
// AspectMesh / StaticState being drawn.
mat4 Dynacoe_ModelTransform;

// Equivalent to inverse(transpose(Dynacoe_ViewTransform)) to 
// correctly transform normals. (Rarely useful, you will usually 
// want Dynacoe_ModelNormalTransform)
mat4 Dynacoe_ViewNormalTransform;

// Equivalent to inverse(transpose(Dynacoe_ModelTransform)) to 
// correctly transform normals. 
mat4 Dynacoe_ModelNormalTransform;




//__________ Material data ___________//

// Ambient light color from the material
vec3 Dynacoe_MaterialAmbient;

// Diffuse light RGB of the attached material
vec3 Dynacoe_MaterialDiffuse;

// Intensity of the diffuse color of the attached material 
float Dynacoe_MaterialDiffuseAmount;

// Specular light RGB of the attached material 
vec3 Dynacoe_MaterialSpecular;

// Intensity of the specular color of the attached material 
float Dynacoe_MaterialSpecularAmount;

// The shininess amount from the attached material
float Dynacoe_MaterialShininess;

// The auxiliary data set by the user in the material
vec4[8] Dynacoe_MaterialData;


```



Inputs
------

Usually in GLSL, you need to specify the inputs into the Vertex shader using the `in` or `varying` qualifier.
Dynacoe handles the linking of the data under-the-hood, so that part is done for you. As such, every vertex shader will 
have the same inputs:

```{.c}
// The position of the input vertex.
vec3 Dynacoe_Position;

// The normal of the input vertex.
vec3 Dynacoe_Normal;

// The texture coordinates of the input vertex.
vec2 Dynacoe_UV;

// User-specified data specified through the Mesh. (See Dynacoe::Mesh)
vec4 Dynacoe_Input;
```

These represent the vertex position, normal vector, and texture coordinates respectively. You have explicit 
control over these attriutes via AspectMesh, Mesh, and MeshObject, since these are the only kinds of objects 
that are drawn with shaders.


Sampling
--------

Dynacoe does not provide any means to connect to `sampler`s. Instead, Dynacoe allows you to sample 
all the textures you've specified in the meshing interface. Each time you AddTexture() to an Material,
a unique index refers to that texture. Starting at 0, each new texture gets the subsequent index
within the Material. 
```{.c}

// Samples the given texture index at the given texture coordinates.
// If textureIndex does not refer to a valid texture slot, the resulting 
// color is undefined.
vec4 Dynacoe_SampleColor(in int textureIndex, in vec2 uv);

// Samples shininess information from the given texture. The shiny componeny is the first
float Dynacoe_SampleShininess(in int textureIndex, in vec2 uv);

// Returns whether the texture index has a texture binding associated with it.
bool Dynacoe_SlotHasTexture(in int textureIndex);

// Returns the normal vector from the given texture.
vec3 Dynacoe_SampleNormal(in int textureIndex, in vec2 uv);


```

Because you specify the index as a generic index, the actual number of reachable textures  
quite high. The minimum is 32, but if running opengl 3.0 or higher, its 
somewhere on the order of 1024 simulateous texture bindings. This makes it possible to 
render complex scenes in very few draws, or use the texture interface to access 
and utilize custom data.

Along with regular texture sampling, you can also sample from previously drawn scenes
via the last drawn visual in the Camera passed to AspectMesh::SetFramebufferSource().
You sample it much like a texture.

```{.c}

// Returns whether theres an attached framebuffer texture to sample from 
// If there isn't one, trying to sample the framebuffer 
// results in undefined texel colors.
bool Dynacoe_CanSampleFramebuffer();

// Samples the attached camera's visual as a texture.
vec4 Dynacoe_SampleFramebuffer(in vec2 uv);

```



Lighting
--------

Dynacoe also provides basic lighting capability that integrates with AspectLights.
Keep in mind that the positional inputs need to be in the same world space. This can be tricky 
to wrap your head around, but check out the Shader sample to see an example of how you can do it.
```{.c}

// Standard function that calulates the color of a fragment of a surface 
// given the fragment's world position, normal,
// view direction, and material attributes. This applies all 
// enabled AspectLights and considers their respective types
vec3 Dynacoe_CalculateLightFragment(
    in vec3 position, in vec3 normal, in vec3 viewDir,
    in float diffuseAmount,  in vec3 diffuseColor,
    in float specularAmount, in vec3 specularColor
    );
    
// Calculates the light from a single point-style light.
// Point lights obey the inserse-square law for brightness vs. distance, 
// emulating the behavior of a localized light source in all directions.
// Inputs are the same as CalculateLightFragment() but 
// adds a parameter for the light's position
vec3 Dynacoe_PointLight(
    in vec3 position, in vec3 normal, in vec3 lightPos, in vec3 viewDir,
    in float diffuseAmount,  in vec3 diffuseColor,
    in float specularAmount, in vec3 specularColor
    );

// Calculates the light from a single directional-style light.
// Directional lights cast light in a direction with uniform brightness regardless of 
// surface position. Directional lights emulate very bright and distanced lights,
// i.e. the sky light, or ambient lighting. Inputs follow from Dynacoe_PointLight, 
// but the lighting position is now a lighting direction.
vec3 Dynacoe_DirectionalLight(
    in vec3 position, in vec3 normal, in vec3 lightDir, in vec3 viewDir,
    in float diffuseAmount,  in vec3 diffuseColor,
    in float specularAmount, in vec3 specularColor
    );

```
