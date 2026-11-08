#include "FogStyle.hpp"
#include "Shader.hpp"
#include <algorithm>

void RetroFuturaGUI::FogStyle::SetFogAlpha(const f32 alpha)
{
    _fogAlpha = alpha;
}

void RetroFuturaGUI::FogStyle::SetFogSpeed(const f32 speed)
{
    _fogSpeed = speed;
}

void RetroFuturaGUI::FogStyle::SetFogDensity(std::span<f32> density)
{
    _fogDensity = density;
    _fogDensityCount = std::min<uSize>(_fogDensity.size(), 8);
}

void RetroFuturaGUI::FogStyle::SetFogClearing(const f32 clearing)
{
    _fogClearing = clearing;
}

void RetroFuturaGUI::FogStyle::SetFogNoise(const FogNoise& noise)
{
    _fogNoise = noise;
}

u32 RetroFuturaGUI::FogStyle::getFogFeatures() const
{
    return 0 < _fogDensityCount ? ShaderFeatures::FogEffect : 0;
}

void RetroFuturaGUI::FogStyle::uploadFog(Shader& shader)
{
    _fogAnimationOffset += _fogSpeed;

    shader.SetUniformFloat("uFogAlpha", _fogAlpha);
    shader.SetUniformFloat("uFogAnimationOffset", _fogAnimationOffset);
    shader.SetUniformFloat("uFogClearing", _fogClearing);
    shader.SetUniformInt("uFogDensityCount", static_cast<i32>(_fogDensityCount));
    shader.SetUniformFloat("uFogCellSize", _fogNoise._CellSize);
    shader.SetUniformFloat("uFogDetailCellSize", _fogNoise._DetailCellSize);
    shader.SetUniformInt("uFogSmoothing", static_cast<i32>(_fogNoise._Smoothing));
    shader.SetUniformFloat("uFogSeed", static_cast<f32>(_fogNoise._Seed % 1024u)); //the shader reads seeds 1024 apart as the same.

    if(0 < _fogDensityCount)
        shader.SetUniformFloat("uFogDensity", _fogDensity.data(), static_cast<u32>(_fogDensityCount));
}