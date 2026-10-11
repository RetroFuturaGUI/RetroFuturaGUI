#version 330 core

uniform mat4 uPosition;
uniform mat4 uScaling;
uniform mat4 uRotation;

layout (location = 0) in vec2 Position;
out vec4 vs_WorldPosition;

void main()
{
    // Line.gs widens the line in world space, after the scaling, so its thickness stays in pixels however the line is stretched
    vs_WorldPosition = uPosition * uRotation * uScaling * vec4(Position, 0.0, 1.0);
}
