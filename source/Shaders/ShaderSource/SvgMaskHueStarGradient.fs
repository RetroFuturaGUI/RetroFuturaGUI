#version 330 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform float uDegree;
uniform int uNumColors;
uniform vec4 uColors[255]; // 255 colors maximum. I don't think you'll need more than that
uniform float uGradientOffset;
uniform sampler2D uMask;

void main()
{
    // Hue star: map colors by angle around center (Siemens star pattern)
    vec2 center = vec2(0.5);
    vec2 pos = vTexCoord - center;

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

    fragColor = mix(color1, color2, interpolation);
    fragColor.a *= texture(uMask, vTexCoord).r;
}
