#include "ITexture.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::ITexture::ITexture(Projection* projection, const bool verticallyFlipped)
    : _verticallyFlipped(verticallyFlipped), _projection(projection)
{}

RetroFuturaGUI::ITexture::ITexture(ITexture&& other) noexcept
    : _id(other._id), _resolution(other._resolution), _aspectRatio(other._aspectRatio),
      _verticallyFlipped(other._verticallyFlipped), _projection(other._projection),
      _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo),
      _scalingMatrix(other._scalingMatrix), _translationMatrix(other._translationMatrix), _rotationMatrix(other._rotationMatrix)
{
    other._id = 0;
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
}

RetroFuturaGUI::ITexture& RetroFuturaGUI::ITexture::operator=(ITexture&& other) noexcept
{
    if(this == &other)
        return *this;

    releaseGPUResources();

    _id = other._id;
    _resolution = other._resolution;
    _aspectRatio = other._aspectRatio;
    _verticallyFlipped = other._verticallyFlipped;
    _projection = other._projection;
    _vao = other._vao;
    _vbo = other._vbo;
    _ebo = other._ebo;
    _scalingMatrix = other._scalingMatrix;
    _translationMatrix = other._translationMatrix;
    _rotationMatrix = other._rotationMatrix;

    other._id = 0;
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;

    return *this;
}

RetroFuturaGUI::ITexture::~ITexture()
{
    releaseGPUResources();
}

void RetroFuturaGUI::ITexture::releaseGPUResources()
{
    glDeleteTextures(1, &_id);
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void RetroFuturaGUI::ITexture::Bind(const u32 unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, _id);
}

void RetroFuturaGUI::ITexture::Draw() const
{
    if(!_projection)
        return;

    Shader& shader = ShaderManager::GetTextureFillShader();
    shader.UseProgram();
    shader.SetUniformInt("uTexture", 0);
    shader.SetUniformMat4("uProjection", _projection->GetProjectionMatrix());
    shader.SetUniformMat4("uPosition", _translationMatrix);
    shader.SetUniformMat4("uScaling", _scalingMatrix);
    shader.SetUniformMat4("uRotation", _rotationMatrix);

    Bind(0);
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void RetroFuturaGUI::ITexture::SetSize(const glm::vec2& size)
{
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
}

void RetroFuturaGUI::ITexture::SetPosition(const glm::vec3& position)
{
    _translationMatrix = glm::translate(glm::mat4(1.0f), position);
}

void RetroFuturaGUI::ITexture::SetRotation(const glm::vec3& rotation)
{
    const glm::vec3 radians = glm::radians(rotation);
    _rotationMatrix =
        glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
}

void RetroFuturaGUI::ITexture::setupQuad()
{
    if(!_projection)
        return;

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), reinterpret_cast<void*>(3 * sizeof(f32)));

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

glm::i32vec2 RetroFuturaGUI::ITexture::GetResolution() const
{
    return _resolution;
}

f32 RetroFuturaGUI::ITexture::GetAspectRatio() const
{
    return _aspectRatio;
}

bool RetroFuturaGUI::ITexture::IsTextureVerticallyFlipped() const
{
    return _verticallyFlipped;
}

u32 RetroFuturaGUI::ITexture::GetID() const
{
    return _id;
}