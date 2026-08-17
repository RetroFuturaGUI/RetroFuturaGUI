#include "Rectangle.hpp"
#include <algorithm>

RetroFuturaGUI::Rectangle::Rectangle(Projection* projection)
    : _projection(*projection)
{
    setupMesh();
}

RetroFuturaGUI::Rectangle::~Rectangle()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void RetroFuturaGUI::Rectangle::Draw()
{
    if(_colors.size() == 0)
        return;

    switch(_rectangleMode)
    {
        case RectangleMode::Border:
        {
            switch(_fillType)
            {
                case FillType::LINEAR_GRADIENT:
                {
                    drawLinearGradientBorder();
                } break;
                case FillType::RADIAL_GRADIENT:
                {
                    drawRadialGradientBorder();
                } break;
                case FillType::HUESTAR_GRADIENT:
                {
                    drawHueStarGradientBorder();
                } break;
                default: // FillType::SOLID
                {
                    drawSolidBorder();
                } break;
            }
        } break;
        default: // RectangleMode::Plane
        {
            switch(_fillType)
            {
                case FillType::LINEAR_GRADIENT:
                {
                    drawLinearGradientFill();
                } break;
                case FillType::RADIAL_GRADIENT:
                {
                    drawRadialGradientFill();
                } break;
                case FillType::HUESTAR_GRADIENT:
                {
                    drawHueStarGradientFill();
                } break;
                default: // FillType::SOLID
                {
                    drawWithSolidFill();
                } break;
            }
        }
    }

    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void RetroFuturaGUI::Rectangle::SetSize(const glm::vec2& size)
{
    _scale = glm::vec2(size.x, size.y);
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(_scale, 1.0f));
}

void RetroFuturaGUI::Rectangle::SetPosition(const glm::vec3& position)
{
    _position = position;
    _translationMatrix = glm::translate(glm::mat4(1.0f), _position);
}

void RetroFuturaGUI::Rectangle::SetColors(std::span<glm::vec4> colors)
{
    initColors(colors);
}

void RetroFuturaGUI::Rectangle::SetCornerRadii(const glm::vec4& radii)
{
    _cornerRadii = radii;

    for(u32 i = 0; i < 4; ++i)
    {
        if(_cornerRadii[i] > 0.0f)
        {
            _shaderFeatureDIP |= ShaderFeatures::RoundedCorners;
            return;
        }
    }

    _shaderFeatureDIP &= ~ShaderFeatures::RoundedCorners;
}

void RetroFuturaGUI::Rectangle::SetGradientOffset(const f32 gradientOffset)
{
    _gradientOffset = gradientOffset;
}

void RetroFuturaGUI::Rectangle::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _gradientAnimationSpeed = animationSpeed;
}

void RetroFuturaGUI::Rectangle::SetGradientDegree(const f32 degree)
{
    _gradientDegree = degree;
}

void RetroFuturaGUI::Rectangle::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _gradientRotationSpeed = rotationSpeed;
}

void RetroFuturaGUI::Rectangle::SetDotColor(const glm::vec4& color)
{
    _dotColor = color;
}

void RetroFuturaGUI::Rectangle::SetDotDistance(const f32 distance)
{
    _dotDistance = distance;
}

void RetroFuturaGUI::Rectangle::SetDotSizeTransferDegree(const f32 degree)
{
    _dotSizeTransferDegree = degree;
}

void RetroFuturaGUI::Rectangle::SetDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    _dotRadiusTransfer = radiusTransfer;
    _dotRadiusTransferCount = static_cast<i32>(std::min<size_t>(_dotRadiusTransfer.size(), 255));

    if(_dotRadiusTransferCount > 0)
        _shaderFeatureDIP |= ShaderFeatures::DottedPattern;
    else
        _shaderFeatureDIP &= ~ShaderFeatures::DottedPattern;
}

void RetroFuturaGUI::Rectangle::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    _dotTransparencyTransfer = transparencyTransfer;
}

