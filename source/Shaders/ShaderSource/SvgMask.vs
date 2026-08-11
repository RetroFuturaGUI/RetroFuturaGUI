#version 330 core

uniform mat4 uProjection;
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
layout (location = 0) in vec4 Position;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	gl_Position = uProjection * uPosition * uRotation * uScaling * Position;
	vTexCoord = aTexCoord;
}
