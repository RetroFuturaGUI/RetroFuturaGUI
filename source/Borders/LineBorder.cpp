#include "LineBorder.hpp"

RetroFuturaGUI::LineBorder::LineBorder(const GeometryParams2D& geometry, const glm::vec4& color, const f32 borderThickness)
: _projection(const_cast<Projection&>(geometry._Projection)), _borderThickness(borderThickness)
{
    setupMesh();
    initBasic(std::span<const glm::vec4>(&color, 1));
    Resize(geometry._Size);
    Move(geometry._Position);
    Rotate(geometry._Rotation);
}

RetroFuturaGUI::LineBorder::~LineBorder()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void RetroFuturaGUI::LineBorder::Draw()
{
    ShaderManager::GetLineFillShader().UseProgram();
    ShaderManager::GetLineFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    //ShaderManager::GetLineFillShader().SetUniformVec4("uCornerRadii", _cornerRadii);
    ShaderManager::GetLineFillShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetLineFillShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetLineFillShader().SetUniformMat4("uRotation", _rotationMatrix);
    ShaderManager::GetLineFillShader().SetUniformVec4("uColor", _colors[0]);
    ShaderManager::GetLineFillShader().SetUniformFloat("uLineThickness", _borderThickness * 0.5f);

    glBindVertexArray(_vao);
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RetroFuturaGUI::LineBorder::Resize(const glm::vec2& size)
{
    _scale = size * 0.5f;
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(_scale, 1.0f));
}

void RetroFuturaGUI::LineBorder::Move(const glm::vec2& position)
{
    _position = position;
    _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_position, 0.0f));
}

void RetroFuturaGUI::LineBorder::Rotate(const float rotation)
{
    _rotation = rotation;
    _rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
}

void RetroFuturaGUI::LineBorder::SetColor(const glm::vec4 &color)
{
    _colors[0] = color;
}

glm::vec4 RetroFuturaGUI::LineBorder::GetColor() const
{
    return _colors[0];
}

void RetroFuturaGUI::LineBorder::setupMesh()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_testBorderPoints), _testBorderPoints, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_testBorderIndices), _testBorderIndices, GL_STATIC_DRAW);
}

void RetroFuturaGUI::LineBorder::initBasic(std::span<const glm::vec4> colors)
{
    _colorCount = colors.size();
    _fillType = _colorCount > 1 ? FillType::GRADIENT : FillType::SOLID;

    _colors = std::make_unique<glm::vec4[]>(_colorCount);
    for (u32 i = 0; i < colors.size(); ++i)
        _colors[i] = colors[i];
}
