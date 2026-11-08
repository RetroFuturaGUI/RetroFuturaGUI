#version 330 core
uniform float uOpacity = 1.0; //Shader::OpacityScope's product, uploaded with every UseProgram
// A rectangle's drop shadow: the shape of what casts it, rounded or cut like it, faded over uShadowBlur at its edges. The
// quad is grown by the blur around the shape. The FILL_ define picks the fill. GLOW turns it into the glow around the
// shape, fading away from its edge over uGlowWidth
#include "Modules/RoundedCorners.glsl"

// A shadow lies under nothing of its own, so the fill shows as it is.
vec4 overBackground(vec4 color)
{
    return color;
}

#ifdef FILL_SOLID
#include "Fills/Solid.glsl"
#else
#include "Fills/Gradient.glsl"
#endif

uniform vec2 uShapeHalfSize; // the half size of the shape casting the shadow, scaled with it
uniform float uShadowBlur; // how wide the edges fade, in pixels: half inside the shape, half outside
uniform float uRingWidth; // a border's width, casting the shadow of its outline only; 0 casts the whole shape

#ifdef GLOW
uniform float uGlowWidth; // how far the glow reaches out of the edge, in pixels
uniform int uGlowInward; // 1 glows inward from the ring's inner edge, 0 outward from the shape's outer edge

// How much of the glow covers a pixel this far beyond its edge: full at the edge, nothing past the width, none on the shape
float glowCoverage(float distance)
{
    if (distance < 0.0)
        return 0.0;

    return 1.0 - smoothstep(0.0, uGlowWidth, distance);
}
#endif

layout(location = 0) out vec4 Color;

// How much of the shadow covers a pixel this far outside the shape (negative inside it)
float shadowCoverage(float distance)
{
    if (uShadowBlur <= 0.0)
        return distance <= 0.0 ? 1.0 : 0.0;

    return 1.0 - smoothstep(-0.5 * uShadowBlur, 0.5 * uShadowBlur, distance);
}

void main()
{
    vec2 p = vLocalPos * uScale;
    float cornerRadius = ((uDIP & ROUNDED_CORNERS) != 0) ? cornerRadiusAt(p) : 0.0;
    float innerRadius = max(cornerRadius - uRingWidth * (isCutCornerAt(p) ? CUT_INSET_PER_WIDTH : 1.0), 0.0);

#ifdef GLOW
    float coverage = uGlowInward != 0
        ? glowCoverage(-cornerShapeSDF(p, uShapeHalfSize - vec2(uRingWidth), innerRadius))
        : glowCoverage(cornerShapeSDF(p, uShapeHalfSize, cornerRadius));
#else
    float coverage = shadowCoverage(cornerShapeSDF(p, uShapeHalfSize, cornerRadius));

    if (uRingWidth > 0.0)
        coverage -= shadowCoverage(cornerShapeSDF(p, uShapeHalfSize - vec2(uRingWidth), innerRadius));
#endif

    if (coverage < 1.0 / 255.0)
        discard;

    vec4 finalColor = fillColor();
    Color = vec4(finalColor.rgb, finalColor.a * coverage * uOpacity); //faded with the widget and whatever fades around it
}
