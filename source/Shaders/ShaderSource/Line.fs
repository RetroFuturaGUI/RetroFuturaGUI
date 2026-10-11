#version 330 core

uniform vec4 uColor;
uniform float uLineThickness; // the line's full width, in pixels

in float gs_Distance;
out vec4 FragColor;

void main()
{
    // Fully covered up to half a pixel inside the edge, not at all half a pixel outside it - Line.gs leaves room for that fade
    float coverage = clamp(uLineThickness * 0.5 + 0.5 - abs(gs_Distance), 0.0, 1.0);

    // Nothing to draw, and it mustn't write depth either, or it would hide what is drawn behind it later
    if (coverage <= 0.0)
        discard;

    FragColor = vec4(uColor.rgb, uColor.a * coverage);
}
