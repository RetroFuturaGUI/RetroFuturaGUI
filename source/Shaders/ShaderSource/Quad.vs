#version 330 core
// Every rectangle's vertex shader: the shared quad, -0.5 .. 0.5, sized, turned and moved into place
#include "Modules/Clip.glsl"

uniform mat4 uProjection;
uniform mat4 uFrame = mat4(1.0); // tilts the untilted layout into place: the FrameScopes around the draw.
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
layout(location = 0) in vec4 Position;

out vec2 vLocalPos;
out vec2 vTexCoord;
out vec2 vUV;

void main()
{
    vLocalPos = Position.xy; // -0.5 .. 0.5 across the quad
    vec4 placedPosition = uFrame * uPosition * uRotation * uScaling * Position;
    gl_Position = uProjection * placedPosition;
    clipToScopes(placedPosition);
    vTexCoord = Position.xy + 0.5; // 0 .. 1 across the quad, like the SVG icons' coordinates
    vUV = (gl_Position.xy / gl_Position.w * 0.5 + 0.5); // where the pixel sits in the window, 0 .. 1, for the glass
}
