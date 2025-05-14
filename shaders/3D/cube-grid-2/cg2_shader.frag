// sb_shader.frag
#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 viewProj;
    float time;
} pc;

layout(location = 0) in  float vWave;
layout(location = 1) in  vec3  fsLocalPos;
layout(location = 0) out vec4 outColor;

// dark purple → crimson
vec3 palette(float t) {
    vec3 darkPurple = vec3(0.15, 0.00, 0.20);
    vec3 crimson    = vec3(0.60, 0.00, 0.20);
    return mix(darkPurple, crimson, t);
}

// compute outline factor
float outlineFactor(vec3 p) {
    const float H = 0.3;
    float dx = min(abs(p.x - H), abs(p.x + H));
    float dy = min(abs(p.y - H), abs(p.y + H));
    float dz = min(abs(p.z - H), abs(p.z + H));
    float d  = min(min(dx, dy), dz);
    return smoothstep(0.04, 0.0, d);
}

void main() {
    // use wave value for gradient
    vec3 baseCol = palette(vWave);
    float ol     = outlineFactor(fsLocalPos);
    vec3 col     = mix(baseCol, vec3(1.0), ol);
    outColor     = vec4(col, 1.0);
}