void RetroFuturaGUI::Rectangle::SetDotAnimationSpeed(const f32 animationSpeed)
{
    _dotAnimationSpeed = animationSpeed;
}

void RetroFuturaGUI::Rectangle::SetFogAlpha(const f32 alpha)
{
    _fogAlpha = alpha;
}

void RetroFuturaGUI::Rectangle::SetFogSpeed(const f32 speed)
{
    _fogSpeed = speed;
}

void RetroFuturaGUI::Rectangle::SetFogDensity(std::span<f32> density)
{
    _fogDensity = density;
    _fogDensityCount = static_cast<i32>(std::min<size_t>(_fogDensity.size(), 8));

    if(_fogDensityCount > 0)
        _shaderFeatureDIP |= ShaderFeatures::FogEffect;
    else
        _shaderFeatureDIP &= ~ShaderFeatures::FogEffect;
}

void RetroFuturaGUI::Rectangle::SetFogClearing(const f32 clearing)
{
    _fogClearing = clearing;
}

void RetroFuturaGUI::Rectangle::SetShaderFeatures(const u32 features, const bool reset)
{
    if(reset)
        _shaderFeatureDIP = features;
    else
        _shaderFeatureDIP |= features;
}

void RetroFuturaGUI::Rectangle::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    _shaderFeatureDIP |= ShaderFeatures::GlassEffectWithImage;
    _windowBackgroundTextureID = textureID;
}

void RetroFuturaGUI::Rectangle::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;
    glm::vec3 radians = glm::radians(_rotation);
    _rotationMatrix =
        glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
}

void RetroFuturaGUI::Rectangle::setupMesh()
{
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), nullptr);

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices), _indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void RetroFuturaGUI::Rectangle::initColors(std::span<glm::vec4> colors)
{
    _colors = colors;
    _colorCount = static_cast<i32>(_colors.size());
}

void RetroFuturaGUI::Rectangle::uploadDotUniforms(Shader& shader)
{
    _dotAnimationOffset += _dotAnimationSpeed;

    shader.SetUniformVec4("uDotColor", _dotColor);
    shader.SetUniformFloat("uDotDistance", _dotDistance);
    shader.SetUniformFloat("uDotSizeTransferDegree", _dotSizeTransferDegree);
    shader.SetUniformFloat("uDotTransparencyTransfer", _dotTransparencyTransfer);
    shader.SetUniformFloat("uDotAnimationOffset", _dotAnimationOffset);
    shader.SetUniformInt("uDotRadiusTransferCount", _dotRadiusTransferCount);

    if(_dotRadiusTransferCount > 0)
        shader.SetUniformFloat("uDotRadiusTransfer", _dotRadiusTransfer.data(), static_cast<u32>(_dotRadiusTransferCount));
}

void RetroFuturaGUI::Rectangle::uploadFogUniforms(Shader& shader)
{
    _fogAnimationOffset += _fogSpeed;

    shader.SetUniformFloat("uFogAlpha", _fogAlpha);
    shader.SetUniformFloat("uFogAnimationOffset", _fogAnimationOffset);
    shader.SetUniformFloat("uFogClearing", _fogClearing);
    shader.SetUniformInt("uFogDensityCount", _fogDensityCount);

    if(_fogDensityCount > 0)
        shader.SetUniformFloat("uFogDensity", _fogDensity.data(), static_cast<u32>(_fogDensityCount));
}

