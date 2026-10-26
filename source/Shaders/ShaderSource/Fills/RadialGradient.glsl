// Rings around the rectangle's center. Included by Gradient.glsl, which declares the uniforms
float gradientPosition()
{
    // Radial gradient: distance from center normalized to [0,1]
    vec2 center = vec2(0.5);
    // take scale into account so gradient follows scaled rectangle used for rounded corners
    vec2 pos = (vTexCoord - center) * uScale;
    float dist = length(pos);
    float maxDist = length(center * uScale);
    return dist / maxDist;
}
