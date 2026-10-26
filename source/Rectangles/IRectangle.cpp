#include "IRectangle.hpp"
#include <algorithm>

RetroFuturaGUI::IRectangle::IRectangle(Projection* projection)
    : _projection(*projection)
{
    if(0 == _rectangleCount)
        createQuad();

    ++_rectangleCount;
}

RetroFuturaGUI::IRectangle::~IRectangle()
{
    --_rectangleCount;

    if(0 == _rectangleCount)
        deleteQuad();
}

void RetroFuturaGUI::IRectangle::SetSize(const glm::vec2& size)
{
    _scale = glm::vec2(size.x, size.y);
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(_scale, 1.0f));
}

void RetroFuturaGUI::IRectangle::SetPosition(const glm::vec3& position)
{
    _position = position;
    _translationMatrix = glm::translate(glm::mat4(1.0f), _position);
}

void RetroFuturaGUI::IRectangle::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;
    glm::vec3 radians = glm::radians(_rotation);
    _rotationMatrix =
        glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
}

const glm::vec2& RetroFuturaGUI::IRectangle::GetSize() const
{
    return _scale;
}

const glm::vec3& RetroFuturaGUI::IRectangle::GetPosition() const
{
    return _position;
}

const glm::vec3& RetroFuturaGUI::IRectangle::GetRotation() const
{
    return _rotation;
}

void RetroFuturaGUI::IRectangle::SetColors(std::span<glm::vec4> colors)
{
    _colors = colors;
    _colorCount = std::min<uSize>(_colors.size(), 255);
}

void RetroFuturaGUI::IRectangle::SetFillType(const FillType fillType)
{
    _fillType = fillType;
}

void RetroFuturaGUI::IRectangle::SetCornerRadii(const glm::vec4& radii)
{
    _cornerRadii = radii;

    for(u32 i = 0; 4 > i; ++i)
    {
        if(0.0f < _cornerRadii[i])
        {
            _shaderFeatureDIP |= ShaderFeatures::RoundedCorners;
            return;
        }
    }

    _shaderFeatureDIP &= ~ShaderFeatures::RoundedCorners;
}

void RetroFuturaGUI::IRectangle::SetGradientOffset(const f32 gradientOffset)
{
    _gradientOffset = gradientOffset;
}

void RetroFuturaGUI::IRectangle::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _gradientAnimationSpeed = animationSpeed;
}

void RetroFuturaGUI::IRectangle::SetGradientDegree(const f32 degree)
{
    _gradientDegree = degree;
}

void RetroFuturaGUI::IRectangle::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _gradientRotationSpeed = rotationSpeed;
}

f32 RetroFuturaGUI::IRectangle::GetGradientOffset() const
{
    return _gradientOffset;
}

f32 RetroFuturaGUI::IRectangle::GetGradientDegree() const
{
    return _gradientDegree;
}

void RetroFuturaGUI::IRectangle::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    _shaderFeatureDIP |= ShaderFeatures::GlassEffectWithImage;
    _windowBackgroundTextureID = textureID;
}

void RetroFuturaGUI::IRectangle::SetRasterPattern(const RasterPattern rasterPattern)
{
    _rasterPattern = rasterPattern;
}

void RetroFuturaGUI::IRectangle::SetPrimaryRasterColor(const glm::vec4& color)
{
    _primaryRasterColor = color;
}

void RetroFuturaGUI::IRectangle::SetSecondaryRasterColor(const glm::vec4& color)
{
    _secondaryRasterColor = color;
}

void RetroFuturaGUI::IRectangle::SetDotDistance(const f32 distance)
{
    _dotDistance = distance;
}

void RetroFuturaGUI::IRectangle::SetRasterDegree(const f32 degree)
{
    _rasterDegree = degree;
}

void RetroFuturaGUI::IRectangle::SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _primaryRasterWidthTransfer = widthTransfer;
    _primaryRasterWidthTransferCount = std::min<uSize>(_primaryRasterWidthTransfer.size(), 255);

    if(0 < _primaryRasterWidthTransferCount)
        _shaderFeatureDIP |= ShaderFeatures::Raster;
    else
        _shaderFeatureDIP &= ~ShaderFeatures::Raster;
}

void RetroFuturaGUI::IRectangle::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    _dotTransparencyTransfer = transparencyTransfer;
}

void RetroFuturaGUI::IRectangle::SetRasterAnimationSpeed(const f32 animationSpeed)
{
    _rasterAnimationSpeed = animationSpeed;
}

