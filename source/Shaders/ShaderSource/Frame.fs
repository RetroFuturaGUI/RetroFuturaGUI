#version 330 core
// A rectangle's outline, uBorderWidth wide. The FILL_ define picks the fill
#include "Modules/RoundedCorners.glsl"
#include "Modules/Raster.glsl"
#include "Modules/BorderGap.glsl"
#include "Modules/Glass.glsl"

// Lays a fill color over the window behind the glass
vec4 overBackground(vec4 color)
{
    if((uDIP & GLASS_EFFECT) == 0)
        return color;

    return applyGlass(color, glassUV());
}

#ifdef FILL_SOLID
#include "Fills/Solid.glsl"
#else
#include "Fills/Gradient.glsl"
#endif

uniform float uBorderWidth;

layout(location = 0) out vec4 Color;

void main()
{
    vec2 scaledPos = vLocalPos * uScale;
    vec2 halfSize = vec2(0.5) * uScale;
    vec2 innerHalfSize = halfSize - vec2(uBorderWidth);

    float cornerRadius = ((uDIP & ROUNDED_CORNERS) != 0) ? cornerRadiusAt(scaledPos) : 0.0; // each corner its own radius, so the outline follows the fill
    float innerRadius = max(cornerRadius - uBorderWidth, 0.0);

    float outerDist = roundedRectSDF(scaledPos, halfSize, cornerRadius);
    float innerDist = roundedRectSDF(scaledPos, innerHalfSize, innerRadius);

    if (outerDist > 0.0 || innerDist <= 0.0 || isInBorderGap(scaledPos, halfSize, innerHalfSize))
        discard;

#ifdef FILL_SOLID
    vec4 finalColor = uColor; // no glass on a solid frame: the old border shader dropped it, so this keeps the look until that's decided
#else
    vec4 finalColor = fillColor();
#endif

    if((uDIP & RASTER) != 0)
        finalColor = applyRaster(finalColor, vLocalPos);

    Color = finalColor;
}
