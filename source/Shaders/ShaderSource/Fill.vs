#version 330 core

uniform mat4 uProjection;
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
layout(location = 0) in vec4 Position;

out vec2 vLocalPos;

void main()
{
    vLocalPos = Position.xy;
    gl_Position = uProjection * uPosition * uRotation * uScaling * Position;
}