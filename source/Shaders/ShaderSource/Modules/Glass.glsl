#include "Common.glsl"
#include "Noise.glsl"

uniform sampler2D uBackgroundTexture;

// Blends the window-wide backdrop in uBackgroundTexture into baseColor. screenUV comes from clip space, so
// the quad shows the part of it sitting behind the quad, which is what sells the glass.
vec4 blendBackgroundImage(vec4 baseColor, vec2 screenUV)
{
    return mix(texture(uBackgroundTexture, screenUV), baseColor, baseColor.a);
}

// Where the glass shows the window behind: vUV with a small distortion
vec2 glassUV()
{
    return vUV + (noise(vUV * 10.0) - 0.5) * 0.02;
}

// Lays color over the glass at distortedUV
vec4 applyGlass(vec4 color, vec2 distortedUV)
{
    if((uDIP & GlassEffectWithImage) != 0)
        return blendBackgroundImage(color, distortedUV);

    float fakeBackground = noise(distortedUV * 5.0);
    return mix(vec4(vec3(fakeBackground), 1.0), color, color.a);
}
