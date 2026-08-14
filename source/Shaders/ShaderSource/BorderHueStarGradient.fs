#version 330 core
#define ROUNDED_CORNERS 1
#define GLASS_EFFECT 2
#define GlassEffectWithImage 6
#define DOTTED_PATTERN 8
#define MAX_DOT_RADIUS_TRANSFER 255

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
uniform float uBorderWidth;
uniform vec4 uDotColor;
uniform float uDotDistance;
uniform float uDotSizeTransferDegree;
uniform float uDotTransparencyTransfer;
uniform float uDotAnimationOffset;
uniform int uDotRadiusTransferCount;
uniform float uDotRadiusTransfer[MAX_DOT_RADIUS_TRANSFER];

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

float roundedRectSDF(vec2 p, vec2 halfSize, float radius)
{
    vec2 d = abs(p) - halfSize + vec2(radius);
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius;
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

void main()
{
    vec4 finalColor;

    vec2 scaledPos = vLocalPos * uScale;
    vec2 halfSize = vec2(0.5) * uScale;
    vec2 innerHalfSize = halfSize - vec2(uBorderWidth);
    
    float cornerRadius = ((uDIP & ROUNDED_CORNERS) != 0) ? uCornerRadii.x : 0.0;
    float innerRadius = max(cornerRadius - uBorderWidth, 0.0);
    
    float outerDist = roundedRectSDF(scaledPos, halfSize, cornerRadius);
    float innerDist = roundedRectSDF(scaledPos, innerHalfSize, innerRadius);
    
    if (outerDist > 0.0 || innerDist <= 0.0)
    {
        discard;
    }
    else
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
        float gradient = (angle + PI) / (2.0 * PI); // [0,1)

        // Apply the angular offset (uGradientOffset interpreted as fraction of full turn) and wrap
        gradient += uGradientOffset;
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

        finalColor = mix(color1, color2, interpolation);
    }

    if((uDIP & DOTTED_PATTERN) != 0)
    {
        finalColor = applyDottedPattern(finalColor, vLocalPos);
    }

    fragColor = finalColor;
}