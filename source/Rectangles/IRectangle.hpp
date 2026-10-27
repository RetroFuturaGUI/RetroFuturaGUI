#pragma once
#include "IncludeHelper.hpp"
#include "ContextObjects.hpp"
#include "FillStyle.hpp"
#include "Shader.hpp"
#include "Projection.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    /// @brief The base of every rectangle: its size, position, rotation and corners, and the quad all rectangles share. The
    ///        fill, the glass and the raster come from FillStyle, and Plane and Frame make them public.
    class IRectangle : public FillStyle
    {
    public:
        /// @brief Constructs a rectangle that draws using the given projection.
        IRectangle(Projection* projection);
        virtual ~IRectangle();
        IRectangle(const IRectangle&) = delete;
        IRectangle(IRectangle&&) = delete;
        auto operator=(const IRectangle&) = delete;
        auto operator=(IRectangle&&) = delete;

        /// @brief Draws the rectangle.
        virtual void Draw() = 0;

        /// @brief Sets the size of the rectangle.
        void SetSize(const glm::vec2& size);

        /// @brief Sets the world position of the rectangle.
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the rectangle.
        void SetRotation(const glm::vec3& rotation);

        /// @brief Returns the current size of the rectangle.
        const glm::vec2& GetSize() const;

        /// @brief Returns the current world position of the rectangle.
        const glm::vec3& GetPosition() const;

        /// @brief Returns the current world rotation of the rectangle.
        const glm::vec3& GetRotation() const;

        /// @brief Returns the projection the rectangle draws with.
        Projection& GetProjection() const;

    protected:
        /// @brief Sets the per-corner rounding radii. Any radius above 0 rounds the corners. Public on Plane and Frame.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Uploads the projection, the three matrices, the size in pixels, and the corner radii when the corners are
        ///        rounded.
        void uploadTransform(Shader& shader) const;

        /// @brief Draws the shared quad with the shader in use.
        static void drawQuad();

    //Geometry
        Projection& _projection;
        glm::mat4
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
        glm::vec2 _scale { 0.01f };
        glm::vec3
            _position { 0.0f },
            _rotation { 0.0f };
        glm::vec4 _cornerRadii { 0.0f };

    private:
    //The quad every rectangle draws, made with the first rectangle and deleted with the last
        static constexpr f32 _quadVertices[3 * 4] =
        {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        static constexpr u32 _quadIndices[3 * 2] =
        {
            0, 1, 2,
            2, 3, 0
        };

        static inline u32
            _quadVbo { 0 },
            _quadEbo { 0 };
        static inline std::unique_ptr<VertexArray> _quadVertexArray { nullptr }; //one VAO per context, over the same buffers
        static inline uSize _rectangleCount { 0 };

        static void createQuad();
        static void deleteQuad();
    };
}
