#version 330 core
#define ROUNDED_CORNERS 1

layout(location = 0) out vec4 Color;
uniform vec4 uColor;
uniform vec4 uCornerRadii;
uniform vec2 uScale;
uniform int uDIP;

in vec2 vLocalPos;

void main()
{
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

    Color = uColor;
}