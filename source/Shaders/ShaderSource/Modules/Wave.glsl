#include "Common.glsl"

uniform float uWaveThickness;
uniform float uWaveHeight;
uniform float uWaveLength;

/* Signed-distance-style test for a horizontal sine wave running through the rectangle's vertical
   center: y = sin(x / uWaveLength * TAU) * uWaveHeight. A pixel counts as outside the line once
   its vertical distance to the curve clears half of uWaveThickness; fwidth widens that boundary
   by about a screen pixel so the discard edge doesn't alias into jagged steps. */
bool isOutsideWaveLine(vec2 localPos)
{
    vec2 pixelPos = localPos * uScale;
    float waveLength = max(uWaveLength, 1.0);
    float waveY = sin(pixelPos.x / waveLength * 6.28318530718) * uWaveHeight;
    float dist = abs(pixelPos.y - waveY);
    float halfThickness = max(uWaveThickness, 0.0) * 0.5;
    return dist > halfThickness + max(fwidth(dist), 1e-4);
}

