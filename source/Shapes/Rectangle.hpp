#pragma once
#include "ShaderManager.hpp"
#include "Projection.hpp"
#include <span>
#include "Image.hpp"

namespace RetroFuturaGUI
{
    enum ShaderFeatures : u32
    {
        RoundedCorners = 1,
        GlassEffect = 1 << 1,
        GlassEffectWithImage = GlassEffect + (1 << 2)
    };

    enum class RectangleMode : u32
    {
        Plane,
        Border
    };

    class Rectangle
    {
    public:
        /// @brief Constructs a Rectangle that draws using the given projection.
        Rectangle(Projection* projection);
        ~Rectangle();

        /// @brief Draws the rectangle with its currently configured fill/border and shader features.
        void Draw();

        /// @brief Sets the size of the rectangle.
        void SetSize(const glm::vec2& size);

        /// @brief Sets the world position of the rectangle.
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the rectangle.
        void SetRotation(const glm::vec3& rotation);

        /// @brief Sets the colors used for fill/border, interpreted according to the current FillType.
        void SetColors(std::span<glm::vec4> colors);

        /// @brief Sets the per-corner rounding radii, used when the RoundedCorners shader feature is enabled.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets the offset applied to the gradient's start position.
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the gradient animates over time.
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of a linear gradient, in degrees.
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the gradient rotates over time.
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the enabled ShaderFeatures bitmask.
        /// @param reset If true, replaces the current feature set; if false, ORs the given features in.
        void SetShaderFeatures(const u32 features, const bool reset = true);

        /// @brief Sets the texture ID sampled for the GlassEffectWithImage shader feature.
        void SetWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the border width, used when RectangleMode::Border is active.
        void SetBorderWidth(const f32 width);

        /// @brief Sets whether the rectangle draws a filled plane or just its border.
        void SetRectangleMode(const RectangleMode rectanlgeMode);

        /// @brief Sets the fill type (solid, linear/radial/huestar gradient).
        void SetFillType(const FillType fillType);

        /// @brief Returns the current size of the rectangle.
        const glm::vec2 GetSize() const;

        /// @brief Returns the current world position of the rectangle.
        const glm::vec3 GetPosition() const;

    private:
    //Geometry
        f32 _vertices[3 * 4] = 
        {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        u32 _indices[3 * 2] = 
        {
            0, 1, 2,
            2, 3, 0
        };
        
        u32 
            _vao { 0 },
            _vbo { 0 },
            _ebo { 0 };

        Projection& _projection;
        glm::mat4 
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
        glm::vec2 _scale { 0.01f };
        glm::vec3 
            _position { 0.0f },
            _rotation { 0.0f };
        glm::vec4 _vertexPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        // Fill
        std::span<glm::vec4> _colors; //this considers the color source may change many during runtime
        FillType _fillType { FillType::SOLID };
        i32 _colorCount { 0 };
        f32
            _gradientOffset { 0.0f },
            _gradientAnimationSpeed { 0.0003f },
            _gradientDegree { 45.0f },
            _gradientRotationSpeed { 0.02f };
        glm::vec4 _cornerRadii { 0.0f };
        u32 _shaderFeatureDIP { 0 };
        u32 _windowBackgroundTextureID { 0 };
        f32 _borderWidth { 5.0f };
        RectangleMode _rectangleMode { RectangleMode::Plane };
        
        void setupMesh();
        void initColors(std::span<glm::vec4> colors);
        void drawWithSolidFill();
        void drawRadialGradientFill();
        void drawHueStarGradientFill();
        void drawLinearGradientFill();
        void drawSolidBorder();
        void drawLinearGradientBorder();
        void drawRadialGradientBorder();
        void drawHueStarGradientBorder();
    };
}