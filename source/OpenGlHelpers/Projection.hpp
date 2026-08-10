#pragma once
#include "config.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RetroFuturaGUI
{
    class Projection
    {
    public:
        /// @brief Constructs an orthographic projection for the given resolution.
        Projection(const f32 width, const f32 height);
        ~Projection() = default;

        /// @brief Returns the current projection matrix.
        const glm::mat4& GetProjectionMatrix() const;

        /// @brief Recomputes the projection matrix for a new resolution.
        void UpdateProjectionMatrix(const f32 width, const f32 height);

        /// @brief Returns the resolution the projection was last computed for.
        const glm::vec2& GetResolution() const;

        /// @brief Returns the depth used for the projection.
        const float& GetDepth() const { return _depth; }

    private:
        glm::mat4 _projectionMatrix  { 1.0f };
        glm::vec2 _resolution { 1.0f };
        float _depth { 1.0f };
        void setProjectionMatrix();
    };
}