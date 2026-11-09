#include "Common.glsl"

uniform sampler2D uPixelateTexture; // the window behind the plane, copied right before it draws.
uniform vec4 uPixelateRegion; // where in the window the copy came from, in pixels: x, y, width, height.
uniform vec2 uPixelateTextureSize; // the texture's size, which may be larger than the copy.
uniform float uPixelateBlockSize; // the blocks' width, in the plane's own pixels.
uniform mat4 uPixelatePlacement; // the quad's placement into clip space: projection, frame, position, rotation, scaling.
uniform vec2 uPixelateViewportSize; // the window's size in pixels.

// Lays color over what was drawn behind the plane, shown in square blocks of one color each: every block shows the
// pixel behind its middle. The blocks sit on the plane's own grid from its corner, so they tilt and recede with it.
vec4 applyPixelate(vec4 color)
{
    float blockSize = max(uPixelateBlockSize, 1.0);
    vec2 corner = -0.5 * uScale;
    vec2 blockMiddle = corner + (floor((vLocalPos * uScale - corner) / blockSize) + 0.5) * blockSize;

    vec4 clipPosition = uPixelatePlacement * vec4(blockMiddle / max(uScale, vec2(1.0)), 0.0, 1.0);
    vec2 windowPosition = (clipPosition.xy / max(clipPosition.w, 1e-6) * 0.5 + 0.5) * uPixelateViewportSize;

    // Within the copy, which reaches a block past the plane, so even an edge block finds its middle.
    vec2 copied = clamp(windowPosition - uPixelateRegion.xy, vec2(0.5), max(uPixelateRegion.zw - 0.5, vec2(0.5)));
    vec4 behind = texture(uPixelateTexture, copied / max(uPixelateTextureSize, vec2(1.0)));
    behind.rgb /= max(behind.a, 1e-4); // the window holds its colors premultiplied by their alpha.

    return mix(behind, color, color.a);
}
