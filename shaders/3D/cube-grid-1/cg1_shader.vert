// sb_shader.vert
#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 viewProj;
    float time;
} pc;

layout(location = 0) out float vWave;

// 12 triangles (36 verts) for a unit cube centered at origin
const vec3 cubeVerts[36] = vec3[](
// front +Z
vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5), vec3( 0.5,  0.5,  0.5),
vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5), vec3(-0.5, -0.5,  0.5),
// back -Z
vec3( 0.5, -0.5, -0.5), vec3(-0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5),
vec3(-0.5,  0.5, -0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5, -0.5, -0.5),
// left -X
vec3(-0.5, -0.5, -0.5), vec3(-0.5, -0.5,  0.5), vec3(-0.5,  0.5,  0.5),
vec3(-0.5,  0.5,  0.5), vec3(-0.5,  0.5, -0.5), vec3(-0.5, -0.5, -0.5),
// right +X
vec3( 0.5, -0.5,  0.5), vec3( 0.5, -0.5, -0.5), vec3( 0.5,  0.5, -0.5),
vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5), vec3( 0.5, -0.5,  0.5),
// top +Y
vec3(-0.5,  0.5, -0.5), vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5),
vec3( 0.5,  0.5,  0.5), vec3(-0.5,  0.5,  0.5), vec3(-0.5,  0.5, -0.5),
// bottom -Y
vec3( 0.5, -0.5,  0.5), vec3( 0.5, -0.5, -0.5), vec3(-0.5, -0.5, -0.5),
vec3(-0.5, -0.5, -0.5), vec3(-0.5, -0.5,  0.5), vec3( 0.5, -0.5,  0.5)
);

void main() {
    uint idx = gl_VertexIndex;
    uint inst = gl_InstanceIndex;

    // grid layout
    const int GRID = 16;
    const float SPACING = 1.5;
    float gx = float(int(inst) % GRID) - (GRID-1)/2.0;
    float gz = float(int(inst) / GRID) - (GRID-1)/2.0;

    // beat‐synced wave
    float beat   = pc.time * (175.0/60.0) * 2.0 * 3.14159265;
    float phase  = beat + (gx+gz)*0.5;
    float wave   = sin(phase) * 0.5 + 1.0;
    vWave = wave;

    // local cube vertex
    vec3 pos = cubeVerts[idx] * wave * 0.3;

    // world position = pos + grid offset
    vec4 world = vec4(pos + vec3(gx*SPACING, 0.0, gz*SPACING), 1.0);

    gl_Position = pc.viewProj * pc.model * world;
}
