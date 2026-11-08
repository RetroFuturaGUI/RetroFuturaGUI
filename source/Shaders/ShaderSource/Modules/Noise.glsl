// Value noise, for the glass and the fog

float random(vec2 st)
{
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}

// The noise at p, in cells. smoothing picks how it blends between the corners of a cell, as NoiseSmoothing does (0 linear,
// 1 cubic, 2 quintic). Another seed reads another stretch of the noise, far enough away not to repeat it.
float noise(vec2 p, int smoothing, float seed)
{
    p += mod(vec2(seed * 37.0, seed * 17.0), 1024.0);
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f;

    if (smoothing == 1)
        u = f * f * (3.0 - 2.0 * f);
    else if (smoothing == 2)
        u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0); // continuous in its first and second derivatives: soft, round blobs.

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}
