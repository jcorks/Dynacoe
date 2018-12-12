//
const char * static_es_glsl = R"(//GLSL BEGIN
#version 100
#pragma optimize(on)
#pragma debug(on)
#line 6

attribute highp   vec3 Dynacoe_Position;
attribute mediump vec3 Dynacoe_Normal;
attribute highp   vec2 Dynacoe_UV;
attribute highp   vec4 Dynacoe_UserData;



highp mat4 Dynacoe_ViewTransform;
highp mat4 Dynacoe_ViewNormalTransform;
highp mat4 Dynacoe_ProjectionTransform;


highp vec4    Dynacoe_MaterialAmbient;
highp vec3    Dynacoe_MaterialDiffuse;
highp float   Dynacoe_MaterialDiffuseAmount;
highp vec3    Dynacoe_MaterialSpecular;
highp float   Dynacoe_MaterialSpecularAmount;
highp float   Dynacoe_MaterialShininess; 
highp vec4[8] Dynacoe_MaterialData;

highp mat4 Dynacoe_ModelTransform;
highp mat4 Dynacoe_ModelNormalTransform;



/* TO avoid needing to query offsets, the data will be implicitly laid
   out in memory as so:
   - The first  element will refer to the GUTw followed by the GUTh, x and y accordingly

   Each each other vec4 will belong to a Texture Info packet

   - Texture info is packed as follows in 2 elements:
     . the first element holds whether or not this Texture info refers to a texture that should be used
       If the x value is above .5, this texture info block is valid and should be processed. If
       the x value is below -.5, the TexInfo is invalid or marks the end of textures to be read and texture reading should cease

     . the second element holds the texture base coordinates, x, y, z, w   

   - This continues until the implementation limit is reached, which is hardcoded into the 
     shader.

   - This implementation will hold up to 511 simultaneous texture bindings.

*/
highp vec4[1 + 511*2] _impl_Dynacoe_TexInfo;
highp vec4[1 + 511*2] _impl_Dynacoe_TexInfo2;



/*


    // light struct (8 floats)
    
    vec3  lightPos   -   if point light or spot light, its the position.
                            if a directional light, refers to the directional vector


    float lightIntensity-   light strength multiplier.

*/
highp vec4 _impl_Dynacoe_LightData[1024]; //512



/*


    // light struct (4 floats)
    vec3  lightColor    -   rgb of light

    float lightType    -   
                            [1, 10] - point light
                            [10, 20] - directional light
                            [20, 30] - spotlight
    

*/
highp vec4 _impl_Dynacoe_LightData2[1024]; //512


// Internal interface for standard functions.
#define _BSI_Dynacoe_Texture_GUT_x         (_impl_Dynacoe_TexInfo[0].x)
#define _BSI_Dynacoe_Texture_GUT_y         (_impl_Dynacoe_TexInfo[0].y)
vec2  _BSI_Dynacoe_Texture_pos (in int i) { return _impl_Dynacoe_TexInfo[i+1].xy;}
vec2  _BSI_Dynacoe_Texture_dims(in int i) { return _impl_Dynacoe_TexInfo[i+1].zw;}
float _BSI_Dynacoe_Texture_exists(in int i) { return _impl_Dynacoe_TexInfo2[i].x;}

// textures
uniform sampler2D _BSI_Dynacoe_BaseTexture;
uniform sampler2D _BSI_Dynacoe_FBtexture;
uniform int       _BSI_Dynacoe_hasFBtexture;

// lights
vec3  _BSI_Dynacoe_Light_pos(in int i)       {return _impl_Dynacoe_LightData[i].xyz;}
float _BSI_Dynacoe_Light_intensity(in int i) {return _impl_Dynacoe_LightData[i].w;}

vec3  _BSI_Dynacoe_Light_color(in int i)      {return _impl_Dynacoe_LightData2[i].xyz;}
float _BSI_Dynacoe_Light_type(in int i)      {return _impl_Dynacoe_LightData2[i].w;}



// Returns green if the statement is true and red otherwise
lowp vec4 Dynacoe_ColorAssert(in bool statement) {
   if (statement) {
       return vec4(0, 1, 0, 1);
   }
   return vec4(1, 0, 0, 1);
}

// Returns the color sample at the specified position of the given image slot.
vec4 Dynacoe_SampleColor(in int textureSlot, in vec2 localUV) {
   localUV.x = clamp(localUV.x, 0.f, 1.f);
   localUV.y = clamp(localUV.y, 0.f, 1.f);
   localUV.y = 1-localUV.y; // compensate for reversal
//   int realIndex = textureSlot*2+1+1;
   vec2 realUVs;
   vec2 localPos  = _BSI_Dynacoe_Texture_pos(textureSlot);
   vec2 localDims = _BSI_Dynacoe_Texture_dims(textureSlot);
//   vec2 GUT = _impl_Dynacoe_TexInfo[0].xy;
//   realUVs.x = (    localUV.x*_impl_Dynacoe_TexInfo[realIndex].z + _impl_Dynacoe_TexInfo[realIndex].x) / GUT.x;
//   realUVs.y = (    localUV.y*_impl_Dynacoe_TexInfo[realIndex].w + _impl_Dynacoe_TexInfo[realIndex].y) / GUT.y;
   realUVs.x = (    localUV.x*localDims.x + localPos.x) / _BSI_Dynacoe_Texture_GUT_x;
   realUVs.y = (    localUV.y*localDims.y + localPos.y) / _BSI_Dynacoe_Texture_GUT_y;

   return texture2D(_BSI_Dynacoe_BaseTexture, realUVs);
}




