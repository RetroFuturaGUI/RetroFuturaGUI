#version 330 core

uniform mat4 uProjection;
layout(location = 0) in vec4 Position;

void main()
{
    gl_Position = uProjection * Position;
}