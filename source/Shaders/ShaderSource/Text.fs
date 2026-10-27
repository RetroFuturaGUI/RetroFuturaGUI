#version 330 core
// Text: the fill where each glyph covers the pixel. The FILL_ define picks the fill, and the raster, fog and glass are laid
// over it as on a Plane
#include "Modules/Raster.glsl"
#include "Modules/Fog.glsl"
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

in vec2 vGlyphUV;
out vec4 fragColor;

uniform sampler2D uGlyphs;

void main()
{
    float coverage = texture(uGlyphs, vGlyphUV).r;

    if (coverage <= 0.0)
        discard;

    vec4 finalColor = fillColor();

    if((uDIP & RASTER) != 0)
        finalColor = applyRaster(finalColor, vLocalPos);

    if((uDIP & FOG_EFFECT) != 0)
        finalColor = applyFog(finalColor, vLocalPos);

    fragColor = vec4(finalColor.rgb, finalColor.a * coverage);
}
