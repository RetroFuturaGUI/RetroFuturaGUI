#include "Common.glsl"

uniform vec4 uCornerRadii;

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

// Whether the pixel lies outside the rounded corner nearest to it
bool isOutsideRoundedCorners(vec2 localPos)
{
    vec2 scaledPos = localPos * uScale;
    vec2 halfSize = vec2(0.5) * uScale;

    float cornerRadius;
    if (scaledPos.x >= 0.0 && scaledPos.y >= 0.0)
    {
        cornerRadius = uCornerRadii.y;
    }
    else if (scaledPos.x < 0.0 && scaledPos.y >= 0.0)
    {
        cornerRadius = uCornerRadii.x;
    }
    else if (scaledPos.x < 0.0 && scaledPos.y < 0.0)
    {
        cornerRadius = uCornerRadii.w;
    }
    else
    {
        cornerRadius = uCornerRadii.z;
    }

    vec2 p = abs(scaledPos) - halfSize + vec2(cornerRadius);
    float dist = length(max(p, 0.0)) + min(max(p.x, p.y), 0.0) - cornerRadius;

    return dist > 0.0;
}
