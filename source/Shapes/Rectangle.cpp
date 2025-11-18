#include "Rectangle.hpp"

using namespace RetroFuturaGUI;

Rectangle::Rectangle()
{
    glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    initBasic(std::span<const glm::vec4>(&color, 1));
}

Rectangle::Rectangle(const glm::vec4 &color)
{
    initBasic(std::span<const glm::vec4>(&color, 1));
}

Rectangle::Rectangle(std::span<const glm::vec4> colors, const float degree, const float animationSpeed, const float rotationSpeed)
{
    initBasic(colors);
    _degree = degree;
    _animationSpeed = animationSpeed;
    _rotationSpeed = rotationSpeed;

    ShaderManager::GetFillGradientShader().UseProgram();

    u32 progId = ShaderManager::GetFillGradientShader().GetProgramId();
    i32 locColors = glGetUniformLocation(progId, "uColors");
    glUniform4fv(locColors, 255, &_colors[0][0]);

    i32 locDegree = glGetUniformLocation(progId, "uDegree");
    glUniform1f(locDegree, _degree);

    i32 locNum = glGetUniformLocation(progId, "uNumColors");
    glUniform1i(locNum, _colorCount);
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

void RetroFuturaGUI::Rectangle::UpdateAnimationSpeed(const float speed)
{
    _animationSpeed = speed;
}

void RetroFuturaGUI::Rectangle::UpdateDegree(const float degree)
{
    _degree = degree;
}

void RetroFuturaGUI::Rectangle::UpdateRotationSpeed(const float speed)
{
    _rotationSpeed = speed;
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
    setupMesh();
    _colorCount = colors.size();
    _fillType = _colorCount > 1 ? FillType::GRADIENT : FillType::SOLID;

    _colors = std::make_unique<glm::vec4[]>(_colorCount);
    for (u32 i = 0; i < colors.size(); ++i)
        _colors[i] = colors[i];
}

void RetroFuturaGUI::Rectangle::drawWithSolidFill()
{            
    ShaderManager::GetFillShader().UseProgram();
    i32 locColor = glGetUniformLocation(ShaderManager::GetFillShader().GetProgramId(), "uColor");
            
    if (locColor != -1) 
        glUniform4f(locColor, _colors[0].r, _colors[0].g, _colors[0].b, _colors[0].a);
}

void RetroFuturaGUI::Rectangle::drawWithGradientFill()
{
    ShaderManager::GetFillGradientShader().UseProgram();
    u32 progId = ShaderManager::GetFillGradientShader().GetProgramId();

    _gradientOffset += _animationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _degree += _rotationSpeed;
    if(_degree >= 360.0f)
        _degree = 0.0f;

    glUniform1f(glGetUniformLocation(progId, "uGradientOffset"), _gradientOffset);
    glUniform1f(glGetUniformLocation(progId, "uDegree"), _degree);
}
