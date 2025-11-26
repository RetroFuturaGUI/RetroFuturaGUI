#version 330 core

uniform mat4 uProjection;
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
layout(location = 0) in vec4 Position;

out vec2 vTexCoord;

void main()
{
    gl_Position = uProjection * uPosition * uRotation * uScaling * Position;
    vTexCoord = Position.xy * 0.5 + 0.5; // Normalize to [0, 1]
}