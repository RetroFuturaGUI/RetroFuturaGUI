#include "Common.glsl"

uniform vec4 uCornerRadii;
uniform int uCutCorners; // one bit per corner in uCornerRadii's order (1 top-left, 2 top-right, 4 bottom-right, 8 bottom-left): cut off straight instead of rounded

float roundedRectSDF(vec2 p, vec2 halfSize, float radius)
{
    vec2 d = abs(p) - halfSize + vec2(radius);
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius;
}

// The radius of the corner nearest to p, which is centered on the rectangle with y up. uCornerRadii runs clockwise from the top-left
float cornerRadiusAt(vec2 p)
{
    if (p.x >= 0.0)
        return p.y >= 0.0 ? uCornerRadii.y : uCornerRadii.z;

    return p.y >= 0.0 ? uCornerRadii.x : uCornerRadii.w;
}

// A rectangle whose corners are cut off straight, cut long along each edge, as a signed distance: the box's own, or the cut's
// line where that lies further out. Only the corner of p's quadrant is cut, since abs mirrors p into it
float cutRectSDF(vec2 p, vec2 halfSize, float cut)
{
    vec2 q = abs(p);
    vec2 d = q - halfSize;
    float boxDist = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
    float cutDist = (q.x + q.y - (halfSize.x + halfSize.y - cut)) * 0.70710678;
    return max(boxDist, cutDist);
}

// Whether the corner nearest to p is cut rather than rounded
bool isCutCornerAt(vec2 p)
{
    int corner;
    if (p.x >= 0.0)
        corner = p.y >= 0.0 ? 2 : 4;
    else
        corner = p.y >= 0.0 ? 1 : 8;

    return (uCutCorners & corner) != 0;
}

// The signed distance to the rectangle with the corner nearest to p rounded or cut by radius
float cornerShapeSDF(vec2 p, vec2 halfSize, float radius)
{
    return isCutCornerAt(p) ? cutRectSDF(p, halfSize, radius) : roundedRectSDF(p, halfSize, radius);
}

// How much shorter the cut of a corner is on an outline's inner edge, per pixel of its width, so the inner cut runs
// parallel to the outer one: 2 - sqrt(2)
const float CUT_INSET_PER_WIDTH = 0.58578644;

// How much of the pixel a shape covers, from the signed distance to its edge (negative inside): it fades over one screen
// pixel, so edges come out smooth at any angle, tilt or scale. Call it outside branches that differ between pixels, since
// the fade measures how fast the distance changes from one pixel to the next.
float edgeCoverage(float distance)
{
    return clamp(0.5 - distance / max(fwidth(distance), 1e-4), 0.0, 1.0);
}

// How much of the pixel the rectangle covers, with its corners rounded or cut.
float shapeCoverage(vec2 localPos)
{
    vec2 scaledPos = localPos * uScale;
    float cornerRadius = ((uDIP & ROUNDED_CORNERS) != 0) ? cornerRadiusAt(scaledPos) : 0.0;
    return edgeCoverage(cornerShapeSDF(scaledPos, vec2(0.5) * uScale, cornerRadius));
}
