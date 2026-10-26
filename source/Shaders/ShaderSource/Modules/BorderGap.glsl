#include "PeriodicGap.glsl"

#define MAX_BORDER_GAPS 255

uniform int uBorderGapCount;
uniform vec4 uBorderGaps[MAX_BORDER_GAPS]; // per gap: (edge + anchorFarCorner*4, offsetPx, lengthPx, repeat)

/* Tests scaledPos (pixels, local to the rectangle, origin at center) against the configured
   border gaps. Each gap is defined in absolute pixels relative to one corner of its edge, so it
   keeps a fixed size and a fixed distance from that corner regardless of how the rectangle is
   resized - unlike a UV-based test, which would stretch with the rectangle. */
bool isInBorderGap(vec2 scaledPos, vec2 halfSize, vec2 innerHalfSize)
{
    for (int i = 0; i < uBorderGapCount; ++i)
    {
        vec4 gap = uBorderGaps[i];
        int packedEdge = int(gap.x + 0.5); // edge in bits 0-1, anchorFarCorner in bit 2
        int edge = packedEdge & 3;
        bool anchorFar = (packedEdge & 4) != 0;
        float gapOffset = gap.y;
        float gapLength = gap.z;
        int repeat = int(gap.w);

        if (edge == 0 || edge == 2) // Top / Bottom: gap runs along X
        {
            bool inStripe = (edge == 0) ? (scaledPos.y > innerHalfSize.y) : (scaledPos.y < -innerHalfSize.y);
            if (!inStripe)
                continue;

            float d = anchorFar ? (halfSize.x - scaledPos.x) : (scaledPos.x + halfSize.x);
            if (isInPeriodicGap(d, gapOffset, gapLength, repeat))
                return true;
        }
        else // Left / Right: gap runs along Y
        {
            bool inStripe = (edge == 1) ? (scaledPos.x > innerHalfSize.x) : (scaledPos.x < -innerHalfSize.x);
            if (!inStripe)
                continue;

            float d = anchorFar ? (scaledPos.y + halfSize.y) : (halfSize.y - scaledPos.y);
            if (isInPeriodicGap(d, gapOffset, gapLength, repeat))
                return true;
        }
    }

    return false;
}

