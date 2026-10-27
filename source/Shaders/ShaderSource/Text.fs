#version 330 core
// Text: the fill where each glyph covers the pixel. The FILL_ define picks the fill, and the raster, fog and glass are laid
// over it as on a Plane
#include "Modules/Raster.glsl"
#include "Modules/Fog.glsl"
#include "Modules/Glass.glsl"
#include "Modules/Image.glsl"

// Lays a fill color over what shows through it, nearest first: the text's image, stretched across the whole text, then
// the window behind the glass
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
