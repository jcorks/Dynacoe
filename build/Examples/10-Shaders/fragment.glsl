varying vec2   UV;
varying vec3   pos;
varying vec3   normal;


// Diffuse color (wide-reflected light)
const vec3  surface_diffuse_color   = vec3(.3, .3, .4);
const float surface_diffuse_amount  = 3.f; 

// Specular color (face-on reflected light)
const vec3  surface_specular_color  = vec3(.4, .3, .5);
const float surface_specular_amount = .5f;

const float surface_shininess = 6.f;

void main() {
    vec4 finalColor;
    
    // Current fragment position
    vec3 surfacePos    = (Dynacoe_ModelTransform * vec4(pos, 1)).xyz;
    
    // Normal from the current position
    vec3 surfaceNormal = (Dynacoe_ModelNormalTransform * vec4(Dynacoe_SampleNormal(1, UV) +normal, 0.f)).xyz;

    // Direction from the surface pos to the viewer
    //vec3 viewDirection = (Dynacoe_ViewTransform * vec4(-pos, 1.f)).xyz;
    
    finalColor.xyz = Dynacoe_SampleColor(0, UV).xyz * Dynacoe_CalculateLightFragment(
        surfacePos,
        normalize(surfaceNormal),
        //normalize(viewDirection),
 
        surface_diffuse_amount,  surface_diffuse_color,
        surface_specular_amount, surface_specular_color,


        Dynacoe_MaterialShininess


    );
    
    finalColor.a = 1.0f;
    gl_FragColor = finalColor;
}
