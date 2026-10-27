#include "IRectangle.hpp"

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

RetroFuturaGUI::Projection& RetroFuturaGUI::IRectangle::GetProjection() const
{
    return _projection;
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

void RetroFuturaGUI::IRectangle::uploadTransform(Shader& shader) const
{
    shader.SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    shader.SetUniformMat4("uPosition", _translationMatrix);
    shader.SetUniformMat4("uScaling", _scalingMatrix);
    shader.SetUniformMat4("uRotation", _rotationMatrix);
    shader.SetUniformVec2("uScale", _scale);

    if(_shaderFeatureDIP & ShaderFeatures::RoundedCorners)
        shader.SetUniformVec4("uCornerRadii", _cornerRadii);
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
