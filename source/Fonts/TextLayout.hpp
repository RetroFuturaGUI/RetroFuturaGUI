#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    enum class TextAlignment: u32
    {
        Left,
        Right,
        Center,
        //Block
    };

    /// @brief How a text's lines run. Vertical lines are columns read top to bottom, one em per glyph, the glyphs upright
    enum class TextFlow : u32
    {
        Horizontal,
        VerticalLeftToRight, // the first column leftmost, as Latin or Mongolian vertical text runs
        VerticalRightToLeft // the first column rightmost, as Japanese and Chinese vertical text runs
    };

    /// @brief What a text's mask is laid over
    enum class MaskSpan : u32
    {
        WholeText,
        EachLine
    };
}