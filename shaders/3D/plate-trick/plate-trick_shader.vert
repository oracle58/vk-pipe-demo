#version 450

layout(push_constant) uniform PushConstants {
    vec2 resolution;
    float time;
} pc;

void main() {
    // full-screen triangle (no vertex buffer)
    vec2 pos = vec2((gl_VertexIndex << 1) & 2,
    gl_VertexIndex        & 2);
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
