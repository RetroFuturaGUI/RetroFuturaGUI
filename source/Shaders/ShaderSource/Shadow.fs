#version 330 core
uniform float uOpacity = 1.0; //Shader::OpacityScope's product, uploaded with every UseProgram
// A rectangle's drop shadow: the shape of what casts it, rounded or cut like it, faded over uShadowBlur at its edges. The
// quad is grown by the blur around the shape. The FILL_ define picks the fill. GLOW turns it into the glow around the
// shape, fading away from its edge over uGlowWidth. INNER_GLOW draws the light inside the shape instead, fading in from its
// edges, moved by uInnerOffset, over uGlowWidth
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

#if defined(GLOW) || defined(INNER_GLOW)
uniform float uGlowWidth; // how far the glow reaches from the edge, in pixels
#endif

#ifdef INNER_GLOW
uniform vec2 uInnerOffset; // how far the edges the inner glow is measured from are moved, in pixels
#endif

#ifdef GLOW
uniform int uGlowInward; // 1 glows inward from the ring's inner edge, 0 outward from the shape's outer edge

// How much of the glow covers a pixel this far beyond its edge: full at the edge, nothing past the width, none on the shape.
// Under the shape's faded edge it fills what the shape leaves uncovered, so no seam shows between them.
float glowCoverage(float distance)
{
    return (1.0 - edgeCoverage(distance)) * (1.0 - smoothstep(0.0, uGlowWidth, max(distance, 0.0)));
}
#endif

layout(location = 0) out vec4 Color;

// How much of the shadow covers a pixel this far outside the shape (negative inside it)
float shadowCoverage(float distance)
{
    if (uShadowBlur <= 0.0)
        return edgeCoverage(distance);

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
#elif defined(INNER_GLOW)
    // Only inside the shape, or inside a ring's outline, faded at their edges like them.
    float insideCoverage = edgeCoverage(cornerShapeSDF(p, uShapeHalfSize, cornerRadius));
    float ringHoleCoverage = edgeCoverage(cornerShapeSDF(p, uShapeHalfSize - vec2(uRingWidth), innerRadius));

    if (uRingWidth > 0.0)
        insideCoverage *= 1.0 - ringHoleCoverage;

    // How deep inside the edges moved by the offset: full at those edges and beyond them, nothing past the width
    vec2 q = p - uInnerOffset;
    float shiftedRadius = ((uDIP & ROUNDED_CORNERS) != 0) ? cornerRadiusAt(q) : 0.0;
    float depth = -cornerShapeSDF(q, uShapeHalfSize, shiftedRadius);

    if (uRingWidth > 0.0)
    {
        float shiftedInnerRadius = max(shiftedRadius - uRingWidth * (isCutCornerAt(q) ? CUT_INSET_PER_WIDTH : 1.0), 0.0);
        depth = min(depth, cornerShapeSDF(q, uShapeHalfSize - vec2(uRingWidth), shiftedInnerRadius));
    }

    float coverage = insideCoverage * (depth <= 0.0 ? 1.0 : 1.0 - smoothstep(0.0, uGlowWidth, depth));
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
