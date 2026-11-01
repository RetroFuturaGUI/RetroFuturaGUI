#pragma once
#include "FillStyle.hpp"

namespace RetroFuturaGUI
{
    class Text;

    /// @brief The border around every glyph of a text: the glyph's shape widened by the border's width, drawn behind the
    ///        glyph with a fill, raster and image of its own. Black until colors are set; the width is set on the Text.
    class TextBorder final : public FillStyle
    {
    public:
        TextBorder()
        {
            SetColors(std::span<const glm::vec4>(&_defaultColor, 1));
        }

        using FillStyle::SetColors;
        using FillStyle::SetFillType;
        using FillStyle::SetGradientOffset;
        using FillStyle::SetGradientAnimationSpeed;
        using FillStyle::SetGradientDegree;
        using FillStyle::SetGradientRotationSpeed;
        using FillStyle::GetGradientOffset;
        using FillStyle::GetGradientDegree;
        using FillStyle::SetBackgroundImageTextureID;
        using FillStyle::SetBackgroundImagePadding;
        using FillStyle::SetBackgroundImageFit;
        using FillStyle::SetMaskTextureID;
        using FillStyle::SetMaskMode;
        using FillStyle::SetMaskFit;
        using FillStyle::SetRasterPattern;
        using FillStyle::SetPrimaryRasterColor;
        using FillStyle::SetSecondaryRasterColor;
        using FillStyle::SetRasterSpacing;
        using FillStyle::SetRasterDegree;
        using FillStyle::SetPrimaryRasterWidthTransfer;
        using FillStyle::SetDotTransparencyTransfer;
        using FillStyle::SetRasterAnimationSpeed;

    private:
        friend class Text; //draws it

        static inline const glm::vec4 _defaultColor { 0.0f, 0.0f, 0.0f, 1.0f };
    };
}