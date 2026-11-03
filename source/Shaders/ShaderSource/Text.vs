#version 330 core
// Text's glyph quads, laid out in the text's own space with the first baseline at y = 0, moved and turned into place
#include "Modules/Clip.glsl"

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aLineCenter; // the middle of the glyph's line across it: y for a line, x for a column

uniform mat4 uProjection;
uniform mat4 uFrame = mat4(1.0); // tilts the untilted layout into place: the FrameScopes around the draw.
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
uniform vec2 uScale;      // the text span's size in pixels
uniform vec2 uSpanCenter; // the text span's center, in the text's own space
uniform int uMaskPerLine = 0; // 1 lays the mask over each line (each column) instead of the whole text
uniform int uIsVertical = 0;  // 1 when the lines are columns
uniform float uLineExtent = 1.0; // a line's height from its lowest to its highest glyph, or a column's width

out vec2 vGlyphUV;
out vec2 vLocalPos;
out vec2 vTexCoord;
out vec2 vUV;
out vec2 vMaskPos;

void main()
{
    vec4 placedPosition = uFrame * uPosition * uRotation * uScaling * vec4(Position, 0.0, 1.0);
    gl_Position = uProjection * placedPosition;
    clipToScopes(placedPosition);
    vGlyphUV = aTexCoord;
    vLocalPos = (Position - uSpanCenter) / uScale; // -0.5 .. 0.5 across the whole text, so a gradient, raster or fog runs across all of it
    vTexCoord = vLocalPos + 0.5;
    vUV = (gl_Position.xy / gl_Position.w * 0.5 + 0.5); // where the pixel sits in the window, 0 .. 1, for the glass

    // Per line, the mask spans the line across it and the whole text along it
    vMaskPos = vLocalPos;

    if(0 != uMaskPerLine && 0 != uIsVertical)
        vMaskPos.x = (Position.x - aLineCenter) / uLineExtent;
    else if(0 != uMaskPerLine)
        vMaskPos.y = (Position.y - aLineCenter) / uLineExtent;
}
