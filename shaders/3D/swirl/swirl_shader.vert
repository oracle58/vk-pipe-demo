#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    mat4 viewProj;
    float time;
} pc;

layout(location = 0) out vec2 vUV;

void main() {
    // full-screen triangle
    vec2 pos = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vUV = pos;
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
