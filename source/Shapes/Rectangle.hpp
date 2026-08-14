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
        GlassEffectWithImage = GlassEffect + (1 << 2),
        DottedPattern = 1 << 3,
        FogEffect = 1 << 4
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

        /// @brief Sets the dot color used for the DottedPattern shader feature. Alpha blends the dots over whatever the shader has already drawn.
        void SetDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the DottedPattern shader feature.
        void SetDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which dot radii are sampled from DotRadiusTransfer and along which the pattern animates.
        void SetDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, sampled along the DotSizeTransferDegree direction across the rectangle. Enables the DottedPattern shader feature when non-empty.
        void SetDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each dot's opacity reaches from its center before fading to transparent. 1.0 = fully opaque disc, 0.5 = opaque inner half then fades out, 0.0 = only the center pixel is opaque.
        void SetDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the dotted pattern animates along the DotSizeTransferDegree direction.
        void SetDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the FogEffect shader feature.
        void SetFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the fog drifts over time.
        void SetFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the fog's fractal noise. Enables the FogEffect shader feature when non-empty.
        void SetFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which fog appears; higher values carve larger clear (fog-free) gaps out of the cloud.
        void SetFogClearing(const f32 clearing);

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

        // Dotted pattern (ShaderFeatures::DottedPattern)
        std::span<f32> _dotRadiusTransfer;
        i32 _dotRadiusTransferCount { 0 };
        glm::vec4 _dotColor { 1.0f };
        f32
            _dotDistance { 15.0f },
            _dotSizeTransferDegree { 45.0f },
            _dotTransparencyTransfer { 1.0f },
            _dotAnimationSpeed { 0.0f },
            _dotAnimationOffset { 0.0f };
        RectangleMode _rectangleMode { RectangleMode::Plane };

        // Fog (ShaderFeatures::FogEffect)
        std::span<f32> _fogDensity;
        i32 _fogDensityCount { 0 };
        f32
            _fogAlpha { 0.0f },
            _fogSpeed { 0.0f },
            _fogClearing { 0.5f },
            _fogAnimationOffset { 0.0f };

        void setupMesh();
        void initColors(std::span<glm::vec4> colors);
        void uploadDotUniforms(Shader& shader);
        void uploadFogUniforms(Shader& shader);
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