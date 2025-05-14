// sb_shader.frag
#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 viewProj;
    float time;
} pc;

layout(location = 0) in  vec2 vUV;
layout(location = 0) out vec4 outColor;

const float BPM = 175.0;
const float PI  = 3.14159265359;

// rotate2D
mat2 Rot(float a) {
    float s = sin(a), c = cos(a);
    return mat2(c, -s, s, c);
}

// SDF of an axis-aligned box
float sdBox(vec3 p, vec3 b) {
    vec3 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}

// scene SDF: grid of pulsating, twisted cubes
float map(vec3 p) {
    float spacing = 2.0;
    vec3 q = mod(p, spacing) - spacing * 0.5;

    // twist over time and radius
    float twist  = sin(pc.time * 0.5 + length(q.xy)) * 0.5;
    q.xy = Rot(twist) * q.xy;

    // beat-sync pulse
    float beat  = pc.time * BPM / 60.0;
    float pulse = sin(beat * 2.0 * PI + q.x * 5.0) * 0.3 + 0.7;

    vec3 boxSize = vec3(0.4) * pulse;
    return sdBox(q, boxSize);
}

// raymarch the SDF
float raymarch(vec3 ro, vec3 rd) {
    float t = 0.0;
    for (int i = 0; i < 100; i++) {
        vec3 p = ro + rd * t;
        float d = map(p);
        t += d;
        if (d < 0.001 || t > 20.0) break;
    }
    return t;
}

// simple HSV→RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0,2.0/3.0,1.0/3.0,3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    // NDC → ray
    vec2 uv = vUV * 2.0 - 1.0;
    vec3 ro = vec3(0.0, 0.0, 5.0);
    vec3 rd = normalize(vec3(uv, -1.0));

    // rotate camera slowly
    float camAng = pc.time * 0.2;
    ro.xz = Rot(camAng) * ro.xz;
    rd.xz = Rot(camAng) * rd.xz;

    float t = raymarch(ro, rd);

    vec3 col = vec3(0.0);
    if (t < 20.0) {
        vec3 pos = ro + rd * t;
        float hue = fract(pc.time * 0.05 + pos.z * 0.1);
        vec3 rgb = hsv2rgb(vec3(hue, 1.0, 1.0));
        float fog = exp(-t * 0.2);
        col = rgb * fog;
    }

    outColor = vec4(col, 1.0);
}
