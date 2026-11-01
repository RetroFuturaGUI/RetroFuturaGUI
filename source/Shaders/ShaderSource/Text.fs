#version 330 core
uniform float uOpacity = 1.0; //Shader::OpacityScope's product, uploaded with every UseProgram
// Text: the fill where each glyph covers the pixel. The FILL_ define picks the fill, and the raster, fog and glass are laid
// over it as on a Plane
#include "Modules/Raster.glsl"
#include "Modules/Fog.glsl"
#include "Modules/Glass.glsl"
#include "Modules/Image.glsl"
in vec2 vMaskPos; //where the mask is laid: across the whole text, or across each line
#define MASK_POSITION vMaskPos
#include "Modules/Mask.glsl"

bool gIsPlainFill = false; //set while a color glyph asks for the fill's own color, without what shows through it

// Lays a fill color over what shows through it, nearest first: the text's image, stretched across the whole text, then
// the window behind the glass
vec4 overBackground(vec4 color)
{
    if(gIsPlainFill)
        return color;

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
uniform int uColorGlyphs = 0; //1 while drawing from a color page: a color emoji keeps its own colors instead of the fill
uniform float uColorGlyphTint = 0.0; //how far a color glyph takes on the fill's color, 0 to 1

// A color glyph's own color: tinted toward the fill as far as uColorGlyphTint says, and over the glass as see-through as the
// fill is, so it matches the text around it. The text's image stays out, as it would hide the glyph's colors like the fill
vec4 colorGlyphColor(vec4 glyph)
{
    vec4 color = vec4(glyph.rgb / glyph.a, 1.0); //kept premultiplied, so filtering blends no black into the edges

    gIsPlainFill = true;
    vec4 fill = fillColor();
    gIsPlainFill = false;

    color.rgb = mix(color.rgb, fill.rgb, uColorGlyphTint * fill.a);

    if((uDIP & GLASS_EFFECT) != 0)
        color = applyGlass(vec4(color.rgb, fill.a), glassUV());

    return color;
}

void main()
{
    //The glyph's own colors or the fill, with the glyph's coverage as its alpha; the raster, fog and mask go over either
    vec4 glyph = texture(uGlyphs, vGlyphUV);
    float coverage = 0 != uColorGlyphs ? glyph.a : glyph.r;

    if (coverage <= 0.0)
        discard;

    vec4 finalColor = 0 != uColorGlyphs ? colorGlyphColor(glyph) : fillColor();

    if((uDIP & RASTER) != 0)
        finalColor = applyRaster(finalColor, vLocalPos);

    if((uDIP & FOG_EFFECT) != 0)
        finalColor = applyFog(finalColor, vLocalPos);

    if((uDIP & MASK) != 0)
        finalColor = applyMask(finalColor);

    fragColor = vec4(finalColor.rgb, finalColor.a * coverage);
    fragColor.a *= uOpacity; //faded with the widget and whatever fades around it
}
