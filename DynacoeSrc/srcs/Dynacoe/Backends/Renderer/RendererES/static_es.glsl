//
const char * static_es_glsl = R"(//GLSL BEGIN
#version 100
#pragma optimize(off)
#pragma debug(on)
#line 7






uniform highp mat4 Dynacoe_ViewTransform;
uniform highp mat4 Dynacoe_ViewNormalTransform;
uniform highp mat4 Dynacoe_ProjectionTransform;
// vec ct: 12/256





uniform highp vec4    _impl_Dynacoe_MaterialAmbient;
#define Dynacoe_MaterialAmbient       (_impl_Dynacoe_MaterialAmbient.xyz)
#define Dynacoe_MaterialShininess     (_impl_Dynacoe_MaterialAmbient.w)


uniform highp vec4    _impl_Dynacoe_MaterialDiffuse; // xyz
#define Dynacoe_MaterialDiffuse       (_impl_Dynacoe_MaterialDiffuse.xyz)
#define Dynacoe_MaterialDiffuseAmount (_impl_Dynacoe_MaterialDiffuse.w)


uniform highp vec4    _impl_Dynacoe_MaterialSpecular;
#define Dynacoe_MaterialSpecular       (_impl_Dynacoe_MaterialSpecular.xyz)
#define Dynacoe_MaterialSpecularAmount (_impl_Dynacoe_MaterialSpecular.w)


uniform highp vec4[8] Dynacoe_MaterialData;

uniform highp mat4 Dynacoe_ModelTransform;
uniform highp mat4 Dynacoe_ModelNormalTransform;

// vec ct: 31/256

// raw texture references
uniform sampler2D fragTex_slots[15];
// vec ct: 63/256





/*
    Since source geometry only refers to textures by a slot and UVs,
    texture slot info needs to bemaintained as follows (indexed by slot)
    
    atlasX,
    atlasY,
    atlasW, // in atlas coords
    atlasH, // in atlas coords

    samplerHandle (float)
        - if no texture, should be negative
        

*/
uniform highp vec4  _impl_Dynacoe_TexInfo_coords[32];
uniform highp float _impl_Dynacoe_TexInfo_handle[32];

// vec ct: 127/256




/*
    // light struct (8 floats)
    
    vec3  lightPos   -   if point light or spot light, its the position.
                            if a directional light, refers to the directional vector


    float lightIntensity-   light strength multiplier.

*/
uniform highp vec4 _impl_Dynacoe_LightData[16]; //512



/*


    // light struct (4 floats)
    vec3  lightColor    -   rgb of light

    float lightType    -   
                            [1, 10] - point light
                            [10, 20] - directional light
                            [20, 30] - spotlight
    

*/
uniform highp vec4 _impl_Dynacoe_LightData2[16]; //512


// vec ct: 255/256










lowp vec4 texture2D3(highp vec3 tex) {
   int index = int(tex.z);
   if (index == 0) return texture2D(fragTex_slots[0], tex.xy);
   if (index == 1) return texture2D(fragTex_slots[1], tex.xy);
   if (index == 2) return texture2D(fragTex_slots[2], tex.xy);
   if (index == 3) return texture2D(fragTex_slots[3], tex.xy);
   if (index == 4) return texture2D(fragTex_slots[4], tex.xy);
   if (index == 5) return texture2D(fragTex_slots[5], tex.xy);
   if (index == 6) return texture2D(fragTex_slots[6], tex.xy);
   if (index == 7) return texture2D(fragTex_slots[7], tex.xy);
   if (index == 8) return texture2D(fragTex_slots[8], tex.xy);
   if (index == 9) return texture2D(fragTex_slots[9], tex.xy);
   if (index == 10) return texture2D(fragTex_slots[10], tex.xy);
   if (index == 11) return texture2D(fragTex_slots[11], tex.xy);
   if (index == 12) return texture2D(fragTex_slots[12], tex.xy);
   if (index == 13) return texture2D(fragTex_slots[13], tex.xy);
   if (index == 14) return texture2D(fragTex_slots[14], tex.xy);
}



// textures
uniform sampler2D   _BSI_Dynacoe_FBtexture;
uniform highp float _BSI_Dynacoe_hasFBtexture;



// lights
highp vec3  _BSI_Dynacoe_Light_pos(in int i)       {return _impl_Dynacoe_LightData[i].xyz;}
highp float _BSI_Dynacoe_Light_intensity(in int i) {return _impl_Dynacoe_LightData[i].w;}

highp vec3  _BSI_Dynacoe_Light_color(in int i)      {return _impl_Dynacoe_LightData2[i].xyz;}
highp float _BSI_Dynacoe_Light_type(in int i)      {return _impl_Dynacoe_LightData2[i].w;}



// Returns green if the statement is true and red otherwise
lowp vec4 Dynacoe_ColorAssert(in bool statement) {
    if (statement) {
        return vec4(0.0, 1.0, 0.0, 1.0);
    }
    return vec4(1.0, 0.0, 0.0, 1.0);
}

