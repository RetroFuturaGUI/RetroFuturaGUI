#version 330 core
uniform float uOpacity = 1.0; //Shader::OpacityScope's product, uploaded with every UseProgram

uniform vec4 uColor;
uniform float uLineThickness; // the strip's full width, in pixels
uniform float uGlowWidth = 0.0; // above 0, the strip is a glow around a line uCoreThickness wide, fading out over this many pixels
uniform float uCoreThickness; // the line's own width, in pixels, which a glow surrounds
uniform int uCoverageBand = 0; // 1: only the fully covered fragments, 2: only the faded ones, 0: both

in float gs_Distance;
out vec4 FragColor;

void main()
{
    // How much of the pixel the strip covers. A line: fully up to half a pixel inside its edge, not at all half a pixel outside it,
    // which Line.gs leaves room for. A glow: full up to the line's edge, then fading to nothing over its width, as the glows around
    // rectangles and text
    float coverage = uGlowWidth > 0.0
        ? 1.0 - smoothstep(0.0, uGlowWidth, abs(gs_Distance) - uCoreThickness * 0.5)
        : clamp(uLineThickness * 0.5 + 0.5 - abs(gs_Distance), 0.0, 1.0);

    // Nothing to draw, and it mustn't write depth either, or it would hide what is drawn behind it later
    if (coverage <= 0.0)
        discard;

    // Polyline draws the full fragments first, so where pieces of a turn overlap a pixel takes a full one over a faded edge
    bool isFull = coverage >= 1.0;

    if ((1 == uCoverageBand && !isFull) || (2 == uCoverageBand && isFull))
        discard;

    FragColor = vec4(uColor.rgb, uColor.a * coverage);
    FragColor.a *= uOpacity; //faded with the widget and whatever fades around it
}
