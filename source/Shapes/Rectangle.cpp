#include "Rectangle.hpp"

RetroFuturaGUI::Rectangle::Rectangle(const GeometryParams2D& geometry, const glm::vec4& color)
    : _projection(const_cast<Projection&>(geometry._Projection))
{
    setupMesh();
    initBasic(std::span<const glm::vec4>(&color, 1));
    Resize(geometry._Size);
    Move(geometry._Position);
    Rotate(geometry._Rotation);
}

RetroFuturaGUI::Rectangle::Rectangle(const GeometryParams2D& geometry, const BackgroundParams& background)
    : _projection(const_cast<Projection&>(geometry._Projection))
{
    setupMesh();
    initBasic(background._Colors);
    _gradientDegree = background._GradientDegree;
    _animationSpeed = background._AnimationSpeed;
    _gradientRotationSpeed = background._GradientRotationSpeed;

    ShaderManager::GetFillGradientShader().UseProgram();
    ShaderManager::GetFillGradientShader().SetUniformVec4("uColors", &_colors[0][0], 255);
    ShaderManager::GetFillGradientShader().SetUniformFloat("uDegree", _gradientDegree);
    ShaderManager::GetFillGradientShader().SetUniformInt("uNumColors", _colorCount);
    Resize(geometry._Size);
    Move(geometry._Position);
    Rotate(geometry._Rotation);
}

RetroFuturaGUI::Rectangle::~Rectangle()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void RetroFuturaGUI::Rectangle::Draw()
{
    switch(_fillType)
    {
        case FillType::GRADIENT:
        {
            drawWithGradientFill();
        } break;
        default: // FillType::SOLID
        {
            drawWithSolidFill();
        } break;
    }
    
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void RetroFuturaGUI::Rectangle::UpdateAnimationSpeed(const f32 speed)
{
    _animationSpeed = speed;
}

void RetroFuturaGUI::Rectangle::UpdateGradientDegree(const f32 degree)
{
    _gradientDegree = degree;
}

void RetroFuturaGUI::Rectangle::UpdateGradientRotationSpeed(const f32 speed)
{
    _gradientRotationSpeed = speed;
}

void RetroFuturaGUI::Rectangle::Resize(const glm::vec2& size)
{
    _scale = glm::vec2(size.x * 0.5f, size.y * 0.5f);
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(_scale, 1.0f));
}

void RetroFuturaGUI::Rectangle::Move(const glm::vec2& position)
{
    _position = position;
    _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_position, 0.0f));
}

glm::vec4 RetroFuturaGUI::Rectangle::GetColor() const
{
    return _colors[0];
}

void RetroFuturaGUI::Rectangle::Rotate(const float rotation)
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

void RetroFuturaGUI::Rectangle::initBasic(std::span<const glm::vec4> colors)
{
    _colorCount = colors.size();
    _fillType = _colorCount > 1 ? FillType::GRADIENT : FillType::SOLID;

    _colors = std::make_unique<glm::vec4[]>(_colorCount);
    for (u32 i = 0; i < colors.size(); ++i)
        _colors[i] = colors[i];
}

void RetroFuturaGUI::Rectangle::drawWithSolidFill()
{            
    ShaderManager::GetFillShader().UseProgram();
    ShaderManager::GetFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetFillShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetFillShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetFillShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetFillShader().SetUniformVec4("uColor", _colors[0]);
}

void RetroFuturaGUI::Rectangle::drawWithGradientFill()
{
    _gradientOffset += _animationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _gradientDegree += _gradientRotationSpeed;
    if(_gradientDegree >= 360.0f)
        _gradientDegree = 0.0f;

    ShaderManager::GetFillGradientShader().UseProgram();
    ShaderManager::GetFillGradientShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetFillGradientShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetFillGradientShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetFillGradientShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetFillGradientShader().SetUniformFloat("uGradientOffset", _gradientOffset);
    ShaderManager::GetFillGradientShader().SetUniformFloat("uDegree", _gradientDegree);
}