#version 320 es

precision highp float;

layout(location = 0) out vec4 fragColor;

layout(location = 0) uniform float a;

void main() {
    fragColor = vec4(0.0, 0.5 * a + 0.5, 0.0, 1.0);
}

