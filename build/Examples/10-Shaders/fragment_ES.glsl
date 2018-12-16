// Fragment shader

varying highp vec2   UV;
varying highp vec3   pos;
varying highp vec3   normal;


// Diffuse color (wide-reflected light)
const lowp  vec3  surface_diffuse_color   = vec3(.3, .3, .4);
const highp float surface_diffuse_amount  = 3.; 

// Specular color (face-on reflected light)
const lowp  vec3  surface_specular_color  = vec3(.4, .3, .5);
const highp float surface_specular_amount = .5;

const highp float surface_shininess = 6.;

void main() {
    lowp vec4 finalColor;
    
    // Current fragment position
    highp vec3 surfacePos    = (Dynacoe_ModelTransform * vec4(pos, 1)).xyz;
    
    // Normal from the current position
    highp vec3 surfaceNormal = (Dynacoe_ModelNormalTransform * vec4(Dynacoe_SampleNormal(1, UV) +normal, 0.0)).xyz;

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
    
    finalColor.a = 1.0;
    gl_FragColor = finalColor;
}
