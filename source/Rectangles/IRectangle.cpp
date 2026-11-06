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
    if(radii == _cornerRadii)
        return;

    _cornerRadii = radii;
    const bool isInstant
    {
        !_transitionSettings || !_hasShownCornerRadii
        || 0 >= Transitions::Resolve(*_transitionSettings, TransitionProperty::CornerRadii)._Duration.count()
    };

    if(isInstant)
    {
        _isCornerRadiiTransitionRunning = false;
        _shownCornerRadii = _cornerRadii;
        setRoundedCornersFeature(isRounded(_cornerRadii));
        return;
    }

    _fromCornerRadii = _shownCornerRadii; //from where the corners are now, which is midway when the last transition hasn't arrived yet
    _cornerRadiiTransitionStart = std::chrono::steady_clock::now();
    _isCornerRadiiTransitionRunning = true;
    setRoundedCornersFeature(isRounded(_fromCornerRadii) || isRounded(_cornerRadii));
}

const glm::vec4& RetroFuturaGUI::IRectangle::GetCornerRadii() const
{
    return _cornerRadii;
}

void RetroFuturaGUI::IRectangle::uploadTransform(Shader& shader)
{
    updateShownCornerRadii();
    shader.SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    shader.SetUniformMat4("uPosition", _translationMatrix);
    shader.SetUniformMat4("uScaling", _scalingMatrix);
    shader.SetUniformMat4("uRotation", _rotationMatrix);
    shader.SetUniformVec2("uScale", _scale);

    if(_shaderFeatureDIP & ShaderFeatures::RoundedCorners)
        shader.SetUniformVec4("uCornerRadii", _shownCornerRadii);
}

void RetroFuturaGUI::IRectangle::updateShownCornerRadii()
{
    _hasShownCornerRadii = true;

    if(!_isCornerRadiiTransitionRunning)
        return;

    if(!_transitionSettings)
        return;

    const f32 progress
    {
        Transitions::GetProgress(_cornerRadiiTransitionStart, std::chrono::steady_clock::now(),
                                 Transitions::Resolve(*_transitionSettings, TransitionProperty::CornerRadii))
    };

    if(1.0f > progress)
    {
        _shownCornerRadii = glm::mix(_fromCornerRadii, _cornerRadii, progress);
        return;
    }

    _isCornerRadiiTransitionRunning = false;
    _shownCornerRadii = _cornerRadii;
    setRoundedCornersFeature(isRounded(_cornerRadii));
}

void RetroFuturaGUI::IRectangle::setRoundedCornersFeature(const bool rounded)
{
    if(rounded)
        _shaderFeatureDIP |= ShaderFeatures::RoundedCorners;
    else
        _shaderFeatureDIP &= ~ShaderFeatures::RoundedCorners;
}

bool RetroFuturaGUI::IRectangle::isRounded(const glm::vec4& radii)
{
    return 0.0f < radii.x || 0.0f < radii.y || 0.0f < radii.z || 0.0f < radii.w;
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
