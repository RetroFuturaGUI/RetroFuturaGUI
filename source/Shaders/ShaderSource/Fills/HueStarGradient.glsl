// Rays around the rectangle's center, turned by uDegree. Included by Gradient.glsl, which declares the uniforms
float gradientPosition()
{
    // Hue star: map colors by angle around center (Siemens star pattern)
    vec2 center = vec2(0.5);
    // take scale into account so gradient follows scaled rectangle used for rounded corners
    vec2 pos = (vTexCoord - center) * uScale;

    // Apply rotation in degrees (uDegree) to orient the star
    float rot = radians(uDegree);
    float c = cos(rot);
    float s = sin(rot);
    pos = vec2(c * pos.x - s * pos.y, s * pos.x + c * pos.y);

    // Compute angle in [0,1)
    const float PI = 3.14159265358979323846;
    float angle = atan(pos.y, pos.x); // [-PI, PI]
    return (angle + PI) / (2.0 * PI); // [0,1)
}
