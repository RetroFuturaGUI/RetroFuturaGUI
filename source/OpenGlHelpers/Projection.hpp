#pragma once
#include "config.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RetroFuturaGUI
{
    class Projection
    {
    public:
        Projection(const f32 width, const f32 height);
        ~Projection() = default;

        glm::mat4& GetProjectionMatrix();
        void UpdateProjectionMatrix(const f32 width, const f32 height);
        glm::vec2& GetResolution();

    private:
        glm::mat4 _projectionMatrix = glm::mat4(1.0f);
        glm::vec2 _resolution = glm::vec2(1.0f);
        void setProjectionMatrix();
    };

    struct GeometryParams2D
    {
        const Projection& _Projection;
        const glm::vec2& _Position;
        const glm::vec2& _Size;
        f32 _Rotation;
    };
}