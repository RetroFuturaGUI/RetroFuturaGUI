#pragma once
#include "config.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>

namespace RetroFuturaGUI
{
    /// @brief How a Projection shows depth
    enum class ProjectionMode : u32
    {
        Orthographic,   // flat: depth never changes a size, so pixels stay pixels at any depth
        Perspective     // what lies further back shrinks toward the vanishing point. The screen plane (z = 0) stays pixel-exact
    };

    class Projection
    {
    public:
        /// @brief Constructs an orthographic projection for the given resolution
        Projection(const f32 width, const f32 height);
        ~Projection() = default;

        /// @brief Returns the current projection matrix (the view included). Shaders take it as uProjection
        const glm::mat4& GetProjectionMatrix() const;

        /// @brief Recomputes the projection matrix for a new resolution.
        void UpdateProjectionMatrix(const f32 width, const f32 height);

        /// @brief Returns the resolution the projection was last computed for.
        const glm::vec2& GetResolution() const;

        /// @brief Returns the depth layouts spread over such as Lasagna's layers. 1000 px orthographic, 1 px under
        ///        perspective, so stacking stays flat there and only what is meant to be 3D shows depth
        f32 GetDepth() const { return _depth; }

        /// @brief Shows depth in perspective. The eye sits viewingDistance pixels in front of the screen plane looking at the
        ///        vanishing point (bottom-up window pixels), or at the middle of the resolution following it, when none is given
        void SetPerspective(const f32 viewingDistance, const std::optional<glm::vec2>& vanishingPoint);

        /// @brief Shows depth flat again (default)
        void SetOrthographic();

        /// @brief Returns how the projection shows depth
        ProjectionMode GetMode() const;

        /// @brief Returns how far in front of the screen plane the eye sits under perspective (in pixels)
        f32 GetViewingDistance() const;

        /// @brief Returns where the eye sits under perspective in window pixels in front of the vanishing point. Nothing when
        ///        orthographic. The eye then looks along -z from infinitely far away
        std::optional<glm::vec3> GetEyePosition() const;

        /// @brief The direction from a point of the layout toward the eye: to the eye itself in perspective, the direction it
        ///        looks from when orthographic. Read from the projection matrix, so it holds through an EnvironmentView's camera too
        glm::vec3 GetTowardEye(const glm::vec3& point) const;

    private:
        friend class Environment; //draws its widgets through an EnvironmentView's camera

        static constexpr f32
            _orthographicDepth { 1000.0f },
            _perspectiveDepth { 1.0f };

        glm::mat4 _projectionMatrix  { 1.0f };
        glm::vec2 _resolution { 1.0f };
        f32 _depth { _orthographicDepth };
        ProjectionMode _mode { ProjectionMode::Orthographic };
        f32 _viewingDistance { 1000.0f };
        std::optional<glm::vec2> _vanishingPoint {}; //the middle of the resolution
        std::optional<glm::mat4> _viewOverride {}; //an EnvironmentView's camera, while it draws a world through this projection
        void setProjectionMatrix();

        /// @brief The 4D vector at right angles to all three. Of a projection's x, y and w rows, it is the eye: the one point
        ///        the projection sends nowhere, at infinity (w 0) when it is orthographic
        static glm::vec4 crossProduct(const glm::vec4& first, const glm::vec4& second, const glm::vec4& third);
    };
}