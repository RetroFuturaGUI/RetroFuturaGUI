#include "Common.glsl"

uniform sampler2D uBackgroundImage;
uniform vec2 uImagePadding;
uniform vec2 uImageCoverage = vec2(1.0); //how much of the inset box the image spans per axis: 1 stretched, more when filling, less when fitting

// Lays color over the plane's own image, which sits centered in the quad inset by uImagePadding, stretched to it or
// keeping its proportions. Outside the image color is returned as it is, making the padding read as empty space rather
// than a stretched edge pixel.
vec4 applyImage(vec4 color)
{
    vec2 span = vec2(1.0) - uImagePadding * 2.0;

    if(span.x <= 0.0 || span.y <= 0.0)
        return color;

    vec2 imageUV = ((vLocalPos + 0.5 - uImagePadding) / span - 0.5) / uImageCoverage + 0.5;

    if(any(lessThan(imageUV, vec2(0.0))) || any(greaterThan(imageUV, vec2(1.0))))
        return color;

    return mix(texture(uBackgroundImage, imageUV), color, color.a);
}
