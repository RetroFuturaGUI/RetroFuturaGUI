#include "Polyline.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::Polyline::Polyline(Projection* projection)
    : _projection(*projection)
{
    setupMesh();
}

RetroFuturaGUI::Polyline::~Polyline()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
}

void RetroFuturaGUI::Polyline::SetPoints(std::span<const glm::vec2> points)
{
    _vertexCount = 2 > points.size() ? 0 : points.size() + 2;

    if(0 == _vertexCount)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    if(_vertexCount > _bufferVertexCount)
    {
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(_vertexCount * sizeof(glm::vec2)), nullptr, GL_DYNAMIC_DRAW);
        _bufferVertexCount = _vertexCount;
    }

    // GL_LINE_STRIP_ADJACENCY reads a neighbour beyond each end, so the first and last points go in twice - where the neighbour
    // coincides, Line.gs cuts the end square
    constexpr GLsizeiptr vertexSize { sizeof(glm::vec2) };
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, &points.front());
    glBufferSubData(GL_ARRAY_BUFFER, vertexSize, static_cast<GLsizeiptr>(points.size_bytes()), points.data());
    glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(points.size() + 1) * vertexSize, vertexSize, &points.back());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RetroFuturaGUI::Polyline::SetSize(const glm::vec3& size)
{
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
}

void RetroFuturaGUI::Polyline::SetPosition(const glm::vec3& position)
{
    _translationMatrix = glm::translate(glm::mat4(1.0f), position);
}

void RetroFuturaGUI::Polyline::SetRotation(const glm::vec3& rotation)
{
    const glm::vec3 radians { glm::radians(rotation) };
    _rotationMatrix =
        glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
}

void RetroFuturaGUI::Polyline::SetColor(const glm::vec4& color)
{
    _color = color;
}

const glm::vec4& RetroFuturaGUI::Polyline::GetColor() const
{
    return _color;
}

void RetroFuturaGUI::Polyline::SetThickness(const f32 thickness)
{
    _thickness = thickness;
}

void RetroFuturaGUI::Polyline::Draw()
{
    if(0 == _vertexCount)
        return;

    Shader& shader { ShaderManager::GetLineFillShader() };
    shader.UseProgram();
    shader.SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    shader.SetUniformMat4("uPosition", _translationMatrix);
    shader.SetUniformMat4("uScaling", _scalingMatrix);
    shader.SetUniformMat4("uRotation", _rotationMatrix);
    shader.SetUniformVec4("uColor", _color);
    shader.SetUniformFloat("uLineThickness", _thickness);

    // Where a sharp turn gets a bevel, its two segments overlap at the same depth.
    // anti-aliased edge instead of failing the depth test against it
    GLint depthFunction { GL_LESS };
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(_vao);
    glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, static_cast<GLsizei>(_vertexCount));
    glBindVertexArray(0);

    glDepthFunc(static_cast<GLenum>(depthFunction));
}

void RetroFuturaGUI::Polyline::setupMesh()
{
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);

    glBindVertexArray(0);
}