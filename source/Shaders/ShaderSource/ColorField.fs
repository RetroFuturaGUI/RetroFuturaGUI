#version 330 core
// The ColorPicker's field of colors, a circle (SHAPE_CIRCLE) or a square (SHAPE_SQUARE)

in vec2 vLocalPos;
out vec4 fragColor;

uniform vec2 uScale;       // the quad's size in pixels
uniform float uBrightness; // 0..1, darkens the whole field

/* The fully saturated color of hue (0..1, 0 = red). Must stay the same formula as ColorPicker::updateColor,
   or the color under the indicator differs from the color picked. */
vec3 hueToRgb(float hue)
{
    return clamp(abs(mod(hue * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
}

#if defined(SHAPE_CIRCLE)
void main()
{
    const float TAU = 6.28318530718;

    // In pixels, so a non-square quad still gets a round circle, sized by its shorter side
    vec2 pixelPos = vLocalPos * uScale;
    float radius = 0.5 * min(uScale.x, uScale.y);
    float dist = length(pixelPos);

    // The rim fades out over about one screen pixel instead of stepping
    float edgeWidth = max(fwidth(dist), 1e-4);
    float coverage = 1.0 - smoothstep(radius - edgeWidth, radius, dist);

    if (coverage <= 0.0)
        discard;

    // Hue runs counter-clockwise from red at the right. atan is undefined at the center, where the color is white anyway
    float hue = dist > 0.0 ? fract(atan(pixelPos.y, pixelPos.x) / TAU) : 0.0;
    float saturation = clamp(dist / radius, 0.0, 1.0); // white at the center, full at the rim

    fragColor = vec4(uBrightness * mix(vec3(1.0), hueToRgb(hue), saturation), coverage);
}
#elif defined(SHAPE_SQUARE)
void main()
{
    vec2 uv = vLocalPos + 0.5; // 0..1 across the quad, 0,0 at the bottom left

    // Hue runs left to right, from red back to red. Saturation runs from white at the bottom to full at the top
    fragColor = vec4(uBrightness * mix(vec3(1.0), hueToRgb(uv.x), uv.y), 1.0);
}
#endif