void RetroFuturaGUI::Rectangle::drawWithSolidFill()
{
    ShaderManager::GetSolidFillShader().UseProgram();
    ShaderManager::GetSolidFillShader().SetUniformInt("uDIP", _shaderFeatureDIP);
    ShaderManager::GetSolidFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetSolidFillShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetSolidFillShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetSolidFillShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetSolidFillShader().SetUniformVec4("uColor", _colors[0]);

    if(_shaderFeatureDIP & ShaderFeatures::RoundedCorners)
        ShaderManager::GetSolidFillShader().SetUniformVec4("uCornerRadii", _cornerRadii);

    if(_shaderFeatureDIP & (ShaderFeatures::RoundedCorners | ShaderFeatures::DottedPattern | ShaderFeatures::FogEffect))
        ShaderManager::GetSolidFillShader().SetUniformVec2("uScale", _scale);

    if(_shaderFeatureDIP & ShaderFeatures::DottedPattern)
        uploadDotUniforms(ShaderManager::GetSolidFillShader());

    if(_shaderFeatureDIP & ShaderFeatures::FogEffect)
        uploadFogUniforms(ShaderManager::GetSolidFillShader());

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffectWithImage)
    {
        ShaderManager::GetSolidFillShader().SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (_windowBackgroundTextureID));
    }
}

void RetroFuturaGUI::Rectangle::drawLinearGradientFill()
{
    _gradientOffset += _gradientAnimationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _gradientDegree += _gradientRotationSpeed;
    if(_gradientDegree >= 360.0f)
        _gradientDegree = 0.0f;

    ShaderManager::GetLinearGradientShader().UseProgram();
    ShaderManager::GetLinearGradientShader().SetUniformVec4("uColors", &_colors[0][0], 255);
    ShaderManager::GetLinearGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetLinearGradientShader().SetUniformInt("uNumColors", _colorCount);
    ShaderManager::GetLinearGradientShader().SetUniformInt("uDIP", _shaderFeatureDIP);
    ShaderManager::GetLinearGradientShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetLinearGradientShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetLinearGradientShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetLinearGradientShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetLinearGradientShader().SetUniformFloat("uGradientOffset", _gradientOffset);
    ShaderManager::GetLinearGradientShader().SetUniformFloat("uDegree", _gradientDegree);

    if(_shaderFeatureDIP & ShaderFeatures::RoundedCorners)
        ShaderManager::GetLinearGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);

    if(_shaderFeatureDIP & (ShaderFeatures::RoundedCorners | ShaderFeatures::DottedPattern | ShaderFeatures::FogEffect))
        ShaderManager::GetLinearGradientShader().SetUniformVec2("uScale", _scale);

    if(_shaderFeatureDIP & ShaderFeatures::DottedPattern)
        uploadDotUniforms(ShaderManager::GetLinearGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::FogEffect)
        uploadFogUniforms(ShaderManager::GetLinearGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffectWithImage)
    {
        ShaderManager::GetLinearGradientShader().SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (_windowBackgroundTextureID));
    }
}

void RetroFuturaGUI::Rectangle::drawRadialGradientFill()
{
    _gradientOffset += _gradientAnimationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _gradientDegree += _gradientRotationSpeed;
    if(_gradientDegree >= 360.0f)
        _gradientDegree = 0.0f;

    ShaderManager::GetRadialGradientShader().UseProgram();
    ShaderManager::GetRadialGradientShader().SetUniformVec4("uColors", &_colors[0][0], 255);
    ShaderManager::GetRadialGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetRadialGradientShader().SetUniformInt("uNumColors", _colorCount);
    ShaderManager::GetRadialGradientShader().SetUniformInt("uDIP", _shaderFeatureDIP);
    ShaderManager::GetRadialGradientShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetRadialGradientShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetRadialGradientShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetRadialGradientShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetRadialGradientShader().SetUniformFloat("uGradientOffset", _gradientOffset);
    ShaderManager::GetRadialGradientShader().SetUniformFloat("uDegree", _gradientDegree);

    if(_shaderFeatureDIP & ShaderFeatures::RoundedCorners)
        ShaderManager::GetRadialGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);

    if(_shaderFeatureDIP & (ShaderFeatures::RoundedCorners | ShaderFeatures::DottedPattern | ShaderFeatures::FogEffect))
        ShaderManager::GetRadialGradientShader().SetUniformVec2("uScale", _scale);

    if(_shaderFeatureDIP & ShaderFeatures::DottedPattern)
        uploadDotUniforms(ShaderManager::GetRadialGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::FogEffect)
        uploadFogUniforms(ShaderManager::GetRadialGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffectWithImage)
    {
        ShaderManager::GetRadialGradientShader().SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (_windowBackgroundTextureID));
    }
}

