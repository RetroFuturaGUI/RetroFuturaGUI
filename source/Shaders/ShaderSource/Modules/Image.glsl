#include "Common.glsl"

uniform sampler2D uBackgroundImage;
uniform vec2 uImagePadding;

// Lays color over the plane's own image, which is stretched across the quad and inset by uImagePadding. Outside that
// inset box color is returned as it is, making the padding read as empty space rather than a stretched edge pixel.
vec4 applyImage(vec4 color)
{
    vec2 span = vec2(1.0) - uImagePadding * 2.0;

    if(span.x <= 0.0 || span.y <= 0.0)
        return color;

    vec2 imageUV = (vLocalPos + 0.5 - uImagePadding) / span;

    if(any(lessThan(imageUV, vec2(0.0))) || any(greaterThan(imageUV, vec2(1.0))))
        return color;

    return mix(texture(uBackgroundImage, imageUV), color, color.a);
}
