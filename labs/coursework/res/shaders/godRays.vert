#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) out vec2 vUv;

void main() {
    vUv = 0.5 * (position+1.0);

    gl_Position = vec4(position.xy, 0.0, 1.0);
}