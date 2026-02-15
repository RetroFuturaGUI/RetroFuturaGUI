#include "Rectangle.hpp"
#include <print>

/*RetroFuturaGUI::Rectangle::Rectangle(const GeometryParams2D& geometry, glm::vec4& color, const RectangleMode rectangleMode)
    : _projection(const_cast<Projection&>(geometry._Projection)), _rectangleMode(rectangleMode)
{
    setupMesh();
    initBasic(color);
    SetSize(geometry._Size);
    SetPosition(geometry._Position);
    SetRotation(geometry._Rotation);
}*/

RetroFuturaGUI::Rectangle::Rectangle(const GeometryParams2D &geometry, std::span<glm::vec4> colors, const RectangleMode rectangleMode)
    : _projection(const_cast<Projection&>(geometry._Projection)), _rectangleMode(rectangleMode)
{
    setupMesh();
    initBasic(colors);
    SetSize(geometry._Size);
    SetPosition(geometry._Position);
    SetRotation(geometry._Rotation);
}

RetroFuturaGUI::Rectangle::~Rectangle()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void RetroFuturaGUI::Rectangle::Draw()
{
    switch(_rectangleMode)
    {
        case RectangleMode::BORDER:
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
        default: // RectangleMode::PLANE
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

void RetroFuturaGUI::Rectangle::SetPosition(const glm::vec2& position)
{
    _position = position;
    _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_position, 0.0f));
}

void RetroFuturaGUI::Rectangle::SetColor(std::span<glm::vec4> color)
{
    initBasic(color);
}

glm::vec4 RetroFuturaGUI::Rectangle::GetColor() const
{
    return _colors[0];
}

void RetroFuturaGUI::Rectangle::SetCornerRadii(const glm::vec4& radii)
{
    _cornerRadii = radii;

    for(u32 i = 0; i < 4; ++i)
    {
        if(_cornerRadii[i] > 0.0f)
        {
            _shaderFeatureDIP |= ShaderFeatures::ROUNDED_CORNERS;
            return;
        }
    }

    _shaderFeatureDIP &= ~ShaderFeatures::ROUNDED_CORNERS;
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

void RetroFuturaGUI::Rectangle::SetShaderFeatures(const u32 features, const bool reset)
{
    if(reset)
        _shaderFeatureDIP = features;
    else
        _shaderFeatureDIP |= features;
}

void RetroFuturaGUI::Rectangle::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    _shaderFeatureDIP |= ShaderFeatures::GLASS_EFFECT_WITH_IMAGE;
    _windowBackgroundTextureID = textureID;
}

void RetroFuturaGUI::Rectangle::SetRotation(const f32 rotation)
{
    _rotation = rotation;
    _rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
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

void RetroFuturaGUI::Rectangle::initBasic(std::span<glm::vec4> colors)
{
    _colors = colors;
    _colorCount = static_cast<i32>(_colors.size());
    //_fillType = _colorCount > 1 ? FillType::GRADIENT : FillType::SOLID;
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

    if(_shaderFeatureDIP & ShaderFeatures::ROUNDED_CORNERS)
    {
        ShaderManager::GetSolidFillShader().SetUniformVec4("uCornerRadii", _cornerRadii);
        ShaderManager::GetSolidFillShader().SetUniformVec2("uScale", _scale);
    }

    if(_shaderFeatureDIP & ShaderFeatures::GLASS_EFFECT_WITH_IMAGE)
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

    if(_shaderFeatureDIP & ShaderFeatures::ROUNDED_CORNERS)
    {
        ShaderManager::GetLinearGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);
        ShaderManager::GetLinearGradientShader().SetUniformVec2("uScale", _scale);
    }

    if(_shaderFeatureDIP & ShaderFeatures::GLASS_EFFECT_WITH_IMAGE)
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

    if(_shaderFeatureDIP & ShaderFeatures::ROUNDED_CORNERS)
    {
        ShaderManager::GetRadialGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);
        ShaderManager::GetRadialGradientShader().SetUniformVec2("uScale", _scale);
    }

    if(_shaderFeatureDIP & ShaderFeatures::GLASS_EFFECT_WITH_IMAGE)
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

    if(_shaderFeatureDIP & ShaderFeatures::ROUNDED_CORNERS)
    {
        ShaderManager::GetHueStarGradientShader().SetUniformVec4("uCornerRadii", _cornerRadii);
        ShaderManager::GetHueStarGradientShader().SetUniformVec2("uScale", _scale);
    }

    if(_shaderFeatureDIP & ShaderFeatures::GLASS_EFFECT_WITH_IMAGE)
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

    if(_shaderFeatureDIP & ShaderFeatures::GLASS_EFFECT_WITH_IMAGE)
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

    if(_shaderFeatureDIP & ShaderFeatures::GLASS_EFFECT_WITH_IMAGE)
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

    if(_shaderFeatureDIP & ShaderFeatures::GLASS_EFFECT_WITH_IMAGE)
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

    if(_shaderFeatureDIP & ShaderFeatures::GLASS_EFFECT_WITH_IMAGE)
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