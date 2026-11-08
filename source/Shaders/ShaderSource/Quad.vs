#version 330 core
// Every rectangle's vertex shader: the shared quad, -0.5 .. 0.5, sized, turned and moved into place
#include "Modules/Clip.glsl"

uniform mat4 uProjection;
uniform mat4 uFrame = mat4(1.0); // tilts the untilted layout into place: the FrameScopes around the draw.
uniform mat4 uScaling;
uniform mat4 uPosition;
uniform mat4 uRotation;
uniform vec2 uScale; // the quad's size in pixels
layout(location = 0) in vec4 Position;

out vec2 vLocalPos;
out vec2 vTexCoord;
out vec2 vUV;

const float EDGE_MARGIN = 1.0; // pixels the quad reaches past each edge, so the edges' fade has room outside the shape too.

// Whether the rectangle lies flat and unturned under an orthographic projection, where a layout pixel is a screen pixel.
bool isPixelAligned(mat4 placement)
{
    bool isUnturned = abs(placement[0][1]) < 1e-5 && abs(placement[1][0]) < 1e-5 && abs(placement[0][2]) < 1e-5 && abs(placement[1][2]) < 1e-5;
    bool isOrthographic = abs(uProjection[0][3]) + abs(uProjection[1][3]) + abs(uProjection[2][3]) < 1e-6;
    return isUnturned && isOrthographic;
}

void main()
{
    vec2 grownPosition = Position.xy * (1.0 + 2.0 * EDGE_MARGIN / max(uScale, vec2(1.0)));
    vLocalPos = grownPosition; // -0.5 .. 0.5 across the shape, a little more on the margin.
    mat4 placement = uFrame * uPosition * uRotation * uScaling;
    vec4 placedPosition = placement * vec4(grownPosition, Position.zw);

    // Edges that run along the pixels land on whole pixels, the pixels whose centers they hold, so a straight edge or a thin
    // line stays crisp instead of spreading over two pixels at half strength. The margin is whole, so the edges move with it.
    if (isPixelAligned(placement))
        placedPosition.xy = ceil(placedPosition.xy - 0.5);

    gl_Position = uProjection * placedPosition;
    clipToScopes(placedPosition);
    vTexCoord = grownPosition + 0.5; // 0 .. 1 across the shape, like the SVG icons' coordinates.
    vUV = (gl_Position.xy / gl_Position.w * 0.5 + 0.5); // where the pixel sits in the window, 0 .. 1, for the glass
}
