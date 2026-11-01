#include "Common.glsl"

uniform sampler2D uMask;
uniform int uMaskMode; //0: alpha, 1: shade, as MaskMode in C++
uniform vec2 uMaskCoverage; //how much of the area the mask spans per axis: 1 stretched, more when filling, less when fitting

//The position the mask is laid by, -0.5 .. 0.5 across the area; a class shader can lay it otherwise (Text: per line)
#ifndef MASK_POSITION
#define MASK_POSITION vLocalPos
#endif

// Applies a grayscale mask laid over what is drawn, its red channel the gray. As alpha it makes the color that see-through;
// as shade it darkens below mid-gray and lightens above it, so bright and dark bands turn a flat image into chrome. Where
// a fitted mask doesn't reach, the color stays as it is.
vec4 applyMask(vec4 color)
{
    vec2 maskUV = MASK_POSITION / uMaskCoverage + 0.5;

    if(any(lessThan(maskUV, vec2(0.0))) || any(greaterThan(maskUV, vec2(1.0))))
        return color;

    float gray = texture(uMask, maskUV).r;

    if(0 == uMaskMode)
        return vec4(color.rgb, color.a * gray);

    vec3 shaded = gray < 0.5 ? color.rgb * gray * 2.0 : mix(color.rgb, vec3(1.0), gray * 2.0 - 1.0);
    return vec4(shaded, color.a);
}