// Returns the color sample at the specified position of the given image slot.
lowp vec4 Dynacoe_SampleColor(in int textureSlot, in highp vec2 localUV) {
    highp vec3 texIn;
    highp vec4 coordBase = _impl_Dynacoe_TexInfo_coords[textureSlot];


    texIn.x = (localUV.x * coordBase.z) + coordBase.x;
    texIn.y = (localUV.y * coordBase.w) + coordBase.y;
    texIn.z = _impl_Dynacoe_TexInfo_handle[textureSlot];
    return texture2D3(texIn);
}





// returns whether the given texture slot has valid data
bool Dynacoe_SlotHasTexture(in int slot) {
   return (_impl_Dynacoe_TexInfo_handle[slot] >= 0.0);
}


// returns the shininess from the Shiny Maps if any.
// If no shiny maps are used, the Material Shininess is returned.
mediump float Dynacoe_SampleShininess(in int textureSlot, in highp vec2 localUV) {
   return Dynacoe_SampleColor(textureSlot, localUV).r*100.0;
}


// returns the normal from the given texture at the given coordinate
lowp vec3 Dynacoe_SampleNormal(in int textureSlot, in highp vec2 localUV) {
   lowp vec3 v = Dynacoe_SampleColor(textureSlot, localUV).xyz;
   return v*2.0 - 1.0;
}

// returns the sampling of the current framebuffers
lowp vec4 Dynacoe_SampleFramebuffer(in highp vec2 localUV) {
   return texture2D(_BSI_Dynacoe_FBtexture, localUV);
}

// returns whether theres a sample framebuffer to sample from 
bool Dynacoe_CanSampleFramebuffer() {
   return _BSI_Dynacoe_hasFBtexture != 0.0;
}





// blinn-phong shading based on implementation from Wikipedia
vec3 _impl_Dynacoe_BFLight(in highp vec3 pos, in highp vec3 normal, in highp vec3 lightDir,
                      in highp float diffuseAmount,  in lowp vec3 diffuseColor,
                      in highp float specularAmount, in lowp vec3 specularColor,
                      in highp float distance, in highp float shininess) {
    lowp vec3 cOut = vec3(0, 0, 0);

     // diffuse caluclation
    highp float NdotL = dot( normalize(lightDir), normalize(normal));
    highp float intensity = max(0.0, NdotL);
    cOut += intensity * diffuseColor * diffuseAmount / distance;
    highp vec3 viewDir = normalize(-pos);
    
    // specular highlight
    if (intensity > 0.0) {
       highp vec3 H = normalize(lightDir + viewDir);
       highp float NdotH = max(dot(H, normal), 0.0);
       intensity = pow(NdotH, shininess);
       cOut += intensity * specularColor * specularAmount / distance;
    }
    
    return cOut;
}

// Direction shading
vec3 Dynacoe_DirectionalLight(in highp vec3 pos, in highp vec3 normal, in highp vec3 lightPos,
                        in highp float diffuseAmount,  in lowp vec3 diffuseColor,
                        in highp float specularAmount, in lowp vec3 specularColor, in highp float shininess) {
    highp vec3 lightDir = -lightPos;
    return _impl_Dynacoe_BFLight(pos, normal, normalize(lightDir), diffuseAmount, diffuseColor, specularAmount, specularColor, 1.0, shininess);
}




// 
vec3 Dynacoe_PointLight(in highp vec3 pos, in highp vec3 normal, in lowp vec3 lightPos,
                        in highp float diffuseAmount, in lowp vec3 diffuseColor,
                        in highp float specularAmount, in lowp vec3 specularColor, in highp float shininess) {
    highp vec3 lightDir = lightPos - pos;
      // calculate distance for inverse-square law (point)
    highp float distance = max(length(pos - lightDir), 1.0);
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
vec3 Dynacoe_CalculateLightFragment(in highp vec3 position, in highp vec3 normal, in highp float diffuseAmount, in lowp vec3 diffuseMaterial, in highp float specularAmount, in lowp vec3 specularMaterial, in highp float shininess) {

    int index = 0;
    highp vec3 srcLightPos;
    lowp vec3 lcolor;
    lowp vec3  color = vec3(0, 0, 0);
    highp float type = -1.0;
    highp float intensity;
    // L -> Light position
    // E -> Emission direciton
    // R -> Reflectance direction

    type = _BSI_Dynacoe_Light_type(index);

    while((type >= 0.0) && (int(index) < 32)) {   
// if directional light, do not mult by transform.
        intensity   = _BSI_Dynacoe_Light_intensity(index);
        lcolor      = _BSI_Dynacoe_Light_color(index);


        if (type >= 0.0 && type <= .1) { // point light
            srcLightPos = (Dynacoe_ViewTransform * vec4(_BSI_Dynacoe_Light_pos(index), 1.0)).xyz;
            color += intensity*lcolor*Dynacoe_PointLight(position, normal, srcLightPos, diffuseAmount, diffuseMaterial, specularAmount, specularMaterial, shininess);
        } else if (type >= .1 && type <= .2) { // directional
            color += intensity*lcolor*Dynacoe_DirectionalLight(position, normal, _BSI_Dynacoe_Light_pos(index), diffuseAmount, diffuseMaterial, specularAmount, specularMaterial, shininess);
        } 
        index++;
        type = _BSI_Dynacoe_Light_type(index);
    }
    return color;
}
#line 1 1
)";
