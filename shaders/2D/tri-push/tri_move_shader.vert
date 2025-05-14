#version 450
layout(push_constant) uniform PushConstants {
    float time;
} pc;

vec2 positions[3] = vec2[](
vec2(0.0, -0.5),
vec2(0.5, 0.5),
vec2(-0.5, 0.5)
);

void main() {
    float angle = pc.time;
    mat2 rotation = mat2(
    cos(angle), -sin(angle),
    sin(angle),  cos(angle)
    );
    vec2 pos = rotation * positions[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);
}
