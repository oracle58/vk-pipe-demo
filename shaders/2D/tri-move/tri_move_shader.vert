#version 450

layout(push_constant) uniform PushConstants {
    float time;
} pc;

// Base triangle in local space
const vec2 basePos[3] = vec2[](
vec2( 0.0, -0.5),
vec2( 0.5,  0.5),
vec2(-0.5,  0.5)
);

layout(location = 1) flat out float vInstanceId;

void main() {
    // convert instance index to float
    float id = floor(float(gl_VertexIndex) / 3.0);

    // each triangle orbits at a slightly different speed
    float speed    = 0.5 + 0.1 * id;
    float angle    = pc.time * speed + id * 0.6;

    // radius grows over time, modulated per instance
    float radius   = 0.2 + 0.05 * id + 0.3 * sin(pc.time * (0.3 + 0.02 * id));

    // build 2D rotation matrix
    mat2 rot = mat2(
    cos(angle), -sin(angle),
    sin(angle),  cos(angle)
    );

    // compute final position: rotate+scale the base triangle, then offset along its spiral path
    vec2 localPos = rot * (basePos[gl_VertexIndex % 3] * 0.2);
    vec2 offset   = vec2(cos(angle), sin(angle)) * radius;
    vec2 pos      = localPos + offset;

    gl_Position = vec4(pos, 0.0, 1.0);
    vInstanceId  = id;
}