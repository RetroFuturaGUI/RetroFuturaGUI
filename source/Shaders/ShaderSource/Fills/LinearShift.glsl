// Stripes running across the rectangle at uDegree. Included by Gradient.glsl, which declares the uniforms
float gradientPosition()
{
    // convert degree to direction vector
    float angleRad = radians(uDegree);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));

    // Compute gradient value (dot product with direction = projection range -0.5 to 0.5 per color)
    float gradient = dot(vTexCoord - 0.5 /*centers projection around origin 0,0*/, direction);

    // Normalize gradient to the range of [0.0, 1.0]
    return (gradient + 0.5) * 0.5;
}