void RetroFuturaGUI::IRectangle::uploadTransform(Shader& shader) const
{
    shader.SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    shader.SetUniformMat4("uPosition", _translationMatrix);
    shader.SetUniformMat4("uScaling", _scalingMatrix);
    shader.SetUniformMat4("uRotation", _rotationMatrix);
    shader.SetUniformVec2("uScale", _scale);
}

void RetroFuturaGUI::IRectangle::uploadFill(Shader& shader)
{
    shader.SetUniformInt("uDIP", _shaderFeatureDIP);

    if(_shaderFeatureDIP & ShaderFeatures::RoundedCorners)
        shader.SetUniformVec4("uCornerRadii", _cornerRadii);

    if(_fillType == FillType::Solid)
    {
        shader.SetUniformVec4("uColor", _colors[0]);
    }
    else
    {
        _gradientOffset += _gradientAnimationSpeed;
        if (1.0f < _gradientOffset)
            _gradientOffset = 0.0f;

        _gradientDegree += _gradientRotationSpeed;
        if(360.0f <= _gradientDegree)
            _gradientDegree = 0.0f;

        shader.SetUniformVec4("uColors", &_colors[0][0], static_cast<u32>(_colorCount));
        shader.SetUniformInt("uNumColors", static_cast<i32>(_colorCount));
        shader.SetUniformFloat("uDegree", _gradientDegree);
        shader.SetUniformFloat("uGradientOffset", _gradientOffset);
    }

    if(_shaderFeatureDIP & ShaderFeatures::Raster)
        uploadRaster(shader);

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffect)
    {
        shader.SetUniformInt("uBackgroundTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _windowBackgroundTextureID);
    }
}

void RetroFuturaGUI::IRectangle::drawQuad()
{
    if(!_quadVertexArray)
        return;

    _quadVertexArray->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VertexArray::Unbind();
}

void RetroFuturaGUI::IRectangle::createQuad()
{
    glGenBuffers(1, &_quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, _quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_quadVertices), _quadVertices, GL_STATIC_DRAW);

    //Filled through GL_ARRAY_BUFFER, since the GL_ELEMENT_ARRAY_BUFFER binding belongs to whichever VAO is bound
    glGenBuffers(1, &_quadEbo);
    glBindBuffer(GL_ARRAY_BUFFER, _quadEbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_quadIndices), _quadIndices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    _quadVertexArray = std::make_unique<VertexArray>([vbo = _quadVbo, ebo = _quadEbo]()
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        });
}

void RetroFuturaGUI::IRectangle::deleteQuad()
{
    _quadVertexArray.reset();
    glDeleteBuffers(1, &_quadVbo);
    glDeleteBuffers(1, &_quadEbo);
    _quadVbo = 0;
    _quadEbo = 0;
}

void RetroFuturaGUI::IRectangle::uploadRaster(Shader& shader)
{
    _rasterAnimationOffset += _rasterAnimationSpeed;

    // The width transfer, the primary color, the angle and the animation drive every pattern, so
    // they go up whichever one is selected. Each pattern then adds only the uniforms it owns.
    shader.SetUniformInt("uRasterPattern", static_cast<i32>(_rasterPattern));
    shader.SetUniformVec4("uPrimaryRasterColor", _primaryRasterColor);
    shader.SetUniformFloat("uRasterDegree", _rasterDegree);
    shader.SetUniformFloat("uRasterAnimationOffset", _rasterAnimationOffset);
    shader.SetUniformInt("uPrimaryRasterWidthTransferCount", static_cast<i32>(_primaryRasterWidthTransferCount));

    if(0 < _primaryRasterWidthTransferCount)
        shader.SetUniformFloat("uPrimaryRasterWidthTransfer", _primaryRasterWidthTransfer.data(), static_cast<u32>(_primaryRasterWidthTransferCount));

    switch(_rasterPattern)
    {
        case RasterPattern::Checkered:
        {
            uploadCheckeredUniforms(shader);
        } break;
        default: // RasterPattern::Dotted
        {
            uploadDotUniforms(shader);
        } break;
    }
}

void RetroFuturaGUI::IRectangle::uploadDotUniforms(Shader& shader) const
{
    shader.SetUniformFloat("uDotDistance", _dotDistance);
    shader.SetUniformFloat("uDotTransparencyTransfer", _dotTransparencyTransfer);
}

void RetroFuturaGUI::IRectangle::uploadCheckeredUniforms(Shader& shader) const
{
    shader.SetUniformVec4("uSecondaryRasterColor", _secondaryRasterColor);
}
