#pragma once
#include "IRectangle.hpp"
#include "FogStyle.hpp"

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
    class Plane : public IRectangle, public FogStyle
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
        using IRectangle::SetRasterSpacing;
        using IRectangle::SetRasterDegree;
        using IRectangle::SetPrimaryRasterWidthTransfer;
        using IRectangle::SetDotTransparencyTransfer;
        using IRectangle::SetRasterAnimationSpeed;

    //Background gaps
        /// @brief Sets sections of the fill to skip drawing.
        void SetBackgroundGaps(const BackgroundGap& gaps);

    //Fog, from FogStyle
        using FogStyle::SetFogAlpha;
        using FogStyle::SetFogSpeed;
        using FogStyle::SetFogDensity;
        using FogStyle::SetFogClearing;

    //Image, from IRectangle
        using IRectangle::SetBackgroundImageTextureID;
        using IRectangle::SetBackgroundImagePadding;

    protected:
        /// @brief Makes the shader current and uploads everything a plane's shader reads.
        void uploadPlane(Shader& shader);

    private:
        glm::vec4 _backgroundGapData { 0.0f };
    };
}