#include "Rectangle.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace RetroFuturaGUI;

Rectangle::Rectangle(Projection& projection, const f32 width, const f32 height, const f32 positionX, const f32 positionY)
    : _projection(projection)
{
    glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    initBasic(std::span<const glm::vec4>(&color, 1));
    Resize(width, height);
    Move(positionX, positionY);
}

Rectangle::Rectangle(Projection& projection, const glm::vec4 &color, const f32 width, const f32 height, const f32 positionX, const f32 positionY)
    : _projection(projection)
{
    initBasic(std::span<const glm::vec4>(&color, 1));
    Resize(width, height);
    Move(positionX, positionY);
}

Rectangle::Rectangle(Projection& projection, std::span<const glm::vec4> colors, const f32 width, const f32 height, const f32 positionX, const f32 positionY, const f32 degree, const f32 animationSpeed, const f32 rotationSpeed)
    : _projection(projection)
{
    initBasic(colors);
    _degree = degree;
    _animationSpeed = animationSpeed;
    _rotationSpeed = rotationSpeed;

    ShaderManager::GetFillGradientShader().UseProgram();
    ShaderManager::GetFillGradientShader().SetUniformVec4("uColors", &_colors[0][0], 255);
    ShaderManager::GetFillGradientShader().SetUniformFloat("uDegree", _degree);
    ShaderManager::GetFillGradientShader().SetUniformInt("uNumColors", _colorCount);
    Resize(width, height);
    Move(positionX, positionY);
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

void RetroFuturaGUI::Rectangle::UpdateDegree(const f32 degree)
{
    _degree = degree;
}

void RetroFuturaGUI::Rectangle::UpdateRotationSpeed(const f32 speed)
{
    _rotationSpeed = speed;
}

void RetroFuturaGUI::Rectangle::Resize(const f32 width, const f32 height)
{
    _scale = glm::vec2(width * 0.5f, height * 0.5f);
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(_scale, 1.0f));
}

void RetroFuturaGUI::Rectangle::Move(const f32 x, const f32 y)
{
    _position = glm::vec2(x, y);
    _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_position, 0.0f));
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

    //i32 projectionLocation = ShaderManager::GetFillShader().GetProjectionLocation();       
    //if (projectionLocation != -1) 
      //  glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(_projection.GetProjectionMatrix()));

    ShaderManager::GetFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetFillShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetFillShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetFillShader().SetUniformVec4("uColor", _colors[0]);
}

void RetroFuturaGUI::Rectangle::drawWithGradientFill()
{
    ShaderManager::GetFillGradientShader().UseProgram();

    //i32 projectionLocation = ShaderManager::GetFillGradientShader().GetProjectionLocation();       
    //if (projectionLocation != -1) 
      //  glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(_projection.GetProjectionMatrix()));

    _gradientOffset += _animationSpeed;
    if (_gradientOffset > 1.0f) 
        _gradientOffset = 0.0f;

    _degree += _rotationSpeed;
    if(_degree >= 360.0f)
        _degree = 0.0f;

    ShaderManager::GetFillGradientShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetFillGradientShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetFillGradientShader().SetUniformMat4("uScaling", _scalingMatrix);
    ShaderManager::GetFillGradientShader().SetUniformFloat("uGradientOffset", _gradientOffset);
    ShaderManager::GetFillGradientShader().SetUniformFloat("uDegree", _degree);
}
