#version 330 core

layout(location = 0) in vec2 Position;

out vec2 vTexCoord;

void main() {
    gl_Position = vec4(Position, 0.0, 1.0);
    vTexCoord = Position * 0.5 + 0.5; // Normalize to [0, 1]
}