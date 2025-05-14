// sb_shader.frag
#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 viewProj;
    float time;
} pc;

layout(location = 0) in  float vWave;
layout(location = 0) out vec4 outColor;

// HSV→RGB
vec3 hsv2rgb(vec3 c){
    vec4 K = vec4(1.0,2.0/3.0,1.0/3.0,3.0);
    vec3 p = abs(fract(c.xxx+K.xyz)*6.0-K.www);
    return c.z * mix(K.xxx, clamp(p-K.xxx,0.,1.), c.y);
}

void main(){
    // color shifts with wave & time
    float hue = fract(vWave*0.2 + pc.time*0.05);
    float sat = 0.7 + 0.3 * sin(pc.time*2.0 + vWave*3.0);
    float val = 0.6 + 0.4 * cos(pc.time*1.5 - vWave*2.0);
    outColor = vec4(hsv2rgb(vec3(hue,sat,val)), 1.0);
}
