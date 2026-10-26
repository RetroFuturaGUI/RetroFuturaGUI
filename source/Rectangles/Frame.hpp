#pragma once
#include "IRectangle.hpp"
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief Identifies which side of a Frame a BorderGap sits on.
    enum class BorderEdge : u32
    {
        Top = 0,
        Right = 1,
        Bottom = 2,
        Left = 3
    };

    /// @brief Describes a section of a Frame's outline to skip drawing.
    struct BorderGap
    {
        BorderEdge _Edge { BorderEdge::Top };
        f32 _Offset { 0.0f };             // width of each solid segment
        f32 _Length { 0.0f };             // width of each gap segment
        bool _AnchorFarCorner { false };  // false: measure from the top/left corner of the edge. true: from the bottom/right corner
        i32 _Repeat { 1 };                /* 0: no gap at all (solid border)
                                            1: a single gap
                                            >1: amount of gaps in a repeating pattern (dashed/dotted lines)
                                            <0: the same repeating pattern, but tiled for the whole edge*/
    };

    /// @brief A rectangle's outline, such as a widget's border. It takes the fill, the corners, the glass and the raster,
    ///        and gaps cut out of the outline.
    class Frame final : public IRectangle
    {
    public:
        /// @brief Constructs a Frame that draws using the given projection.
        Frame(Projection* projection);

        /// @brief Draws the outline with its fill and features. Draws nothing until it holds a color.
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

        /// @brief Sets the width of the outline, in pixels.
        void SetBorderWidth(const f32 width);

        /// @brief Returns the width of the outline, in pixels.
        f32 GetBorderWidth() const;

        /// @brief Sets sections of the outline to skip drawing. One element expresses the pattern for one edge at a time.
        void SetBorderGaps(std::span<BorderGap> gaps);

    private:
        static constexpr uSize _maxBorderGapCount { 255 };
        f32 _borderWidth { 5.0f };
        std::vector<glm::vec4> _borderGapData {};
        uSize _borderGapCount { 0 };
    };
}
