#version 330 core
// Every rectangle's vertex shader: the shared quad, -0.5 .. 0.5, sized, turned and moved into place

uniform mat4 uProjection;
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
    gl_Position = uProjection * uPosition * uRotation * uScaling * Position;
    vTexCoord = Position.xy + 0.5; // 0 .. 1 across the quad, like the SVG icons' coordinates
    vUV = (gl_Position.xy * 0.5 + 0.5); // where the pixel sits in the window, 0 .. 1, for the glass
}
