#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RetroFuturaGUI
{
    class Projection
    {
    public:
        Projection(const float width, const float height);
        ~Projection() = default;

        glm::mat4& GetProjectionMatrix();
        void UpdateProjectionMatrix(const float width, const float height);
        glm::vec2& GetResolution();

    private:
        glm::mat4 _projectionMatrix = glm::mat4(1.0f);
        glm::vec2 _resolution = glm::vec2(1.0f);

        void setProjectionMatrix();
    };
}