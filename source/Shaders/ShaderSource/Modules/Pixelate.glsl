#include "WindowBehind.glsl"

uniform float uPixelateBlockSize; // the blocks' width, in the plane's own pixels.

// Lays color over what was drawn behind the plane, shown in square blocks of one color each: every block shows the
// pixel behind its middle. The blocks sit on the plane's own grid from its corner, so they tilt and recede with it.
vec4 applyPixelate(vec4 color)
{
    float blockSize = max(uPixelateBlockSize, 1.0);
    vec2 corner = -0.5 * uScale;
    vec2 blockMiddle = corner + (floor((vLocalPos * uScale - corner) / blockSize) + 0.5) * blockSize;
    return mix(windowBehindAt(blockMiddle), color, color.a);
}
