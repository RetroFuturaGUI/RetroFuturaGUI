#version 330 core

uniform mat4 uProjection;
uniform mat4 uScaling;
uniform mat4 uPosition;
layout(location = 0) in vec4 Position;

void main()
{
    gl_Position = uProjection * uPosition * uScaling * Position;
}