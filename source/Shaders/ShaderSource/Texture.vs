#version 330 core
#include "Modules/Clip.glsl"

uniform mat4 uProjection;
uniform mat4 uFrame = mat4(1.0); // tilts the untilted layout into place: the FrameScopes around the draw.
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
layout (location = 0) in vec4 Position;
layout (location = 1) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

void main()
{
	vec4 placedPosition = uFrame * uPosition * uRotation * uScaling * Position;
	gl_Position = uProjection * placedPosition;
	clipToScopes(placedPosition);
	TexCoord = aTexCoord;
}