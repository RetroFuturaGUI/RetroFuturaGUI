#include "Polyline.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::Polyline::Polyline(Projection* projection)
    : _projection(*projection)
{
    setupMesh();
}

RetroFuturaGUI::Polyline::~Polyline()
{
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
    if(!_vertexArray)
        return;

    if(0 == _vertexCount)
        return;

    Shader& shader { ShaderManager::GetLineFillShader() };
    shader.UseProgram();
    shader.SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    shader.SetUniformMat4("uPosition", _translationMatrix);
    shader.SetUniformMat4("uScaling", _scalingMatrix);
    shader.SetUniformMat4("uRotation", _rotationMatrix);

    //The pieces of a turn lie at the same depth, so the line draws over its own glow and its pieces pass the depth test
    //against each other; the stencil, not the depth, keeps each pixel to one fragment per pass
    GLint depthFunction { GL_LESS };
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); //a pixel drawn once takes no second fragment

    if(0.0f < _glowWidth && 0.0f < _glowColor.a)
        drawPass(shader, _thickness + _glowWidth * 2.0f, _glowColor, _glowWidth);

    drawPass(shader, _thickness, _color, 0.0f);

    glDisable(GL_STENCIL_TEST);
    glDepthFunc(static_cast<GLenum>(depthFunction));
}

void RetroFuturaGUI::Polyline::SetGlow(const f32 width, const glm::vec4& color)
{
    _glowWidth = width;
    _glowColor = color;
}

void RetroFuturaGUI::Polyline::drawPass(Shader& shader, const f32 thickness, const glm::vec4& color, const f32 glowWidth)
{
    glClear(GL_STENCIL_BUFFER_BIT); //every pass starts with no pixel drawn
    shader.SetUniformVec4("uColor", color);
    shader.SetUniformFloat("uLineThickness", thickness);
    shader.SetUniformFloat("uGlowWidth", glowWidth);
    shader.SetUniformFloat("uCoreThickness", _thickness);

    //The fully covered fragments first, then the faded edges into the pixels still free, so where pieces of a turn overlap a
    //faded edge never keeps a full fragment out
    _vertexArray->Bind();

    for(const i32 band : { 1, 2 })
    {
        shader.SetUniformInt("uCoverageBand", band);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, static_cast<GLsizei>(_vertexCount));
    }

    VertexArray::Unbind();
}

void RetroFuturaGUI::Polyline::setupMesh()
{
    glGenBuffers(1, &_vbo);

    _vertexArray = std::make_unique<VertexArray>([vbo = _vbo]()
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
        });
}