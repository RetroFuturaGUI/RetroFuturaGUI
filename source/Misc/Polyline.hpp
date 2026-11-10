#pragma once
#include "config.hpp"
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Projection.hpp"
#include "ContextObjects.hpp"
#include "Shader.hpp"

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

        /// @brief Sets the glow drawn under the line: full at the line's edge and fading to nothing over width pixels. A width of
        ///        0, the default, or a fully transparent color draws none
        void SetGlow(const f32 width, const glm::vec4& color);

        /// @brief Draws the line through the points last set, over its glow. Each covers every pixel once, so where segments
        ///        overlap at a sharp turn a translucent line doesn't blend twice.
        void Draw();

    private:
        void setupMesh();

        /// @brief Draws the strip once, thickness pixels wide, each pixel at most once
        /// @param glowWidth Above 0, the strip is a glow around a line _thickness wide, fading out over that many pixels
        void drawPass(Shader& shader, const f32 thickness, const glm::vec4& color, const f32 glowWidth);

    //Geometry
        u32 _vbo { 0 };
        std::unique_ptr<VertexArray> _vertexArray { nullptr }; //one VAO per context that draws the line, over the same buffer
        uSize
            _vertexCount { 0 },
            _bufferVertexCount { 0 };

        Projection& _projection;
        glm::mat4
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };

    //Design
        glm::vec4
            _color { 1.0f },
            _glowColor { 0.0f };
        f32
            _thickness { 2.0f },
            _glowWidth { 0.0f };
    };
}