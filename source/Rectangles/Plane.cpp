#include "Plane.hpp"
#include "ShaderManager.hpp"
#include <algorithm>

RetroFuturaGUI::Plane::Plane(Projection* projection)
    : IRectangle(projection)
{
}

void RetroFuturaGUI::Plane::Draw()
{
    if(_colors.empty())
        return;

    Shader& shader { ShaderManager::GetPlaneShader(_fillType) };
    uploadPlane(shader);
    drawQuad();
}

void RetroFuturaGUI::Plane::SetBackgroundGaps(const BackgroundGap& gaps)
{
    _backgroundGapData = glm::vec4(
        gaps._Degree,
        gaps._Offset,
        gaps._Length,
        static_cast<f32>(gaps._Repeat)
    );
}

void RetroFuturaGUI::Plane::SetFogAlpha(const f32 alpha)
{
    _fogAlpha = alpha;
}

void RetroFuturaGUI::Plane::SetFogSpeed(const f32 speed)
{
    _fogSpeed = speed;
}

void RetroFuturaGUI::Plane::SetFogDensity(std::span<f32> density)
{
    _fogDensity = density;
    _fogDensityCount = std::min<uSize>(_fogDensity.size(), 8);

    if(0 < _fogDensityCount)
        _shaderFeatureDIP |= ShaderFeatures::FogEffect;
    else
        _shaderFeatureDIP &= ~ShaderFeatures::FogEffect;
}

void RetroFuturaGUI::Plane::SetFogClearing(const f32 clearing)
{
    _fogClearing = clearing;
}

void RetroFuturaGUI::Plane::SetBackgroundImageTextureID(const u32 textureID)
{
    _shaderFeatureDIP |= ShaderFeatures::BackgroundImage;
    _backgroundImageTextureID = textureID;
}

void RetroFuturaGUI::Plane::SetBackgroundImagePadding(const f32 padding)
{
    _backgroundImagePadding = padding;
}

void RetroFuturaGUI::Plane::uploadPlane(Shader& shader)
{
    shader.UseProgram();
    uploadTransform(shader);
    uploadFill(shader);
    shader.SetUniformVec4("uBackgroundGap", _backgroundGapData);

    if(_shaderFeatureDIP & ShaderFeatures::FogEffect)
        uploadFog(shader);

    if(_shaderFeatureDIP & ShaderFeatures::BackgroundImage)
        uploadImage(shader);
}

void RetroFuturaGUI::Plane::uploadFog(Shader& shader)
{
    _fogAnimationOffset += _fogSpeed;

    shader.SetUniformFloat("uFogAlpha", _fogAlpha);
    shader.SetUniformFloat("uFogAnimationOffset", _fogAnimationOffset);
    shader.SetUniformFloat("uFogClearing", _fogClearing);
    shader.SetUniformInt("uFogDensityCount", static_cast<i32>(_fogDensityCount));

    if(0 < _fogDensityCount)
        shader.SetUniformFloat("uFogDensity", _fogDensity.data(), static_cast<u32>(_fogDensityCount));
}

void RetroFuturaGUI::Plane::uploadImage(Shader& shader) const
{
    //Unit 1, so the glass keeps unit 0. Unit 0 is made active again for whatever binds a texture next
    shader.SetUniformInt("uBackgroundImage", 1);
    shader.SetUniformVec2("uImagePadding", getBackgroundImagePaddingUV());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _backgroundImageTextureID);
    glActiveTexture(GL_TEXTURE0);
}

glm::vec2 RetroFuturaGUI::Plane::getBackgroundImagePaddingUV() const
{
    // image in UV space, so the pixel padding is expressed as a fraction of each axis.
    if(0.0f >= _scale.x || 0.0f >= _scale.y)
        return glm::vec2(0.0f);

    return glm::vec2(_backgroundImagePadding / _scale.x, _backgroundImagePadding / _scale.y);
}
