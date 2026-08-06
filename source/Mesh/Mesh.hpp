#pragma once
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4201)
#endif
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "config.hpp"
#include "Texture.hpp"

namespace RetroFuturaGUI
{
    struct Vec3
    {
        union
        {
            struct
            {
                f32 _X;
                f32 _Y;
                f32 _Z;
            };

            struct
            {
                f32 _R;
                f32 _G;
                f32 _B;
            };

            struct
            {
                f32 _S;
                f32 _T;
                f32 _P;
            };

            f32 _Data[3];
        };

        Vec3() : _X(0.0f), _Y(0.0f), _Z(0.0f) {}
        Vec3(f32 x, f32 y, f32 z) : _X(x), _Y(y), _Z(z) {}
    };

    struct Vec2
    {
        union
        {
            struct
            {
                f32 _X;
                f32 _Y;
            };

            struct
            {
                f32 _S;
                f32 _T;
            };

            f32 _Data[2];
        };

        Vec2() : _X(0.0f), _Y(0.0f) {}
        Vec2(f32 x, f32 y) : _X(x), _Y(y) {}
    };

    struct Vertex
    {
        Vec3 _Position;
        Vec3 _Normal;
        Vec2 _TextureCoordinate;
    };

    // Associates one of a mesh's material slots with a Texture. The Texture itself is owned
    // elsewhere (Model keeps it alive for as long as any mesh referencing it exists) - this
    // is a non-owning view used purely to pick which texture to bind when drawing.
    struct MeshTexture
    {
        Texture* texture { nullptr };
        std::string type;
        std::string path;
    };

    class Mesh
    {
    public:
        Mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<MeshTexture> textures);
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        ~Mesh();

        void Draw(const glm::mat4& projection, const glm::mat4& position, const glm::mat4& rotation, const glm::mat4& scaling);

    private:
        u32 _vao { 0 };
        u32 _vbo { 0 };
        u32 _ebo { 0 };
        std::vector<Vertex> _vertices;
        std::vector<u32> _indices;
        std::vector<MeshTexture> _textures;

        void setupMesh();
    };
}

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif
