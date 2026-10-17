#version 330 core

in vec2 vLocalPos;
out vec4 fragColor;

uniform float uBrightness; // 0..1, darkens the whole square

/* The fully saturated color of hue (0..1, 0 = red). Must stay the same formula as ColorPicker::updateColor,
   or the color under the indicator differs from the color picked. */
vec3 hueToRgb(float hue)
{
    return clamp(abs(mod(hue * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
}

void main()
{
    vec2 uv = vLocalPos + 0.5; // 0..1 across the quad, 0,0 at the bottom left

    // Hue runs left to right, from red back to red. Saturation runs from white at the bottom to full at the top
    fragColor = vec4(uBrightness * mix(vec3(1.0), hueToRgb(uv.x), uv.y), 1.0);
}
