#version 330 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform float uDegree;
uniform int uNumColors;
uniform vec4 uColors[255]; // 255 colors maximum. I don't think you'll need more than that
uniform float uGradientOffset;

void main() {
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