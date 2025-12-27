#version 330 core

uniform mat4 uProjection;
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
layout(location = 0) in vec4 Position;

out vec2 vLocalPos;
out vec2 vUV;

void main()
{
    vLocalPos = Position.xy;
    gl_Position = uProjection * uPosition * uRotation * uScaling * Position;
    vUV = (gl_Position.xy * 0.5 + 0.5);
}