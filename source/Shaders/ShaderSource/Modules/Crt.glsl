#include "WindowBehind.glsl"

uniform float uCrtChannelDisplacement; // how far red is pulled right and blue left, in the plane's own pixels.
uniform float uCrtScanlineWidth; // how tall each scanline and each lit line between them is; 0 draws none.
uniform float uCrtScanlineAlpha; // how opaque the dark scanlines are, 0 to 1.

// Lays color over what was drawn behind the plane as a CRT screen shows it: red and blue pulled apart sideways, and every
// other line of the scanline width darkened, counted from the plane's top edge.
vec4 applyCrt(vec4 color)
{
    vec2 planePixel = vLocalPos * uScale;
    vec2 displacement = vec2(uCrtChannelDisplacement, 0.0);
    vec4 middle = windowBehindAt(planePixel);
    vec4 behind = vec4(windowBehindAt(planePixel + displacement).r, middle.g, windowBehindAt(planePixel - displacement).b, middle.a);

    if(uCrtScanlineWidth > 0.0)
    {
        // sin() is positive across a lit line and negative across a scanline, and fades between them over about a pixel.
        float lineParity = sin((0.5 * uScale.y - planePixel.y) / uCrtScanlineWidth * 3.14159265359);
        float parityAA = max(fwidth(lineParity), 1e-4);
        float scanline = 1.0 - smoothstep(-parityAA, parityAA, lineParity);
        behind.rgb *= 1.0 - clamp(uCrtScanlineAlpha, 0.0, 1.0) * scanline; // black at the scanlines' alpha laid over.
    }

    return mix(behind, color, color.a);
}
