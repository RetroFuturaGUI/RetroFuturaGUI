#version 330 core

uniform mat4 uProjection;
uniform mat4 uScaling;
layout(location = 0) in vec2 Position;

out vec2 vTexCoord;

void main()
{
    gl_Position = uProjection * uScaling * vec4(Position, 0.0, 1.0);
    vTexCoord = Position * 0.5 + 0.5; // Normalize to [0, 1]
}