// Returns whether or not textures have been prepared for this renderering session
bool Dynacoe_HasTextureBindings() {
   return (_BSI_Dynacoe_Texture_exists(0) > .5f);
}

// returns whether the given texture slot has valid data
bool Dynacoe_SlotHasTexture(in int slot) {
   return (_BSI_Dynacoe_Texture_exists(slot) > .5f);
}


// returns the shininess from the Shiny Maps if any.
// If no shiny maps are used, the Material Shininess is returned.
float Dynacoe_SampleShininess(in int textureSlot, in vec2 localUV) {
   return Dynacoe_SampleColor(textureSlot, localUV).r*100;
}


// returns the normal from the given texture at the given coordinate
vec3 Dynacoe_SampleNormal(in int textureSlot, in vec2 localUV) {
   vec3 v = Dynacoe_SampleColor(textureSlot, localUV).xyz;
   return v*2 - 1;
}

// returns the sampling of the current framebuffers
vec4 Dynacoe_SampleFramebuffer(in vec2 localUV) {
   return texture2D(_BSI_Dynacoe_FBtexture, localUV);
}

// returns whether theres a sample framebuffer to sample from 
bool Dynacoe_CanSampleFramebuffer() {
   return _BSI_Dynacoe_hasFBtexture != 0;
}





// blinn-phong shading based on implementation from Wikipedia
vec3 _impl_Dynacoe_BFLight(in vec3 pos, in vec3 normal, in vec3 lightDir,
                      in float diffuseAmount,  in vec3 diffuseColor,
                      in float specularAmount, in vec3 specularColor,
                      in float distance, in float shininess) {
    vec3 cOut = vec3(0, 0, 0);

     // diffuse caluclation
    float NdotL = dot( normalize(lightDir), normalize(normal));
    float intensity = max(0.0, NdotL);
    cOut += intensity * diffuseColor * diffuseAmount / distance;
    vec3 viewDir = normalize(-pos);
      // specular 
    if (intensity > 0.0) {
       vec3 H = normalize(lightDir + viewDir);
       float NdotH = max(dot(H, normal), 0);
       intensity = pow(NdotH, shininess);
       cOut += intensity * specularColor * specularAmount / distance;
    }
    
    return cOut;
}

// Direction shading
vec3 Dynacoe_DirectionalLight(in vec3 pos, in vec3 normal, in vec3 lightPos,
                        in float diffuseAmount, in vec3 diffuseColor,
                        in float specularAmount, in vec3 specularColor, in float shininess) {
//    vec3 lightDir = vec3(.0f, 1.f, .4f);
    vec3 lightDir = -lightPos;
//    return lightPos.xyz;
    return _impl_Dynacoe_BFLight(pos, normal, normalize(lightDir), diffuseAmount, diffuseColor, specularAmount, specularColor, 1.f, shininess);
}




// 
vec3 Dynacoe_PointLight(in vec3 pos, in vec3 normal, in vec3 lightPos,
                        in float diffuseAmount, in vec3 diffuseColor,
                        in float specularAmount, in vec3 specularColor, in float shininess) {
    vec3 lightDir = lightPos - pos;
      // calculate distance for inverse-square law (point)
    float distance = max(length(pos - lightDir), 1.f);
    distance = distance*distance;
    
    return _impl_Dynacoe_BFLight(pos, normal, normalize(lightDir), diffuseAmount, diffuseColor, specularAmount, specularColor, distance, shininess);
}


// TODO: Spot Light 
// Idea* multiplier to intensity that is 
// the dot product of the light direction from the light and the surface position from 
// the light (perhaps further multiplied by a focus amount. Focus can piggyback on the remaining space 
// in the type float. It would be detrimental to add another vec4.)
//
// dot(lightDirFromLight, surfaceDirFromLight + focus*lightDirFromLight) // <-- well not exactly,but thats the idear



// Returns the interpolated lambertian and specular amount at the
// current fragment given the vertex location and interpolated normal vector.
// An additive blending technique is used for interpolating multiple lights.
// The openGL implementation uses phong-based shading for lighting 




// transformed point (*mv, not projected)
// normal 
vec3 Dynacoe_CalculateLightFragment(in vec3 position, in vec3 normal, in float diffuseAmount, in vec3 diffuseMaterial, in float specularAmount, in vec3 specularMaterial, in float shininess) {

   int index = 0;
   vec3 srcLightPos;
   vec3 lcolor;
   vec3  color = vec3(0, 0, 0);
   float type = -1;
   float intensity;
// L -> Light position
// E -> Emission direciton
// R -> Reflectance direction

   type = _BSI_Dynacoe_Light_type(index);
   while(type > 0 && index < 128) {   

// if directional light, do not mult by transform.
       intensity   = _BSI_Dynacoe_Light_intensity(index);
       lcolor      = _BSI_Dynacoe_Light_color(index);
       if (type >= 0 && type <= .1) { // point light
           srcLightPos = (Dynacoe_ViewTransform * vec4(_BSI_Dynacoe_Light_pos(index), 1.f)).xyz;
           color += intensity*lcolor*Dynacoe_PointLight(position, normal, srcLightPos, diffuseAmount, diffuseMaterial, specularAmount, specularMaterial, shininess);
       } else if (type >= .1 && type <= .2) { // directional
           color += intensity*lcolor*Dynacoe_DirectionalLight(position, normal, _BSI_Dynacoe_Light_pos(index), diffuseAmount, diffuseMaterial, specularAmount, specularMaterial, shininess);
       }

       index++;
       type = _BSI_Dynacoe_Light_type(index);
   }
   return color;
}

)";
