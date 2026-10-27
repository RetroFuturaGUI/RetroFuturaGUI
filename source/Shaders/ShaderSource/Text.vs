#version 330 core
// Text's glyph quads, laid out in the text's own space with the first baseline at y = 0, moved and turned into place

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
uniform vec2 uScale;      // the text span's size in pixels
uniform vec2 uSpanCenter; // the text span's center, in the text's own space

out vec2 vGlyphUV;
out vec2 vLocalPos;
out vec2 vTexCoord;
out vec2 vUV;

void main()
{
    gl_Position = uProjection * uPosition * uRotation * uScaling * vec4(Position, 0.0, 1.0);
    vGlyphUV = aTexCoord;
    vLocalPos = (Position - uSpanCenter) / uScale; // -0.5 .. 0.5 across the whole text, so a gradient, raster or fog runs across all of it
    vTexCoord = vLocalPos + 0.5;
    vUV = (gl_Position.xy * 0.5 + 0.5); // where the pixel sits in the window, 0 .. 1, for the glass
}
