#version 450

layout(push_constant) uniform PushConstants {
    float time;
} pc;

layout(location = 1) flat in float vInstanceId;
layout(location = 0) out vec4 outColor;

// simple HSV→RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    // spread hues evenly per-instance and spin over time
    float hue     = fract(vInstanceId / 20.0 + pc.time * 0.1);
    float sat     = 0.6 + 0.4 * sin(pc.time * 1.2 + vInstanceId);
    float val     = 0.7 + 0.3 * cos(pc.time * 0.9 - vInstanceId);

    vec3 rgb      = hsv2rgb(vec3(hue, sat, val));
    outColor      = vec4(rgb, 1.0);
}
