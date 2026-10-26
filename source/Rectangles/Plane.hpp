#pragma once
#include "IRectangle.hpp"

namespace RetroFuturaGUI
{
    /// @brief Describes sections of a Plane's fill to skip drawing
    struct BackgroundGap
    {
        f32
            _Offset { 0.0f },   // width of each solid segment
            _Length { 0.0f },   // width of each gap segment
            _Degree { 0.0f };   // rotates the pattern's axis: 0 runs along X, 90 along Y
        i32 _Repeat { 1 };      /* 0: no gap at all (solid background)
                                    1: a single gap
                                    >1: amount of gaps in a repeating pattern (dashed/dotted lines)
                                    <0: the same repeating pattern, but tiled for the whole background*/
    };

    /// @brief A filled rectangle, such as a widget's background, a highlight or a track. Besides the fill, the corners, the
    ///        glass and the raster it takes fog, gaps cut out of the fill, and an image of its own.
    class Plane : public IRectangle
    {
    public:
        /// @brief Constructs a Plane that draws using the given projection.
        Plane(Projection* projection);

        /// @brief Draws the plane with its fill and features. Draws nothing until it holds a color.
        void Draw() override;

    //Fill, corners, glass and raster, from IRectangle
        using IRectangle::SetColors;
        using IRectangle::SetFillType;
        using IRectangle::SetCornerRadii;
        using IRectangle::SetGradientOffset;
        using IRectangle::SetGradientAnimationSpeed;
        using IRectangle::SetGradientDegree;
        using IRectangle::SetGradientRotationSpeed;
        using IRectangle::GetGradientOffset;
        using IRectangle::GetGradientDegree;
        using IRectangle::SetWindowBackgroundImageTextureID;
        using IRectangle::SetRasterPattern;
        using IRectangle::SetPrimaryRasterColor;
        using IRectangle::SetSecondaryRasterColor;
        using IRectangle::SetDotDistance;
        using IRectangle::SetRasterDegree;
        using IRectangle::SetPrimaryRasterWidthTransfer;
        using IRectangle::SetDotTransparencyTransfer;
        using IRectangle::SetRasterAnimationSpeed;

    //Background gaps
        /// @brief Sets sections of the fill to skip drawing.
        void SetBackgroundGaps(const BackgroundGap& gaps);

    //Fog
        /// @brief Sets the overall opacity of the fog.
        void SetFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the fog drifts over time.
        void SetFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the fog's fractal noise. Turns the fog on when non-empty.
        void SetFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which fog appears; higher values carve larger clear (fog-free) gaps out of the cloud.
        void SetFogClearing(const f32 clearing);

    //Image
        /// @brief Sets an image of the plane's own, stretched over it behind the fill and inset by SetBackgroundImagePadding.
        /// @note Unlike SetWindowBackgroundImageTextureID, which shows a window-wide backdrop behind the glass, this belongs to
        ///       the plane alone. The two can be shown together: the image sits in front of the glass.
        void SetBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets how far the background image stays clear of the plane's edges, per side in pixels.
        void SetBackgroundImagePadding(const f32 padding);

    protected:
        /// @brief Makes the shader current and uploads everything a plane's shader reads.
        void uploadPlane(Shader& shader);

    private:
        glm::vec4 _backgroundGapData { 0.0f };

    //Fog (ShaderFeatures::FogEffect)
        std::span<f32> _fogDensity;
        uSize _fogDensityCount { 0 };
        f32
            _fogAlpha { 0.0f },
            _fogSpeed { 0.0f },
            _fogClearing { 0.5f },
            _fogAnimationOffset { 0.0f };

    //Image (ShaderFeatures::BackgroundImage)
        u32 _backgroundImageTextureID { 0 };
        f32 _backgroundImagePadding { 0.0f };

        void uploadFog(Shader& shader);
        void uploadImage(Shader& shader) const;

        /// @brief The background image's padding as a fraction of the quad, ready for uImagePadding.
        glm::vec2 getBackgroundImagePaddingUV() const;
    };
}