void RetroFuturaGUI::Rectangle::drawHueStarGradientFill()
{
    _gradientOffset += _gradientAnimationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _gradientDegree += _gradientRotationSpeed;
    if(_gradientDegree >= 360.0f)
        _gradientDegree = 0.0f;

    ShaderManager::GetHueStarGradientShader().UseProgram();
    ShaderManager::GetHueStarGradientShader().SetUniformVec4("uColors", &_colors[0][0], 255);
    ShaderManager::GetHueStarGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetHueStarGradientShader().SetUniformInt("uNumColors", _colorCount);
    ShaderManager::GetHueStarGradientShader().SetUniformInt("uDIP", _shaderFeatureDIP);
    ShaderManager::GetHueStarGradientShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetHueStarGradientShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetHueStarGradientShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetHueStarGradientShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetHueStarGradientShader().SetUniformFloat("uGradientOffset", _gradientOffset);
    ShaderManager::GetHueStarGradientShader().SetUniformFloat("uDegree", _gradientDegree);

    if(_shaderFeatureDIP & ShaderFeatures::RoundedCorners)
        ShaderManager::GetHueStarGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);

    if(_shaderFeatureDIP & (ShaderFeatures::RoundedCorners | ShaderFeatures::DottedPattern | ShaderFeatures::FogEffect))
        ShaderManager::GetHueStarGradientShader().SetUniformVec2("uScale", _scale);

    if(_shaderFeatureDIP & ShaderFeatures::DottedPattern)
        uploadDotUniforms(ShaderManager::GetHueStarGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::FogEffect)
        uploadFogUniforms(ShaderManager::GetHueStarGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffectWithImage)
    {
        ShaderManager::GetHueStarGradientShader().SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (_windowBackgroundTextureID));
    }
}

void RetroFuturaGUI::Rectangle::drawSolidBorder()
{
    ShaderManager::GetBorderSolidFillShader().UseProgram();
    ShaderManager::GetBorderSolidFillShader().SetUniformInt("uDIP", _shaderFeatureDIP);
    ShaderManager::GetBorderSolidFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetBorderSolidFillShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetBorderSolidFillShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetBorderSolidFillShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetBorderSolidFillShader().SetUniformVec4("uColor", _colors[0]);
    ShaderManager::GetBorderSolidFillShader().SetUniformFloat("uBorderWidth", _borderWidth);
    ShaderManager::GetBorderSolidFillShader().SetUniformVec4("uCornerRadii", _cornerRadii);
    ShaderManager::GetBorderSolidFillShader().SetUniformVec2("uScale", _scale);

    if(_shaderFeatureDIP & ShaderFeatures::DottedPattern)
        uploadDotUniforms(ShaderManager::GetBorderSolidFillShader());

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffectWithImage)
    {
        ShaderManager::GetBorderSolidFillShader().SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (_windowBackgroundTextureID));
    }
}

void RetroFuturaGUI::Rectangle::drawLinearGradientBorder()
{
    _gradientOffset += _gradientAnimationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _gradientDegree += _gradientRotationSpeed;
    if(_gradientDegree >= 360.0f)
        _gradientDegree = 0.0f;

    ShaderManager::GetBorderLinearGradientShader().UseProgram();
    ShaderManager::GetBorderLinearGradientShader().SetUniformVec4("uColors", &_colors[0][0], 255);
    ShaderManager::GetBorderLinearGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetBorderLinearGradientShader().SetUniformInt("uNumColors", _colorCount);
    ShaderManager::GetBorderLinearGradientShader().SetUniformInt("uDIP", _shaderFeatureDIP);
    ShaderManager::GetBorderLinearGradientShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetBorderLinearGradientShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetBorderLinearGradientShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetBorderLinearGradientShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetBorderLinearGradientShader().SetUniformFloat("uGradientOffset", _gradientOffset);
    ShaderManager::GetBorderLinearGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetBorderLinearGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);
    ShaderManager::GetBorderLinearGradientShader().SetUniformVec2("uScale", _scale);
    ShaderManager::GetBorderLinearGradientShader().SetUniformFloat("uBorderWidth", _borderWidth);

    if(_shaderFeatureDIP & ShaderFeatures::DottedPattern)
        uploadDotUniforms(ShaderManager::GetBorderLinearGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffectWithImage)
    {
        ShaderManager::GetBorderLinearGradientShader().SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (_windowBackgroundTextureID));
    }
}

