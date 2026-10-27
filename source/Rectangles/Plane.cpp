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

void RetroFuturaGUI::Plane::SetBackgroundImageTextureID(const u32 textureID)
{
    _backgroundImageTextureID = textureID;

    if(0 != _backgroundImageTextureID)
        _shaderFeatureDIP |= ShaderFeatures::BackgroundImage;
    else
        _shaderFeatureDIP &= ~ShaderFeatures::BackgroundImage;
}

void RetroFuturaGUI::Plane::SetBackgroundImagePadding(const f32 padding)
{
    _backgroundImagePadding = padding;
}

void RetroFuturaGUI::Plane::uploadPlane(Shader& shader)
{
    shader.UseProgram();
    uploadTransform(shader);
    uploadFill(shader, getFogFeatures());
    shader.SetUniformVec4("uBackgroundGap", _backgroundGapData);

    if(0 != getFogFeatures())
        uploadFog(shader);

    if(_shaderFeatureDIP & ShaderFeatures::BackgroundImage)
        uploadImage(shader);
}

void RetroFuturaGUI::Plane::uploadImage(Shader& shader) const
{
    //Unit 1, apart from the glass. Unit 0 is made active again for whatever binds a texture next
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