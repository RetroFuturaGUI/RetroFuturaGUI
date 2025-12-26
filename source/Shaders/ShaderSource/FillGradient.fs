#version 330 core
#define ROUNDED_CORNERS 1
#define GLASS_EFFECT 2

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
        vec4 background = texture(uBackgroundTexture, distortedUV);
        color1 = mix(background, color1, color1.a);
        color2 = mix(background, color2, color2.a);
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

    fragColor = mix(color1, color2, interpolation);
}

/*  Usage:
    _shader->UseProgram();

    i32 locColors = glGetUniformLocation(_shader->GetProgramId(), "uColors");
    glUniform4fv(locColors, 255, &colors[0][0]);

    i32 locDegree = glGetUniformLocation(_shader->GetProgramId(), "uDegree");
    glUniform1f(locDegree, degree);

    i32 locNum = glGetUniformLocation(_shader->GetProgramId(), "uNumColors");
    glUniform1i(locNum, sizeof(colors) / sizeof(glm::vec4));


    // Update the gradient and degree in a loop for animation

    gradientOffset += animationSpeed;
    if (gradientOffset > 1.0f)
        gradientOffset = 0.0f;

    degree += 0.02f;
    if(degree >= 360.0f)
        degree = 0.0f;

    // Set the offset uniform
    u32 shaderID = _shader->GetProgramId();
    glUniform1f(glGetUniformLocation(shaderID, "uGradientOffset"), gradientOffset);
    glUniform1f(glGetUniformLocation(shaderID, "uDegree"), degree);
*/