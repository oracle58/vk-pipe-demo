// sb_shader.vert
#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 viewProj;
    float time;
} pc;

layout(location = 0) out float vWave;
layout(location = 1) out vec3 fsLocalPos;

// cube mesh: 12 tris = 36 verts
const vec3 cubeVerts[36] = vec3[](
// front +Z
vec3(-0.5,-0.5, 0.5), vec3( 0.5,-0.5, 0.5), vec3( 0.5, 0.5, 0.5),
vec3( 0.5, 0.5, 0.5), vec3(-0.5, 0.5, 0.5), vec3(-0.5,-0.5, 0.5),
// back -Z
vec3( 0.5,-0.5,-0.5), vec3(-0.5,-0.5,-0.5), vec3(-0.5, 0.5,-0.5),
vec3(-0.5, 0.5,-0.5), vec3( 0.5, 0.5,-0.5), vec3( 0.5,-0.5,-0.5),
// left -X
vec3(-0.5,-0.5,-0.5), vec3(-0.5,-0.5, 0.5), vec3(-0.5, 0.5, 0.5),
vec3(-0.5, 0.5, 0.5), vec3(-0.5, 0.5,-0.5), vec3(-0.5,-0.5,-0.5),
// right +X
vec3( 0.5,-0.5, 0.5), vec3( 0.5,-0.5,-0.5), vec3( 0.5, 0.5,-0.5),
vec3( 0.5, 0.5,-0.5), vec3( 0.5, 0.5, 0.5), vec3( 0.5,-0.5, 0.5),
// top +Y
vec3(-0.5, 0.5,-0.5), vec3( 0.5, 0.5,-0.5), vec3( 0.5, 0.5, 0.5),
vec3( 0.5, 0.5, 0.5), vec3(-0.5, 0.5, 0.5), vec3(-0.5, 0.5,-0.5),
// bottom -Y
vec3( 0.5,-0.5, 0.5), vec3( 0.5,-0.5,-0.5), vec3(-0.5,-0.5,-0.5),
vec3(-0.5,-0.5,-0.5), vec3(-0.5,-0.5, 0.5), vec3( 0.5,-0.5, 0.5)
);

void main() {
    uint idx  = gl_VertexIndex;
    uint inst = gl_InstanceIndex;

    const int   GRID      = 16;
    const float SPACING   = 0.55;
    const float SCALE     = 0.3;
    const float AMPLITUDE = 1.0;
    const float PI        = 3.14159265;

    float gx = float(int(inst) % GRID) - (GRID - 1) * 0.5;
    float gz = float(int(inst) / GRID)     - (GRID - 1) * 0.5;

    float beat  = pc.time * (175.0 / 60.0) * PI;
    float phase = beat + gx;
    float wave  = sin(phase);
    float stick = max(wave, 0.0);
    vWave       = stick;

    vec3 local = cubeVerts[idx] * SCALE;
    fsLocalPos  = local;

    float yOff     = stick * AMPLITUDE;
    vec3 worldPos = local + vec3(gx * SPACING, yOff, gz * SPACING);

    gl_Position = pc.viewProj * pc.model * vec4(worldPos, 1.0);
}
