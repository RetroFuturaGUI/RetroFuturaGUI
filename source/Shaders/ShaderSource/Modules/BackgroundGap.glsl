#include "Common.glsl"
#include "PeriodicGap.glsl"

uniform vec4 uBackgroundGap; // (rotationDegrees, offsetPx, lengthPx, repeat)

/* Tests scaledPos (pixels, local to the rectangle, origin at center) against the configured
   background gap. A background is a single element rather than four separate edges, so one pattern
   covers all of it: it runs along X, rotated by uBackgroundGap.x degrees. The gap is defined in
   absolute pixels from the edge the pattern starts at, so it keeps a fixed size and a fixed
   distance from that edge regardless of how the rectangle is resized - unlike a UV-based test,
   which would stretch with the rectangle. */
bool isInBackgroundGap(vec2 scaledPos)
{
    float gapOffset = uBackgroundGap.y;
    float gapLength = uBackgroundGap.z;
    int repeat = int(uBackgroundGap.w);

    // Bail before reading uScale: with no gap configured it may not have been uploaded at all.
    if (repeat == 0 || gapLength <= 0.0)
        return false;

    float angleRad = radians(uBackgroundGap.x);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));
    vec2 halfSize = vec2(0.5) * uScale;

    /* The rectangle's own extent projected onto the axis. Adding it shifts the measuring origin off
       the center and onto the corner the pattern starts from, so d runs from 0 there across the
       whole rectangle - at 0 degrees this reduces to scaledPos.x + halfSize.x, the same
       left-to-right measurement the border's top/bottom edges use. */
    float maxProjection = abs(direction.x) * halfSize.x + abs(direction.y) * halfSize.y;

    return isInPeriodicGap(dot(scaledPos, direction) + maxProjection, gapOffset, gapLength, repeat);
}

