#include "WavePlane.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::WavePlane::WavePlane(Projection* projection)
    : Plane(projection)
{
}

void RetroFuturaGUI::WavePlane::Draw()
{
    //Hidden, the wave costs nothing: the plain plane's shader has no wave code
    if(!(_shaderFeatureDIP & ShaderFeatures::Wave))
    {
        Plane::Draw();
        return;
    }

    if(_colors.empty())
        return;

    Shader& shader { ShaderManager::GetWavePlaneShader(_fillType) };
    uploadPlane(shader);
    shader.SetUniformFloat("uWaveThickness", _waveThickness);
    shader.SetUniformFloat("uWaveHeight", _waveHeight);
    shader.SetUniformFloat("uWaveLength", _waveLength);
    drawQuad();
}

void RetroFuturaGUI::WavePlane::ShowWave(const bool show)
{
    if(show)
        _shaderFeatureDIP |= ShaderFeatures::Wave;
    else
        _shaderFeatureDIP &= ~ShaderFeatures::Wave;
}

void RetroFuturaGUI::WavePlane::SetWaveThickness(const f32 thickness)
{
    _waveThickness = thickness;
}

void RetroFuturaGUI::WavePlane::SetWaveHeight(const f32 height)
{
    _waveHeight = height;
}

void RetroFuturaGUI::WavePlane::SetWaveLength(const f32 length)
{
    _waveLength = length;
}
