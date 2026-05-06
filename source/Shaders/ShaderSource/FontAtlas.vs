#version 330 core

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;

out vec2 TexCoord;

void main()
{
    gl_Position = uProjection * uPosition * uRotation * uScaling * vec4(Position, 0.0, 1.0);
    TexCoord = aTexCoord;
}
