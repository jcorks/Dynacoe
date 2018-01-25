// Returns green if the statement is true and red otherwise
"vec4 Dynacoe_ColorAssert(in bool statement) {\n"
"   if (statement) {\n"
"       return vec4(0, 1, 0, 1);\n"
"   }\n"
"   return vec4(1, 0, 0, 1);\n"
"}\n"

// Returns the color sample at the specified position of the given image slot.
"vec4 Dynacoe_SampleColor(in int textureSlot, in vec2 localUV) {\n"
"   localUV.x = clamp(localUV.x, 0.f, 1.f);"
"   localUV.y = clamp(localUV.y, 0.f, 1.f);"
"   localUV.y = 1-localUV.y;\n" // compensate for reversal
//"   int realIndex = textureSlot*2+1+1;"
"   vec2 realUVs;\n"
"   vec2 localPos  = _BSI_Dynacoe_Texture_pos(textureSlot);\n"
"   vec2 localDims = _BSI_Dynacoe_Texture_dims(textureSlot);\n"
//"   vec2 GUT = _impl_Dynacoe_TexInfo[0].xy;\n"
//"   realUVs.x = (    localUV.x*_impl_Dynacoe_TexInfo[realIndex].z + _impl_Dynacoe_TexInfo[realIndex].x) / GUT.x;\n"
//"   realUVs.y = (    localUV.y*_impl_Dynacoe_TexInfo[realIndex].w + _impl_Dynacoe_TexInfo[realIndex].y) / GUT.y;\n"
"   realUVs.x = (    localUV.x*localDims.x + localPos.x) / _BSI_Dynacoe_Texture_GUT_x;\n"
"   realUVs.y = (    localUV.y*localDims.y + localPos.y) / _BSI_Dynacoe_Texture_GUT_y;\n"

"   return texture2D(_BSI_Dynacoe_BaseTexture, realUVs);\n"
"}\n"




// Returns whether or not textures have been prepared for this renderering session
"bool Dynacoe_HasTextureBindings() {\n"
"   return (_BSI_Dynacoe_Texture_exists(0) > .5f);\n"
"}\n"

// returns whether the given texture slot has valid data
"bool Dynacoe_SlotHasTexture(in int slot) {\n"
"   return (_BSI_Dynacoe_Texture_exists(slot) > .5f);\n"
"}\n"


// returns the shininess from the Shiny Maps if any.
// If no shiny maps are used, the Material Shininess is returned.
"float Dynacoe_SampleShininess(in int textureSlot, in vec2 localUV) {\n"
"   return Dynacoe_SampleColor(textureSlot, localUV).r*100;\n"
"}\n"


// returns the normal from the given texture at the given coordinate
"vec3 Dynacoe_SampleNormal(in int textureSlot, in vec2 localUV) {\n"
"   vec3 v = Dynacoe_SampleColor(textureSlot, localUV).xyz;"
"   return v*2 - 1;\n"
"}\n"

// returns the sampling of the current framebuffers
"vec4 Dynacoe_SampleFramebuffer(in vec2 localUV) {\n"
"   return texture2D(_BSI_Dynacoe_FBtexture, localUV);\n"
"}\n"

// returns whether theres a sample framebuffer to sample from 
"bool Dynacoe_CanSampleFramebuffer() {\n"
"   return _BSI_Dynacoe_hasFBtexture != 0;\n"
"}\n"





// blinn-phong shading based on implementation from Wikipedia
"vec3 _impl_Dynacoe_BFLight(in vec3 pos, in vec3 normal, in vec3 lightDir,\n"
"                      in float diffuseAmount,  in vec3 diffuseColor,\n"
"                      in float specularAmount, in vec3 specularColor,\n"
"                      in float distance, in float shininess) {\n"
"    vec3 cOut = vec3(0, 0, 0);\n"

     // diffuse caluclation
"    float NdotL = dot( normalize(lightDir), normalize(normal));\n"
"    float intensity = max(0.0, NdotL);\n"
"    cOut += intensity * diffuseColor * diffuseAmount / distance;\n"
"    vec3 viewDir = normalize(-pos);\n"
      // specular 
