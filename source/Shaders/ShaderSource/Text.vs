#version 330 core
layout (location = 0) in vec2 vertex;

uniform mat4 uProjection;
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;

out VS_OUT{
    vec2 TexCoords;
    flat int index;
}vs_out;

uniform mat4 transforms[400];

void main()
{
    gl_Position = uProjection * uPosition * uRotation * /* uScaling **/ transforms[gl_InstanceID]* vec4(vertex.xy, 0.0, 1.0);
    vs_out.index = gl_InstanceID;
    vs_out.TexCoords = vertex.xy;
    vs_out.TexCoords.y = 1.0f - vs_out.TexCoords.y;
}