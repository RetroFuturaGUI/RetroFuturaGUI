#version 330 core
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 7) out;

uniform mat4 uProjection;
uniform float uLineThickness; // the line's full width, in pixels

in vec4 vs_WorldPosition[];
out float gs_Distance; // signed distance from the line's center, in pixels - Line.fs fades the edge with it

// Each primitive is one segment, from vs_WorldPosition[1] to [2]. [0] and [3] are the points before and after it, which decide how
// its ends are cut so that consecutive segments meet without a gap. Polyline repeats the first and last point, so at the ends of the
// line the neighbour coincides and the end is cut square.

const float kFeather = 1.0;       // pixels added on each side, for the anti-aliased edge to fade out in
const float kMiterLimit = 0.75;   // turns sharper than this (the cosine between the segments below -kMiterLimit) get a bevel instead, as a miter would spike far out
const float kEpsilon = 0.0001;

void emit(vec2 position, float z, float distance)
{
    gs_Distance = distance;
    gl_Position = uProjection * vec4(position, z, 1.0);
    EmitVertex();
}

// The direction from one point to the next, or fallback where they coincide
vec2 directionOr(vec2 from, vec2 to, vec2 fallback)
{
    vec2 direction = to - from;
    float directionLength = length(direction);
    return directionLength > kEpsilon ? direction / directionLength : fallback;
}

void main()
{
    vec2
        p0 = vs_WorldPosition[0].xy,
        p1 = vs_WorldPosition[1].xy,
        p2 = vs_WorldPosition[2].xy,
        p3 = vs_WorldPosition[3].xy;
    float
        z1 = vs_WorldPosition[1].z,
        z2 = vs_WorldPosition[2].z;

    if (length(p2 - p1) < kEpsilon)
        return;

    vec2
        v1 = normalize(p2 - p1),
        v0 = directionOr(p0, p1, v1),
        v2 = directionOr(p2, p3, v1),
        n0 = vec2(-v0.y, v0.x),
        n1 = vec2(-v1.y, v1.x),
        n2 = vec2(-v2.y, v2.x);
    float halfWidth = uLineThickness * 0.5 + kFeather;

    // Start: the miter bisects the turn from the previous segment, lengthened so the edges stay halfWidth from the center
    vec2 miterStart = n1;
    float miterStartLength = halfWidth;

    if (dot(v0, v1) >= -kMiterLimit)
    {
        miterStart = normalize(n0 + n1);
        miterStartLength = halfWidth / dot(miterStart, n1);
    }
    else
    {
        // Too sharp to miter: cut square, and fill the gap on the outer side of the turn with a bevel. The previous segment cut its
        // end square as well (see the end below), so the bevel spans from its corner to this one's.
        float outerSide = dot(v0, n1) > 0.0 ? 1.0 : -1.0;
        emit(p1 + outerSide * halfWidth * n0, z1, outerSide * halfWidth);
        emit(p1 + outerSide * halfWidth * n1, z1, outerSide * halfWidth);
        emit(p1, z1, 0.0);
        EndPrimitive();
    }

    // End: the same towards the next segment, which draws the bevel there if the turn is too sharp
    vec2 miterEnd = n1;
    float miterEndLength = halfWidth;

    if (dot(v1, v2) >= -kMiterLimit)
    {
        miterEnd = normalize(n1 + n2);
        miterEndLength = halfWidth / dot(miterEnd, n1);
    }

    emit(p1 + miterStartLength * miterStart, z1, halfWidth);
    emit(p1 - miterStartLength * miterStart, z1, -halfWidth);
    emit(p2 + miterEndLength * miterEnd, z2, halfWidth);
    emit(p2 - miterEndLength * miterEnd, z2, -halfWidth);
    EndPrimitive();
}
