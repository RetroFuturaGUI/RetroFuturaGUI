#include "Common.glsl"

// What was drawn behind the plane, copied right before it draws, for the pixelation (Pixelate.glsl) and the CRT look
// (Crt.glsl) to show.
uniform sampler2D uWindowBehindTexture;
uniform vec4 uWindowBehindRegion; // where in the window the copy came from, in pixels: x, y, width, height.
uniform vec2 uWindowBehindTextureSize; // the texture's size, which may be larger than the copy.
uniform mat4 uWindowBehindPlacement; // the quad's placement into clip space: projection, frame, position, rotation, scaling.
uniform vec2 uWindowBehindViewportSize; // the window's size in pixels.

// The window behind a point of the plane, given in its own pixels from its middle. The copy reaches past the plane by a
// block and the displacement, so even an edge block or a pulled channel finds what lies behind it.
vec4 windowBehindAt(vec2 planePixel)
{
    vec4 clipPosition = uWindowBehindPlacement * vec4(planePixel / max(uScale, vec2(1.0)), 0.0, 1.0);
    vec2 windowPosition = (clipPosition.xy / max(clipPosition.w, 1e-6) * 0.5 + 0.5) * uWindowBehindViewportSize;
    vec2 copied = clamp(windowPosition - uWindowBehindRegion.xy, vec2(0.5), max(uWindowBehindRegion.zw - 0.5, vec2(0.5)));
    vec4 behind = texture(uWindowBehindTexture, copied / max(uWindowBehindTextureSize, vec2(1.0)));
    behind.rgb /= max(behind.a, 1e-4); // the window holds its colors premultiplied by their alpha.
    return behind;
}
