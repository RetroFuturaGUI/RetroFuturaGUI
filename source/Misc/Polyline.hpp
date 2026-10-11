#pragma once
#include "config.hpp"
#include "IncludeHelper.hpp"
#include <span>
#include "Projection.hpp"

namespace RetroFuturaGUI
{
    /// @brief A line through a series of points
    class Polyline
    {
    public:
        /// @brief Constructs a Polyline that draws using the given projection.
        Polyline(Projection* projection);
        Polyline() = delete;
        Polyline(const Polyline&) = delete;
        Polyline(Polyline&&) = delete;
        ~Polyline();
        auto operator =(const Polyline&) = delete;
        auto operator =(Polyline&&) = delete;

        /// @brief Sets the points the line runs through, in its own space: -0.5..0.5 on both axes spans its size.
        /// They are copied to the GPU, so the span only has to stay valid for the call. Fewer than two points draw nothing.
        void SetPoints(std::span<const glm::vec2> points);

        /// @brief Sets the size the points' -0.5..0.5 space is stretched to. The thickness isn't stretched with it.
        void SetSize(const glm::vec3& size);

        /// @brief Sets the world position of the line's center.
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the line.
        void SetRotation(const glm::vec3& rotation);

        /// @brief Sets the line's color.
        void SetColor(const glm::vec4& color);

        /// @brief Returns the line's color.
        const glm::vec4& GetColor() const;

        /// @brief Sets the line's width, in pixels.
        void SetThickness(const f32 thickness);

        /// @brief Draws the line through the points last set.
        void Draw();

    private:
        void setupMesh();

    //Geometry
        u32
            _vao { 0 },
            _vbo { 0 };
        uSize
            _vertexCount { 0 },
            _bufferVertexCount { 0 };

        Projection& _projection;
        glm::mat4
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };

    //Design
        glm::vec4 _color { 1.0f };
        f32 _thickness { 2.0f };
    };
}