"    if (intensity > 0.0) {\n"
"       vec3 H = normalize(lightDir + viewDir);\n"
"       float NdotH = max(dot(H, normal), 0);\n"
"       intensity = pow(NdotH, shininess);\n"
"       cOut += intensity * specularColor * specularAmount / distance;\n"
"    }\n"
    
"    return cOut;\n"
"}\n"

// Direction shading
"vec3 Dynacoe_DirectionalLight(in vec3 pos, in vec3 normal, in vec3 lightPos,\n"
"                        in float diffuseAmount, in vec3 diffuseColor,\n"
"                        in float specularAmount, in vec3 specularColor, in float shininess) {\n"
//"    vec3 lightDir = vec3(.0f, 1.f, .4f);\n"
"    vec3 lightDir = -lightPos;\n"
//"    return lightPos.xyz;\n"
"    return _impl_Dynacoe_BFLight(pos, normal, normalize(lightDir), diffuseAmount, diffuseColor, specularAmount, specularColor, 1.f, shininess);\n"
"}\n"




// 
"vec3 Dynacoe_PointLight(in vec3 pos, in vec3 normal, in vec3 lightPos,\n"
"                        in float diffuseAmount, in vec3 diffuseColor,\n"
"                        in float specularAmount, in vec3 specularColor, in float shininess) {\n"
"    vec3 lightDir = lightPos - pos;\n"
      // calculate distance for inverse-square law (point)
"    float distance = max(length(pos - lightDir), 1.f);\n"
"    distance = distance*distance;\n"
    
"    return _impl_Dynacoe_BFLight(pos, normal, normalize(lightDir), diffuseAmount, diffuseColor, specularAmount, specularColor, distance, shininess);\n"
"}\n"


// TODO: Spot Light 
// Idea* multiplier to intensity that is 
// the dot product of the light direction from the light and the surface position from 
// the light (perhaps further multiplied by a "focus" amount. Focus can piggyback on the remaining space 
// in the type float. It would be detrimental to add another vec4.)
//
// dot(lightDirFromLight, surfaceDirFromLight + focus*lightDirFromLight) // <-- well not exactly,but thats the idear



// Returns the interpolated lambertian and specular amount at the
// current fragment given the vertex location and interpolated normal vector.
// An additive blending technique is used for interpolating multiple lights.
// The openGL implementation uses phong-based shading for lighting 




// transformed point (*mv, not projected)
// normal 
"vec3 Dynacoe_CalculateLightFragment(in vec3 position, in vec3 normal, in float diffuseAmount, in vec3 diffuseMaterial, in float specularAmount, in vec3 specularMaterial, in float shininess) {\n"

"   int index = 0;\n"
"   vec3 srcLightPos;\n"
"   vec3 lcolor;\n"
"   vec3  color = vec3(0, 0, 0);\n"
"   float type = -1;\n"
"   float intensity;\n"
// L -> Light position
// E -> Emission direciton
// R -> Reflectance direction

"   type = _BSI_Dynacoe_Light_type(index);\n"
"   while(type > 0 && index < 128) {\n"   

// if directional light, do not mult by transform.
"       intensity   = _BSI_Dynacoe_Light_intensity(index);\n"
"       lcolor      = _BSI_Dynacoe_Light_color(index);\n"
"       if (type >= 0 && type <= .1) {\n" // point light
"           srcLightPos = (Dynacoe_ViewTransform * vec4(_BSI_Dynacoe_Light_pos(index), 1.f)).xyz;\n"
"           color += intensity*lcolor*Dynacoe_PointLight(position, normal, srcLightPos, diffuseAmount, diffuseMaterial, specularAmount, specularMaterial, shininess);\n"
"       } else if (type >= .1 && type <= .2) {\n" // directional
"           color += intensity*lcolor*Dynacoe_DirectionalLight(position, normal, _BSI_Dynacoe_Light_pos(index), diffuseAmount, diffuseMaterial, specularAmount, specularMaterial, shininess);\n"
"       }\n"

"       index++;\n"
"       type = _BSI_Dynacoe_Light_type(index);\n"
"   }\n"
"   return color;\n"
"}\n"
"#line 0\n";
