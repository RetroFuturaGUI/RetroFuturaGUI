#include "Common.glsl"
#include "Noise.glsl"

uniform sampler2D uBackgroundTexture;

// Blends the window-wide backdrop in uBackgroundTexture into baseColor. screenUV comes from clip space, so
// the quad shows the part of it sitting behind the quad, which is what sells the glass.
vec4 blendBackgroundImage(vec4 baseColor, vec2 screenUV)
{
    return mix(texture(uBackgroundTexture, screenUV), baseColor, baseColor.a);
}

// GlassNoise's settings, in window pixels.
uniform float uGlassCellSize = 128.0;
uniform float uGlassDistortion = 12.0;
uniform float uGlassBackdropCellSize = 256.0;
uniform int uGlassSmoothing = 0;
uniform float uGlassSeed = 0.0;
uniform vec2 uWindowSize = vec2(1.0); // in pixels, the viewport the glass is drawn into.

// How far across the window one pixel reaches, to turn pixels into vUV's 0 .. 1.
vec2 windowUVPerPixel()
{
    return 1.0 / max(uWindowSize, vec2(1.0));
}

// Where the glass shows the window behind: vUV with a small distortion
vec2 glassUV()
{
    float shift = (noise(gl_FragCoord.xy / max(uGlassCellSize, 1.0), uGlassSmoothing, uGlassSeed) - 0.5) * 2.0 * uGlassDistortion;
    return vUV + shift * windowUVPerPixel();
}

// Lays color over the glass at distortedUV
vec4 applyGlass(vec4 color, vec2 distortedUV)
{
    if((uDIP & GlassEffectWithImage) != 0)
        return blendBackgroundImage(color, distortedUV);

    float fakeBackground = noise(distortedUV / windowUVPerPixel() / max(uGlassBackdropCellSize, 1.0), uGlassSmoothing, uGlassSeed);
    return mix(vec4(vec3(fakeBackground), 1.0), color, color.a);
}
