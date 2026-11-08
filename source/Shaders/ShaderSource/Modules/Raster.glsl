#include "Common.glsl"

#define RASTER_PATTERN_DOTTED 0
#define RASTER_PATTERN_CHECKERED 1
#define RASTER_PATTERN_STRIPED 2
#define RASTER_PATTERN_GRID 3
#define MAX_PRIMARY_RASTER_WIDTH_TRANSFER 255

uniform int uRasterPattern;
uniform vec4 uPrimaryRasterColor;
uniform vec4 uSecondaryRasterColor;
uniform float uRasterCellSize; // the largest width plus the spacing, in px: one grid cell of every pattern
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

/* Blends a dot-grid pattern over baseColor. Dot centers sit on a uRasterCellSize grid (in px, local
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

    float cellSize = max(uRasterCellSize, 0.0001);
    vec2 cellCenter = floor(animatedPos / cellSize + 0.5) * cellSize;
    vec2 localOffset = animatedPos - cellCenter;

    // A dot's own center decides which part of the transfer curve sizes it.
    float dotWidth = sampleRasterWidth(rasterTransferCoord(cellCenter, direction, halfSize));

    // The transfer curve gives each dot's width across, so the circle drawn from its center is half of it. The cell is
    // at least as wide as the largest dot, so a circle never reaches past its cell
    float dotRadius = dotWidth * 0.5;

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
   time via uRasterAnimationOffset just as the dots do. The squares sit on the same uRasterCellSize grid
   as the dots, each one centered in its cell and as wide as uPrimaryRasterWidthTransfer at its center,
   so a varying curve grows and shrinks the squares while the grid stays even. Between them the fill
   shows; at no spacing the largest squares touch. */
vec4 applyCheckeredPattern(vec4 baseColor, vec2 localPos)
{
    vec2 pixelPos = localPos * uScale;
    vec2 halfSize = uScale * 0.5;

    float angleRad = radians(uRasterDegree);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));
    vec2 crossDirection = vec2(-direction.y, direction.x);

    // Shift the sampled space along the direction to animate the whole board rigidly.
    vec2 animatedPos = pixelPos + direction * uRasterAnimationOffset;

    // Board coordinates: one unit per cell, so their integer boundaries are the cell edges
    float cellSize = max(uRasterCellSize, 0.0001);
    vec2 board = vec2(dot(animatedPos, direction), dot(animatedPos, crossDirection)) / cellSize;
    vec2 cellCenterBoard = floor(board) + 0.5;

    // A square's own center decides which part of the transfer curve sizes it, as a dot's does
    vec2 cellCenter = (direction * cellCenterBoard.x + crossDirection * cellCenterBoard.y) * cellSize;
    float squareSide = sampleRasterWidth(rasterTransferCoord(cellCenter, direction, halfSize));

    // How far this pixel lies outside its square, in px: negative inside. Smoothed over about a pixel
    vec2 centerOffset = abs(board - cellCenterBoard) * cellSize;
    float edgeDistance = max(centerOffset.x, centerOffset.y) - squareSide * 0.5;
    float edgeAA = max(fwidth(edgeDistance), 1e-4);
    float coverage = 1.0 - smoothstep(-edgeAA, edgeAA, edgeDistance);

    // Squares that touch would each fade at their shared edge and leave a seam of the fill, so a gap only
    // fades in as it widens past the smoothing
    coverage = mix(1.0, coverage, clamp((cellSize - squareSide) / (2.0 * edgeAA), 0.0, 1.0));

    /* sin() is positive across one cell of a row and negative across the next, so the product of
       both axes' sines carries the board's parity and changes sign exactly at every cell edge.
       That lets a single smoothstep over the sign change antialias where touching squares meet - and
       once cells shrink below a pixel it settles at the halfway mix of both colors rather than
       aliasing into moire. */
    float parity = sin(board.x * 3.14159265359) * sin(board.y * 3.14159265359);
    float parityAA = max(fwidth(parity), 1e-4);
    vec4 squareColor = mix(uSecondaryRasterColor, uPrimaryRasterColor, smoothstep(-parityAA, parityAA, parity));
    float squareAlpha = squareColor.a * coverage;

    vec4 result = baseColor;
    result.rgb = mix(baseColor.rgb, squareColor.rgb, squareAlpha);
    result.a = mix(baseColor.a, 1.0, squareAlpha);
    return result;
}

