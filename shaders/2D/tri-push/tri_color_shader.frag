#version 450
layout(push_constant) uniform PushConstants {
    float time;
} pc;

layout(location = 0) out vec4 outColor;

void main() {
    float r = 0.5 + 0.5 * sin(pc.time);
    float g = 0.5 + 0.5 * sin(pc.time + 2.0);
    float b = 0.5 + 0.5 * sin(pc.time + 4.0);
    outColor = vec4(r, g, b, 1.0);
}
