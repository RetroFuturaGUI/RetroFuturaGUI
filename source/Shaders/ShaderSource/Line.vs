#version 330 core

uniform mat4 uProjection;
uniform mat4 uPosition;
uniform mat4 uScaling;
uniform mat4 uRotation;

layout (location = 0) in vec4 Position;
out vec4 vs_WorldPosition;

void main()
{
    vs_WorldPosition = uPosition * uRotation * uScaling * Position;
    gl_Position = uProjection * vs_WorldPosition;
}