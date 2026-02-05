#version 330 core
#define ROUNDED_CORNERS 1
#define GLASS_EFFECT 2
#define GLASS_EFFECT_WITH_IMAGE 6

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
        // Radial gradient: distance from center normalized to [0,1]
        vec2 center = vec2(0.5);
        // take scale into account so gradient follows scaled rectangle used for rounded corners
        vec2 pos = (vTexCoord - center) * uScale;
        float dist = length(pos);
        float maxDist = length(center * uScale);
        float gradient = dist / maxDist;

        // Apply the offset to the gradient (makes colors "move") and wrap
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
            
            if((uDIP & GLASS_EFFECT_WITH_IMAGE) != 0)
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

    fragColor = finalColor;
}