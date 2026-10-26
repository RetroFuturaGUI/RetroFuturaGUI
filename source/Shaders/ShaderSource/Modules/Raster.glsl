#include "Common.glsl"

#define RASTER_PATTERN_DOTTED 0
#define RASTER_PATTERN_CHECKERED 1
#define MAX_PRIMARY_RASTER_WIDTH_TRANSFER 255

uniform int uRasterPattern;
uniform vec4 uPrimaryRasterColor;
uniform vec4 uSecondaryRasterColor;
uniform float uDotDistance;
uniform float uRasterDegree;
uniform float uDotTransparencyTransfer;
uniform float uRasterAnimationOffset;
uniform int uPrimaryRasterWidthTransferCount;
uniform float uPrimaryRasterWidthTransfer[MAX_PRIMARY_RASTER_WIDTH_TRANSFER];

/* Normalizes a position projected onto the transfer direction into the 0..1 coordinate the width
   transfer is sampled with. fract (not clamp) so the curve repeats seamlessly as a pattern scrolls
   forever, instead of freezing once a position passes the rectangle's original extent. */
float rasterTransferCoord(vec2 samplePos, vec2 direction, vec2 halfSize)
{
    float maxProjection = abs(direction.x) * halfSize.x + abs(direction.y) * halfSize.y;

    if(maxProjection <= 0.0001)
        return 0.0;

    return fract(dot(samplePos, direction) / (2.0 * maxProjection) + 0.5);
}

/* Samples uPrimaryRasterWidthTransfer at t, interpolating between the two entries it falls between -
   this is what lets a pattern's elements grow/shrink smoothly across the rectangle. */
float sampleRasterWidth(float t)
{
    int count = max(uPrimaryRasterWidthTransferCount, 1);
    float widthIndexF = t * float(count - 1);
    int idx0 = clamp(int(floor(widthIndexF)), 0, count - 1);
    int idx1 = clamp(idx0 + 1, 0, count - 1);

    return mix(uPrimaryRasterWidthTransfer[idx0], uPrimaryRasterWidthTransfer[idx1], count > 1 ? fract(widthIndexF) : 0.0);
}

/* Blends a dot-grid pattern over baseColor. Dot centers sit on a uDotDistance grid (in px, local
   to the rectangle) that slides along the uRasterDegree direction over time via
   uRasterAnimationOffset. Each dot's total width is sampled from uPrimaryRasterWidthTransfer, indexed by that same
   dot's position projected onto the direction and normalized across the rectangle's extent -
   this is what lets dots grow/shrink smoothly from one side of the rectangle to the other.*/
vec4 applyDottedPattern(vec4 baseColor, vec2 localPos)
{
    vec2 pixelPos = localPos * uScale;
    vec2 halfSize = uScale * 0.5;

    float angleRad = radians(uRasterDegree);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));

    // Shift the sampled space along the direction to animate the whole pattern rigidly.
    vec2 animatedPos = pixelPos + direction * uRasterAnimationOffset;

    float dotDistance = max(uDotDistance, 0.0001);
    vec2 cellCenter = floor(animatedPos / dotDistance + 0.5) * dotDistance;
    vec2 localOffset = animatedPos - cellCenter;

    // A dot's own center decides which part of the transfer curve sizes it.
    float dotWidth = sampleRasterWidth(rasterTransferCoord(cellCenter, direction, halfSize));

    // The transfer curve gives each dot's width across, so the circle drawn from its center is half of it.
    float dotRadius = dotWidth * 0.5;
    // Clamp to half the spacing so a dot's circle never reaches its Voronoi cell's square boundary
    // (which would otherwise make oversized dots look like squares/diamonds instead of circles).
    dotRadius = min(dotRadius, dotDistance * 0.5);

    float normalizedDist = dotRadius > 0.0001 ? length(localOffset) / dotRadius : 1.0;

    // transparencyTransfer fraction of the radius is fully opaque; the rest fades to transparent.
    // Widen the transition band to at least ~1 screen pixel (via screen-space derivatives) so small
    // or hard-edged (transparencyTransfer near 1.0) dots don't alias into blocky/jagged shapes.
    float innerBound = clamp(uDotTransparencyTransfer, 0.0, 1.0) * (1.0 - 1e-4);
    float aa = max(fwidth(normalizedDist), 1e-4);
    innerBound = min(innerBound, 1.0 - aa);
    float dotAlpha = (1.0 - smoothstep(innerBound, 1.0, normalizedDist)) * uPrimaryRasterColor.a;

    vec4 result = baseColor;
    result.rgb = mix(baseColor.rgb, uPrimaryRasterColor.rgb, dotAlpha);
    result.a = mix(baseColor.a, 1.0, dotAlpha);
    return result;
}

/* Blends a checkerboard over baseColor. Its two axes run along the uRasterDegree direction
   and across it, so the angle turns the whole board, and the board slides along that direction over
   time via uRasterAnimationOffset just as the dots do. Every square's side length comes from
   uPrimaryRasterWidthTransfer: a single-entry curve gives an even board, a varying one stretches it
   from one side of the rectangle to the other. */
vec4 applyCheckeredPattern(vec4 baseColor, vec2 localPos)
{
    vec2 pixelPos = localPos * uScale;
    vec2 halfSize = uScale * 0.5;

    float angleRad = radians(uRasterDegree);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));
    vec2 crossDirection = vec2(-direction.y, direction.x);

    // Shift the sampled space along the direction to animate the whole board rigidly.
    vec2 animatedPos = pixelPos + direction * uRasterAnimationOffset;

    float squareSide = max(sampleRasterWidth(rasterTransferCoord(animatedPos, direction, halfSize)), 0.0001);

    /* Dividing both projections by that side length turns them into board coordinates whose integer
       boundaries are the square edges. The divisor is the size sampled at this very position, so a
       varying curve stretches the board continuously instead of tearing it at a seam. */
    vec2 board = vec2(dot(animatedPos, direction), dot(animatedPos, crossDirection)) / squareSide;

    /* sin() is positive across one square of a row and negative across the next, so the product of
       both axes' sines carries the board's parity and changes sign exactly at every square edge.
       That lets a single smoothstep over the sign change antialias all four edges of a square at
       once - and once squares shrink below a pixel it settles at the halfway mix of both colors
       rather than aliasing into moire. */
    float parity = sin(board.x * 3.14159265359) * sin(board.y * 3.14159265359);
    float aa = max(fwidth(parity), 1e-4);
    vec4 squareColor = mix(uSecondaryRasterColor, uPrimaryRasterColor, smoothstep(-aa, aa, parity));

    vec4 result = baseColor;
    result.rgb = mix(baseColor.rgb, squareColor.rgb, squareColor.a);
    result.a = mix(baseColor.a, 1.0, squareColor.a);
    return result;
}

// Blends the selected raster pattern over baseColor
vec4 applyRaster(vec4 baseColor, vec2 localPos)
{
    if(uRasterPattern == RASTER_PATTERN_CHECKERED)
        return applyCheckeredPattern(baseColor, localPos);

    return applyDottedPattern(baseColor, localPos);
}
