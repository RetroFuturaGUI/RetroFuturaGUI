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

        const glm::mat4& GetProjectionMatrix() const;
        void UpdateProjectionMatrix(const f32 width, const f32 height);
        const glm::vec2& GetResolution() const;

    private:
        glm::mat4 _projectionMatrix  { 1.0f };
        glm::vec2 _resolution { 1.0f };
        void setProjectionMatrix();
    };
}