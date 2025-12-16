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
        glm::mat4 _projectionMatrix  { 1.0f };
        glm::vec2 _resolution { 1.0f };
        void setProjectionMatrix();
    };
}