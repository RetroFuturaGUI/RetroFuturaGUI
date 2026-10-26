// One color across the whole rectangle. overBackground() comes from the class shader

uniform vec4 uColor;

// The fill's color at this pixel, over whatever shows through the fill
vec4 fillColor()
{
    return overBackground(uColor);
}