// How much of the pixel the line of its cell covers, along one axis of the raster's board (one unit per cell, a line
// centered in each). Smoothed over about a pixel; lines that touch leave no seam of the fill where they meet.
float rasterLineCoverage(float boardCoord, float cellSize, float lineWidth)
{
    float edgeDistance = abs(fract(boardCoord) - 0.5) * cellSize - lineWidth * 0.5; // px outside the line, negative inside.
    float edgeAA = max(fwidth(edgeDistance), 1e-4);
    float coverage = 1.0 - smoothstep(-edgeAA, edgeAA, edgeDistance);
    return mix(1.0, coverage, clamp((cellSize - lineWidth) / (2.0 * edgeAA), 0.0, 1.0));
}

// Lays a color over baseColor as far as alpha covers it.
vec4 blendRasterColor(vec4 baseColor, vec3 color, float alpha)
{
    vec4 result = baseColor;
    result.rgb = mix(baseColor.rgb, color, alpha);
    result.a = mix(baseColor.a, 1.0, alpha);
    return result;
}

/* Blends stripes over baseColor. They run across the uRasterDegree direction, one centered in every uRasterCellSize along
   it, and slide along it over time via uRasterAnimationOffset. Each is as wide as uPrimaryRasterWidthTransfer at its
   center, and they alternate between the primary and the secondary color. Between them the fill shows. */
vec4 applyStripedPattern(vec4 baseColor, vec2 localPos)
{
    vec2 pixelPos = localPos * uScale;
    vec2 halfSize = uScale * 0.5;

    float angleRad = radians(uRasterDegree);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));
    vec2 animatedPos = pixelPos + direction * uRasterAnimationOffset;

    float cellSize = max(uRasterCellSize, 0.0001);
    float along = dot(animatedPos, direction) / cellSize;
    vec2 stripeCenter = direction * (floor(along) + 0.5) * cellSize;
    float stripeWidth = sampleRasterWidth(rasterTransferCoord(stripeCenter, direction, halfSize));
    float coverage = rasterLineCoverage(along, cellSize, stripeWidth);

    // sin() is positive across one stripe's cell and negative across the next, so the colors alternate and blend
    // smoothly where touching stripes meet, as the checkered squares do.
    float parity = sin(along * 3.14159265359);
    float parityAA = max(fwidth(parity), 1e-4);
    vec4 stripeColor = mix(uSecondaryRasterColor, uPrimaryRasterColor, smoothstep(-parityAA, parityAA, parity));

    return blendRasterColor(baseColor, stripeColor.rgb, stripeColor.a * coverage);
}

/* Blends a grid of lines over baseColor: the stripes, and as many again turned a quarter, so the fill shows in the cells
   between them. The lines across the uRasterDegree direction take the primary color and are as wide as
   uPrimaryRasterWidthTransfer at their center; the lines along it take the secondary color, as wide as the transfer
   where they pass, so they narrow and widen with the lines across. Where they cross, the primary lines lie on top. */
vec4 applyGridPattern(vec4 baseColor, vec2 localPos)
{
    vec2 pixelPos = localPos * uScale;
    vec2 halfSize = uScale * 0.5;

    float angleRad = radians(uRasterDegree);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));
    vec2 crossDirection = vec2(-direction.y, direction.x);
    vec2 animatedPos = pixelPos + direction * uRasterAnimationOffset;

    float cellSize = max(uRasterCellSize, 0.0001);
    vec2 board = vec2(dot(animatedPos, direction), dot(animatedPos, crossDirection)) / cellSize;

    vec2 acrossCenter = direction * (floor(board.x) + 0.5) * cellSize;
    float acrossWidth = sampleRasterWidth(rasterTransferCoord(acrossCenter, direction, halfSize));
    float acrossCoverage = rasterLineCoverage(board.x, cellSize, acrossWidth);

    float alongWidth = sampleRasterWidth(rasterTransferCoord(animatedPos, direction, halfSize));
    float alongCoverage = rasterLineCoverage(board.y, cellSize, alongWidth);

    vec4 result = blendRasterColor(baseColor, uSecondaryRasterColor.rgb, uSecondaryRasterColor.a * alongCoverage);
    return blendRasterColor(result, uPrimaryRasterColor.rgb, uPrimaryRasterColor.a * acrossCoverage);
}

// Blends the selected raster pattern over baseColor
vec4 applyRaster(vec4 baseColor, vec2 localPos)
{
    if(uRasterPattern == RASTER_PATTERN_CHECKERED)
        return applyCheckeredPattern(baseColor, localPos);

    if(uRasterPattern == RASTER_PATTERN_STRIPED)
        return applyStripedPattern(baseColor, localPos);

    if(uRasterPattern == RASTER_PATTERN_GRID)
        return applyGridPattern(baseColor, localPos);

    return applyDottedPattern(baseColor, localPos);
}