void RetroFuturaGUI::Rectangle::drawRadialGradientBorder()
{
    _gradientOffset += _gradientAnimationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _gradientDegree += _gradientRotationSpeed;
    if(_gradientDegree >= 360.0f)
        _gradientDegree = 0.0f;

    ShaderManager::GetBorderRadialGradientShader().UseProgram();
    ShaderManager::GetBorderRadialGradientShader().SetUniformVec4("uColors", &_colors[0][0], 255);
    ShaderManager::GetBorderRadialGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetBorderRadialGradientShader().SetUniformInt("uNumColors", _colorCount);
    ShaderManager::GetBorderRadialGradientShader().SetUniformInt("uDIP", _shaderFeatureDIP);
    ShaderManager::GetBorderRadialGradientShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetBorderRadialGradientShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetBorderRadialGradientShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetBorderRadialGradientShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetBorderRadialGradientShader().SetUniformFloat("uGradientOffset", _gradientOffset);
    ShaderManager::GetBorderRadialGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetBorderRadialGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);
    ShaderManager::GetBorderRadialGradientShader().SetUniformVec2("uScale", _scale);
    ShaderManager::GetBorderRadialGradientShader().SetUniformFloat("uBorderWidth", _borderWidth);

    if(_shaderFeatureDIP & ShaderFeatures::DottedPattern)
        uploadDotUniforms(ShaderManager::GetBorderRadialGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffectWithImage)
    {
        ShaderManager::GetBorderRadialGradientShader().SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (_windowBackgroundTextureID));
    }
}

void RetroFuturaGUI::Rectangle::drawHueStarGradientBorder()
{
    _gradientOffset += _gradientAnimationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _gradientDegree += _gradientRotationSpeed;
    if(_gradientDegree >= 360.0f)
        _gradientDegree = 0.0f;

    ShaderManager::GetBorderHueStarGradientShader().UseProgram();
    ShaderManager::GetBorderHueStarGradientShader().SetUniformVec4("uColors", &_colors[0][0], 255);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformInt("uNumColors", _colorCount);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformInt("uDIP", _shaderFeatureDIP);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetBorderHueStarGradientShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformFloat("uGradientOffset", _gradientOffset);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformVec2("uScale", _scale);
    ShaderManager::GetBorderHueStarGradientShader().SetUniformFloat("uBorderWidth", _borderWidth);

    if(_shaderFeatureDIP & ShaderFeatures::DottedPattern)
        uploadDotUniforms(ShaderManager::GetBorderHueStarGradientShader());

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffectWithImage)
    {
        ShaderManager::GetBorderHueStarGradientShader().SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (_windowBackgroundTextureID));
    }
}

void RetroFuturaGUI::Rectangle::SetBorderWidth(const f32 width)
{
    _borderWidth = width;
}

void RetroFuturaGUI::Rectangle::SetRectangleMode(const RectangleMode rectanlgeMode)
{
    _rectangleMode = rectanlgeMode;
}

void RetroFuturaGUI::Rectangle::SetFillType(const FillType fillType)
{
    _fillType = fillType;
}

const glm::vec2 RetroFuturaGUI::Rectangle::GetSize() const
{
    return _scale;
}

const glm::vec3 RetroFuturaGUI::Rectangle::GetPosition() const
{
    return _position;
}

f32 RetroFuturaGUI::Rectangle::GetBorderWidth() const
{
    return _borderWidth;
}