#version 330 core
#define ROUNDED_CORNERS 1
#define GLASS_EFFECT 2
#define GlassEffectWithImage 6
#define DOTTED_PATTERN 8
#define MAX_DOT_RADIUS_TRANSFER 255
#define FOG_EFFECT 16
#define MAX_FOG_DENSITY 8

in vec2 vTexCoord;
out vec4 fragColor;
in vec2 vLocalPos;
in vec2 vUV;

uniform float uDegree;
uniform int uNumColors;
uniform vec4 uColors[255]; // 255 colors maximum. I don't think you'll need more than that
uniform float uGradientOffset;
uniform vec4 uCornerRadii;
uniform vec2 uScale;
uniform int uDIP;
uniform sampler2D uBackgroundTexture;
uniform vec4 uDotColor;
uniform float uDotDistance;
uniform float uDotSizeTransferDegree;
uniform float uDotTransparencyTransfer;
uniform float uDotAnimationOffset;
uniform int uDotRadiusTransferCount;
uniform float uDotRadiusTransfer[MAX_DOT_RADIUS_TRANSFER];
uniform float uFogAlpha;
uniform float uFogAnimationOffset;
uniform float uFogClearing;
uniform int uFogDensityCount;
uniform float uFogDensity[MAX_FOG_DENSITY];

float random(vec2 st)
{
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    // Four corners
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    
    // Linear interpolation (less smooth)
    vec2 u = f;
    // Smooth interpolation (smoothstep)
    //vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

/* Blends a dot-grid pattern over baseColor. Dot centers sit on a uDotDistance grid (in px, local
   to the rectangle) that slides along the uDotSizeTransferDegree direction over time via
   uDotAnimationOffset. Each dot's radius is sampled from uDotRadiusTransfer, indexed by that same
   dot's position projected onto the direction and normalized across the rectangle's extent -
   this is what lets dots grow/shrink smoothly from one side of the rectangle to the other.*/
vec4 applyDottedPattern(vec4 baseColor, vec2 localPos)
{
    vec2 pixelPos = localPos * uScale;
    vec2 halfSize = uScale * 0.5;

    float angleRad = radians(uDotSizeTransferDegree);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));

    // Shift the sampled space along the direction to animate the whole pattern rigidly.
    vec2 animatedPos = pixelPos + direction * uDotAnimationOffset;

    float dotDistance = max(uDotDistance, 0.0001);
    vec2 cellCenter = floor(animatedPos / dotDistance + 0.5) * dotDistance;
    vec2 localOffset = animatedPos - cellCenter;

    // Normalize this dot's projected position across the rectangle's extent to look up its radius.
    float maxProjection = abs(direction.x) * halfSize.x + abs(direction.y) * halfSize.y;
    // fract (not clamp) so the transfer curve repeats seamlessly as the pattern scrolls forever,
    // instead of freezing once a dot's projected position passes the rectangle's original extent.
    float t = maxProjection > 0.0001 ? fract(dot(cellCenter, direction) / (2.0 * maxProjection) + 0.5) : 0.0;

    int count = max(uDotRadiusTransferCount, 1);
    float radiusIndexF = t * float(count - 1);
    int idx0 = clamp(int(floor(radiusIndexF)), 0, count - 1);
    int idx1 = clamp(idx0 + 1, 0, count - 1);
    float dotRadius = mix(uDotRadiusTransfer[idx0], uDotRadiusTransfer[idx1], count > 1 ? fract(radiusIndexF) : 0.0);
    // Clamp to half the spacing so a dot's circle never reaches its Voronoi cell's square boundary
    // (which would otherwise make oversized dots look like squares/diamonds instead of circles).
    dotRadius = min(dotRadius, dotDistance * 0.5);

    float normalizedDist = dotRadius > 0.0001 ? length(localOffset) / dotRadius : 1.0;

    // transparencyTransfer fraction of the radius is fully opaque; the rest fades to transparent.
    // Widen the transition band to at least ~1 screen pixel (via screen-space derivatives) so small
    // or hard-edged (transparencyTransfer near 1.0) dots don't alias into blocky/jagged shapes.
    float innerBound = clamp(uDotTransparencyTransfer, 0.0, 1.0) * (1.0 - 1e-4);
    float aa = max(fwidth(normalizedDist), 1e-4);
    innerBound = min(innerBound, 1.0 - aa);
    float dotAlpha = (1.0 - smoothstep(innerBound, 1.0, normalizedDist)) * uDotColor.a;

    vec4 result = baseColor;
    result.rgb = mix(baseColor.rgb, uDotColor.rgb, dotAlpha);
    result.a = mix(baseColor.a, 1.0, dotAlpha);
    return result;
}

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

void main()
{
    // covert degree to direction vector
    float angleRad = radians(uDegree);
    vec2 direction = vec2(cos(angleRad), sin(angleRad));

    // Compute gradient value (dot product with direction = projection range -0.5 to 0.5 per color)
    float gradient = dot(vTexCoord - 0.5 /*centers projection around origin 0,0*/, direction);

    // Normalize gradient to the range of [0.0, 1.0]
    gradient = (gradient + 0.5) * 0.5;

    // Apply the offset to the gradient (makes colors "move")
    gradient += uGradientOffset;

    // get the fractional part to create a looping effect
    gradient = fract(gradient);
    gradient = clamp(gradient, 0.0, 1.0);

    // Calculate smooth color index and interpolation factor
    float colorIndex = gradient * float(uNumColors);

    // Ensure idx1 is always within bounds
    int idx1 = min(int(floor(colorIndex)), uNumColors - 1);

    // For idx2, wrap around to 0 if it exceeds uNumColors - 1
    int idx2 = (idx1 + 1) % uNumColors;
    float interpolation = fract(colorIndex);

    // Interpolate between the two closest colors
    vec4 color1 = uColors[idx1];
    vec4 color2 = uColors[idx2];

    if((uDIP & GLASS_EFFECT) != 0)
    {
        // Apply glass distortion
        vec2 distortedUV = vUV + (noise(vUV * 10.0) - 0.5) * 0.02; // Small distortion
        
        if((uDIP & GlassEffectWithImage) != 0)
        {
            vec4 background = texture(uBackgroundTexture, distortedUV);
            color1 = mix(background, color1, color1.a);
            color2 = mix(background, color2, color2.a);
        }
        else
        {
            float fakeBackground = noise(distortedUV * 5.0);
            color1 = mix(vec4(vec3(fakeBackground), 1.0), color1, color1.a);
            color2 = mix(vec4(vec3(fakeBackground), 1.0), color2, color2.a);
        }
    }

    vec4 finalColor = mix(color1, color2, interpolation);

    if((uDIP & DOTTED_PATTERN) != 0)
    {
        finalColor = applyDottedPattern(finalColor, vLocalPos);
    }

    if((uDIP & FOG_EFFECT) != 0)
    {
        finalColor = applyFog(finalColor, vLocalPos);
    }

    if((uDIP & ROUNDED_CORNERS) != 0)
    {
        vec2 scaledPos = vLocalPos * uScale;
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

        if (dist > 0.0)
            discard;
    }

    fragColor = finalColor;
}