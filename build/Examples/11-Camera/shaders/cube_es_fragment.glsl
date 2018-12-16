// Fragment shader
//
// Not as flashy as the previous example, but definitely more colorful 
// Colors based on a field position.
varying highp vec3   pos;


void main() {
    lowp vec4 finalColor;
    
    finalColor.x = sin(pos.x*10.0)*.5+1.0;
    finalColor.y = sin(pos.y*10.0)*.5+1.0;
    finalColor.z = sin(pos.z*10.0)*.5+1.0;
    
    finalColor = finalColor*finalColor.z;

    finalColor.a = 1.0;
    gl_FragColor = finalColor;
}
