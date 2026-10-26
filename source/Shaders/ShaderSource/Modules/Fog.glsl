#include "Common.glsl"
#include "Noise.glsl"

#define MAX_FOG_DENSITY 8

uniform float uFogAlpha;
uniform float uFogAnimationOffset;
uniform float uFogClearing;
uniform int uFogDensityCount;
uniform float uFogDensity[MAX_FOG_DENSITY];

/* Same corner-sampled value noise as noise(), but with a quintic smoothstep interpolation
   curve (continuous 1st/2nd derivatives) instead of noise()'s linear one - this is what turns
   the fog's cloud cells from blocky/faceted into soft, thick blobs. Kept separate from noise()
   so the glass effect's cheaper interpolation elsewhere is unaffected. */
float fogNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

/* Fractal Brownian Motion: sums `fogNoise()` across uFogDensityCount octaves, each rotated and
   doubled in frequency, weighted by uFogDensity[i]. Rotating (not just scaling) the sample space
   between octaves keeps their grids from staying axis-aligned with each other - without this, a
   coarse octave's cell boundaries show through as a visible grid of soft square blobs instead of
   blending into an organic, irregular shape. */
float fogFbm(vec2 p)
{
    float value = 0.0;
    float weightSum = 0.0;
    int count = clamp(uFogDensityCount, 1, MAX_FOG_DENSITY);
    const mat2 rot = mat2(0.8, -0.6, 0.6, 0.8);
    vec2 offset = vec2(0.0);

    for(int i = 0; i < count; ++i)
    {
        float weight = max(uFogDensity[i], 0.0);
        value += weight * fogNoise(p + offset);
        weightSum += weight;
        p = rot * p * 2.0;
        offset += vec2(19.19, 7.71);
    }

    return weightSum > 0.0001 ? value / weightSum : 0.0;
}

/* Blends drifting, cloud-like fog over baseColor. uFogAnimationOffset (accumulated from speed
   each frame) drifts the sampled noise domain so the fog continuously moves. A narrow transition
   band around uFogClearing (rather than spanning the whole clearing..1.0 range) is what makes
   cluster interiors read as thick, near-opaque cloud mass with a defined edge instead of fading
   as a uniform low-contrast haze; raising uFogClearing still shrinks how much of the field clears
   that threshold, carving larger clear gaps out of the cloud. */
vec4 applyFog(vec4 baseColor, vec2 localPos)
{
    vec2 pixelPos = localPos * uScale;
    vec2 drift = vec2(uFogAnimationOffset, uFogAnimationOffset * 0.6);
    vec2 samplePos = (pixelPos + drift) / 170.0;

    float coverage = fogFbm(samplePos);

    // A mild stretch around the midpoint gives uFogClearing some real range to work with,
    // without pushing the field all the way to flat on/off patches.
    coverage = clamp((coverage - 0.5) * 1.5 + 0.5, 0.0, 1.0);

    // A wide transition band keeps the cloud/clear-sky boundary soft and feathered rather than
    // a hard, geometric edge.
    float threshold = clamp(uFogClearing, 0.0, 1.0);
    float band = 0.28;
    float density = smoothstep(threshold - band, threshold + band, coverage);

    // A second, finer noise sample shades the inside of the cloud mass so it isn't a flat,
    // single-opacity fill - real fog/cloud cover has soft brightness variation running through
    // it even where the coverage mask itself is fully "in".
    float detail = fogNoise((pixelPos + drift * 1.3) / 55.0);
    float shading = mix(0.55, 1.0, detail);

    float fogAlpha = density * shading * clamp(uFogAlpha, 0.0, 1.0);

    const vec3 fogColor = vec3(0.85, 0.88, 0.92);
    vec4 result = baseColor;
    result.rgb = mix(baseColor.rgb, fogColor, fogAlpha);
    result.a = mix(baseColor.a, 1.0, fogAlpha);
    return result;
}

