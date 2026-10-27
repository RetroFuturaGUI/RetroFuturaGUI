#pragma once
#include "IncludeHelper.hpp"
#include "Shader.hpp"
#include <span>

namespace RetroFuturaGUI
{
    enum class RasterPattern : u32
    {
        Dotted,
        Checkered
        //Striped
        //Grid
    };

    /// @brief The fill and what is laid over it
    class FillStyle
    {
    protected:
        FillStyle() = default;
        ~FillStyle() = default;

        /// @brief Sets the colors of the fill
        void SetColors(std::span<const glm::vec4> colors);

        /// @brief Sets the fill type (solid, linear shift, radial or hue star gradient).
        void SetFillType(const FillType fillType);

        /// @brief Sets the offset applied to the gradient's start position.
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the gradient animates over time.
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of a linear shift in degrees.
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the gradient rotates over time
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Returns the offset of the gradient's start position.
        f32 GetGradientOffset() const;

        /// @brief Returns the angle of the gradient, in degrees.
        f32 GetGradientDegree() const;

        /// @brief Sets the window-wide backdrop the glass shows, which turns the glass on. 0 turns it off.
        void SetWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets which pattern the raster draws.
        void SetRasterPattern(const RasterPattern rasterPattern);

        /// @brief Sets the primary raster color: the dot color for Dotted, and one of the two square colors for Checkered. Alpha blends over whatever the shader has already drawn.
        void SetPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the second of the two square colors used by the Checkered raster pattern. Alpha blends over whatever the shader has already drawn.
        void SetSecondaryRasterColor(const glm::vec4& color);

        /// @brief Sets the gap, in pixels, between the raster elements: 0 lets them touch
        void SetRasterSpacing(const f32 spacing);

        /// @brief Sets the direction in degrees along which raster widths
        void SetRasterDegree(const f32 degree);

        /// @brief Sets the per-position raster width curve in pixels sampled along the RasterDegree direction across what is drawn. Each value is a dot's full width across for Dotted, and a square's side length for Checkered, each element sized by the value at its center. The largest value plus RasterSpacing sets the grid. Turns the raster on when non-empty.
        void SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each dot's opacity reaches from its center before fading to transparent. 1.0 = fully opaque disc, 0.5 = opaque inner half then fades out, 0.0 = only the center pixel is opaque.
        void SetDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the raster pattern animates along the RasterDegree direction.
        void SetRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Uploads the feature bits with extraFeatures added, the fill, and the raster and the glass when they are on. A
        ///        gradient moves on by one step. Needs at least one color.
        void uploadFill(Shader& shader, const u32 extraFeatures);

    //Fill
        std::span<const glm::vec4> _colors; //this considers the color source may change many during runtime
        FillType _fillType { FillType::Solid };
        uSize _colorCount { 0 };
        f32
            _gradientOffset { 0.0f },
            _gradientAnimationSpeed { 0.0003f },
            _gradientDegree { 45.0f },
            _gradientRotationSpeed { 0.02f };
        u32
            _shaderFeatureDIP { 0 },
            _windowBackgroundTextureID { 0 };

    //Raster (ShaderFeatures::Raster) - the transfer, the spacing, the colors, the angle and the animation serve every
    //pattern; _dotTransparencyTransfer is Dotted's own.
        RasterPattern _rasterPattern { RasterPattern::Dotted };
        std::span<f32> _primaryRasterWidthTransfer;
        uSize _primaryRasterWidthTransferCount { 0 };
        glm::vec4
            _primaryRasterColor { 1.0f },
            _secondaryRasterColor { 1.0f };
        f32
            _rasterSpacing { 0.0f },
            _rasterDegree { 45.0f },
            _dotTransparencyTransfer { 1.0f },
            _rasterAnimationSpeed { 0.0f },
            _rasterAnimationOffset { 0.0f };

    private:
        void uploadRaster(Shader& shader);
        void uploadDotUniforms(Shader& shader) const;
        void uploadCheckeredUniforms(Shader& shader) const;
    };
}