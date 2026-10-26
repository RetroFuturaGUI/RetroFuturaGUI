// The color stops and the animation every gradient shares. Where a pixel lies along the gradient comes from
// gradientPosition(), in the file the FILL_ define picks. overBackground() comes from the class shader

in vec2 vTexCoord;

uniform float uDegree;
uniform int uNumColors;
uniform vec4 uColors[255]; // 255 colors maximum. I don't think you'll need more than that
uniform float uGradientOffset;

#if defined(FILL_LINEAR_SHIFT)
#include "LinearShift.glsl"
#elif defined(FILL_RADIAL_GRADIENT)
#include "RadialGradient.glsl"
#elif defined(FILL_HUESTAR_GRADIENT)
#include "HueStarGradient.glsl"
#endif

// The two stops this pixel lies between, and how far it is from the first to the second
void gradientStops(out vec4 color1, out vec4 color2, out float interpolation)
{
    float gradient = gradientPosition();

    // Apply the offset to the gradient (makes colors "move")
    gradient += uGradientOffset;

    // get the fractional part to create a looping effect
    gradient = fract(gradient);
    gradient = clamp(gradient, 0.0, 1.0);

    // Calculate smooth color index and interpolation factor
    float colorIndex = gradient * float(uNumColors);

    // Ensure idx1 is always within bounds
    int idx1 = min(int(floor(colorIndex)), uNumColors - 1);

    // For idx2, wrap around to 0 if it exceeds uNumColors - 1
    int idx2 = (idx1 + 1) % uNumColors;
    interpolation = fract(colorIndex);

    color1 = uColors[idx1];
    color2 = uColors[idx2];
}

// The fill's color at this pixel. Each stop is laid over whatever shows through the fill before the two are mixed
vec4 fillColor()
{
    vec4 color1;
    vec4 color2;
    float interpolation;
    gradientStops(color1, color2, interpolation);

    return mix(overBackground(color1), overBackground(color2), interpolation);
}
