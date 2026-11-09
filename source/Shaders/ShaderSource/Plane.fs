#version 330 core
uniform float uOpacity = 1.0; //Shader::OpacityScope's product, uploaded with every UseProgram
// A filled rectangle. The FILL_ define picks the fill, and WAVE_PLANE adds the wave line WavePlane draws
#include "Modules/RoundedCorners.glsl"
#include "Modules/Raster.glsl"
#include "Modules/Fog.glsl"
#include "Modules/BackgroundGap.glsl"
#include "Modules/Glass.glsl"
#include "Modules/Image.glsl"
#include "Modules/Mask.glsl"
#include "Modules/Pixelate.glsl"

#ifdef WAVE_PLANE
#include "Modules/Wave.glsl"
#endif

// Lays a fill color over what shows through it, nearest first: the plane's image, then the window behind, pixelated or
// through the glass. Pixelation shows what was really drawn behind, so it takes the glass's place where both are on.
vec4 overBackground(vec4 color)
{
    if((uDIP & BACKGROUND_IMAGE) != 0)
        color = applyImage(color);

    if((uDIP & PIXELATE) != 0)
        return applyPixelate(color);

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
    float coverage = shapeCoverage(vLocalPos);

    if(coverage <= 0.0)
        discard;

    vec4 finalColor = fillColor();

    if((uDIP & RASTER) != 0)
        finalColor = applyRaster(finalColor, vLocalPos);

    if((uDIP & FOG_EFFECT) != 0)
        finalColor = applyFog(finalColor, vLocalPos);

    if((uDIP & MASK) != 0)
        finalColor = applyMask(finalColor);

#ifdef WAVE_PLANE
    if((uDIP & WAVE) != 0 && isOutsideWaveLine(vLocalPos))
        discard;
#endif

    if(isInBackgroundGap(vLocalPos * uScale))
        discard;

    Color = finalColor;
    Color.a *= coverage * uOpacity; //faded at its edges, and with the widget and whatever fades around it.
}
