#version 450

layout(push_constant) uniform PushConstants {
    vec2 resolution;
    float time;
} pc;

layout(location = 0) out vec4 outColor;

#define MAXITERS 300.0
#define LENFACTOR 0.25
#define NDELTA 0.001

#define NDELTAX vec3(NDELTA,0,0)
#define NDELTAY vec3(0,NDELTA,0)
#define NDELTAZ vec3(0,0,NDELTA)

float box(vec3 p, vec3 c, vec3 d) {
    vec3 diff = abs(p - c) - d;
    return max(diff.x, max(diff.y, diff.z));
}

// lighting dirs/colors
const vec3 rDir = normalize(vec3(-3.0,  4.0, -2.0)), rCol = vec3(1.0, 0.6, 0.4);
const vec3 gDir = normalize(vec3( 4.0, -3.0,  0.0)), gCol = vec3(0.7, 1.0, 0.8);
const vec3 bDir = normalize(vec3( 2.0,  3.0, -4.0)), bCol = vec3(0.3, 0.7, 1.0);

mat3 rotationMatrix(vec3 axis, float angle) {
    float s = sin(angle), c = cos(angle), oc = 1.0 - c;
    return mat3(
    oc*axis.x*axis.x + c,        oc*axis.x*axis.y - axis.z*s, oc*axis.z*axis.x + axis.y*s,
    oc*axis.x*axis.y + axis.z*s, oc*axis.y*axis.y + c,        oc*axis.y*axis.z - axis.x*s,
    oc*axis.z*axis.x - axis.y*s, oc*axis.y*axis.z + axis.x*s, oc*axis.z*axis.z + c
    );
}

const float pi = 3.1415926536;
mat2 rot2(float t) {
    float s = sin(t), c = cos(t);
    return mat2(c, s, -s, c);
}

vec3 axisDir() {
    return vec3(cos(pc.time * 0.3), 0.0, sin(pc.time * 0.3));
}
vec3 rotSpace(vec3 p) {
    float ang = pi * pow(smoothstep(100.0,2.0,dot(p,p)),5.0);
    return (ang>0.0) ? p * rotationMatrix(axisDir(), ang) : p;
}

float sceneSDF(vec3 p) {
    p = rotSpace(p);
    float l = pc.time * 0.2 - 0.2;
    l = max(0.0, min(pow(l,6.0),1000.0));
    float d1 = box(p, vec3(0), vec3(0.7,0.1,l));
    float d2 = box(p, vec3(0), vec3(0.1,l,0.7));
    float d3 = box(p, vec3(0), vec3(l,0.7,0.1));
    float d4 = box(p, vec3(0), vec3(1.0));
    return min(min(d1,d2), min(d3,d4));
}

vec3 sceneNormal(vec3 p) {
    return normalize(vec3(
    sceneSDF(p + NDELTAX) - sceneSDF(p - NDELTAX),
    sceneSDF(p + NDELTAY) - sceneSDF(p - NDELTAY),
    sceneSDF(p + NDELTAZ) - sceneSDF(p - NDELTAZ)
    ));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    // --- ray setup ---
    vec2 uv = (fragCoord - 0.5 * pc.resolution) / pc.resolution.y;
    vec3 ray = normalize(vec3(uv, 1.0));
    ray.yz *= rot2(-0.12);
    ray.xz *= rot2(-0.78539816);
    vec3 cam = vec3(10.0, 2.0, -10.0);

    // --- raymarch ---
    vec3 pos = cam;
    float t = 0.0;
    for (; t < MAXITERS; ++t) {
        float dist = sceneSDF(pos);
        if (dist < NDELTA) break;
        pos += ray * dist * LENFACTOR;
    }

    // --- shading ---
    if (t >= MAXITERS) {
        fragColor = vec4(0.0); // miss = black
        return;
    }

    vec3 n = sceneNormal(pos);
    float fade = 1.0 - pow(t / MAXITERS, 2.0);

    // per‐face base colour (COLOURS)
    vec3 baseCol;
    vec3 p2 = rotSpace(pos);
    if (abs(p2.x) > 1.001)      baseCol = vec3(0.6, 0.0, 0.8);  // purple
    else if (abs(p2.y) > 1.001) baseCol = vec3(0.8, 0.2, 0.4);  // crimson
    else                         baseCol = vec3(0.4, 0.0, 0.6);  // dark violet

    // lighting contribution
    float lr = abs(dot(rDir, n));
    float lg = pow(dot(gDir, n), 5.0);
    float lb = abs(dot(bDir, n));
    vec3 light = rCol * lr + gCol * lg + bCol * lb;

    // slow pulsation
    float pulse = 0.6 + 0.4 * sin(pc.time * 0.5);

    fragColor = vec4(baseCol * light * fade * pulse, 1.0);
}

void main() {
    mainImage(outColor, gl_FragCoord.xy);
}
