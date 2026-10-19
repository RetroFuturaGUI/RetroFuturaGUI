#include "Mesh.hpp"
#include "ShaderManager.hpp"

namespace RetroFuturaGUI
{
    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<MeshTexture> textures)
        : _vertices(std::move(vertices)), _indices(std::move(indices)), _textures(std::move(textures))
    {
        setupMesh();
    }

    Mesh::Mesh(Mesh&& other) noexcept
        : _vbo(other._vbo), _ebo(other._ebo), _vertexArray(std::move(other._vertexArray)),
          _vertices(std::move(other._vertices)), _indices(std::move(other._indices)), _textures(std::move(other._textures))
    {
        other._vbo = 0;
        other._ebo = 0;
    }

    Mesh& Mesh::operator=(Mesh&& other) noexcept
    {
        if (this == &other)
            return *this;

        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);

        _vbo = other._vbo;
        _ebo = other._ebo;
        _vertexArray = std::move(other._vertexArray);
        _vertices = std::move(other._vertices);
        _indices = std::move(other._indices);
        _textures = std::move(other._textures);

        other._vbo = 0;
        other._ebo = 0;

        return *this;
    }

    Mesh::~Mesh()
    {
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);
    }

    void Mesh::Draw(const glm::mat4& projection, const glm::mat4& position, const glm::mat4& rotation, const glm::mat4& scaling)
    {
        if(!_vertexArray)
            return;

        Shader& shader = ShaderManager::GetTextureFillShader();
        shader.UseProgram();
        shader.SetUniformInt("uTexture", 0);
        shader.SetUniformMat4("uProjection", projection);
        shader.SetUniformMat4("uPosition", position);
        shader.SetUniformMat4("uRotation", rotation);
        shader.SetUniformMat4("uScaling", scaling);

        bool boundTexture = false;
        for (const MeshTexture& meshTexture : _textures)
        {
            if (meshTexture._Type == "texture_diffuse" && meshTexture.texture)
            {
                meshTexture.texture->Bind(0);
                boundTexture = true;
                break;
            }
        }

        if (!boundTexture)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        _vertexArray->Bind();
        glDrawElements(GL_TRIANGLES, static_cast<i32>(_indices.size()), GL_UNSIGNED_INT, nullptr);
        VertexArray::Unbind();
    }

    void Mesh::setupMesh()
    {
        glGenBuffers(1, &_vbo);
        glGenBuffers(1, &_ebo);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<i64>(_vertices.size() * sizeof(Vertex)), _vertices.data(), GL_STATIC_DRAW);

        //Filled through GL_ARRAY_BUFFER, since the GL_ELEMENT_ARRAY_BUFFER binding belongs to whichever VAO is bound
        glBindBuffer(GL_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<i64>(_indices.size() * sizeof(u32)), _indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        _vertexArray = std::make_unique<VertexArray>([vbo = _vbo, ebo = _ebo]()
            {
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, _Position)));

                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, _TextureCoordinate)));
            });
    }
}