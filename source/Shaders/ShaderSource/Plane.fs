#version 330 core
// A filled rectangle. The FILL_ define picks the fill, and WAVE_PLANE adds the wave line WavePlane draws
#include "Modules/RoundedCorners.glsl"
#include "Modules/Raster.glsl"
#include "Modules/Fog.glsl"
#include "Modules/BackgroundGap.glsl"
#include "Modules/Glass.glsl"
#include "Modules/Image.glsl"

#ifdef WAVE_PLANE
#include "Modules/Wave.glsl"
#endif

// Lays a fill color over what shows through it, nearest first: the plane's image, then the window behind the glass
vec4 overBackground(vec4 color)
{
    if((uDIP & BACKGROUND_IMAGE) != 0)
        color = applyImage(color);

    if((uDIP & GLASS_EFFECT) != 0)
        color = applyGlass(color, glassUV());

    return color;
}

#ifdef FILL_SOLID
#include "Fills/Solid.glsl"
#else
#include "Fills/Gradient.glsl"
#endif

layout(location = 0) out vec4 Color;

void main()
{
    vec4 finalColor = fillColor();

    if((uDIP & RASTER) != 0)
        finalColor = applyRaster(finalColor, vLocalPos);

    if((uDIP & FOG_EFFECT) != 0)
        finalColor = applyFog(finalColor, vLocalPos);

    if((uDIP & ROUNDED_CORNERS) != 0 && isOutsideRoundedCorners(vLocalPos))
        discard;

#ifdef WAVE_PLANE
    if((uDIP & WAVE) != 0 && isOutsideWaveLine(vLocalPos))
        discard;
#endif

    if(isInBackgroundGap(vLocalPos * uScale))
        discard;

    Color = finalColor;
}
