#pragma once
#include "IncludeHelper.hpp"
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief How a rendered glyph's pixels are laid out.
    enum class GlyphPixelFormat : u32
    {
        Coverage, // one byte per pixel: how much of the pixel the glyph covers
        Color // four bytes per pixel, red, green, blue and alpha, the colors premultiplied by alpha: a color emoji keeps its own colors
    };

    /// @brief One glyph rendered by a glyph source, rows packed tightly.
    struct GlyphBitmap
    {
        std::vector<u8> _Pixels {};
        u32 _Width { 0 },
            _Height { 0 },
            _Advance { 0 }; // how far the pen moves on after the glyph, in pixels
        i32 _BearingX { 0 }, // from the pen to the bitmap's left edge
            _BearingY { 0 }; // from the baseline up to the bitmap's top edge
        GlyphPixelFormat _Format { GlyphPixelFormat::Coverage };
    };

    /// @brief Which way shaped glyphs run.
    enum class ShapeDirection : u32
    {
        LeftToRight,
        RightToLeft,
        TopToBottom
    };

    /// @brief One glyph of shaped text: which glyph of the font, which character it starts at, and how far it moves the pen.
    struct ShapedGlyph
    {
        u32 _GlyphIndex { 0 }, // the glyph in the font, not a character: one glyph may draw several (an emoji sequence, a ligature)
            _Cluster { 0 }; // the first character it draws, counted in the text shaped
        f32 _AdvanceX { 0.0f }, // pixels at the source's size, y up
            _AdvanceY { 0.0f },
            _OffsetX { 0.0f }, // from the pen to the glyph's origin
            _OffsetY { 0.0f };
    };

    /// @brief A font's line metrics at the size its source is set to, in pixels.
    struct FontMetrics
    {
        f32 _Ascender { 0.0f }, // from the baseline up to the top of the tallest glyphs
            _Descender { 0.0f }, // from the baseline down to the bottom of the lowest glyphs, positive
            _LineHeight { 0.0f }; // from one baseline to the next: ascender, descender and the font's gap between lines
    };

    /// @brief Where a font's glyphs come from: a scalable font file rendered at any size (FreeType), or later a bitmap font
    ///        offering a few fixed sizes. FontManager packs whatever it renders into its glyph pages.
    class IGlyphSource
    {
    public:
        virtual ~IGlyphSource() = default;

        /// @brief Sets the size the next glyphs are rendered at. A source with fixed sizes takes the closest one it offers.
        /// @return false when the source can't render at all.
        virtual bool SetPixelSize(const f32 pixels) = 0;

        /// @brief Returns the size glyphs are rendered at now, which differs from the size asked for when only fixed sizes are offered.
        virtual f32 GetPixelSize() const = 0;

        /// @brief Returns the sizes the source offers, in pixels, smallest first. Empty for a source rendering at any size.
        virtual std::vector<f32> GetPixelSizes() const = 0;

        /// @brief Returns the line metrics at the size set.
        virtual FontMetrics GetMetrics() const = 0;

        /// @brief Returns whether some glyphs come out in color (GlyphPixelFormat::Color), as an emoji font's do.
        virtual bool HasColorGlyphs() const = 0;

        /// @brief Returns whether the source has a glyph of its own for a code-point
        virtual bool HasGlyph(const u32 codePoint) const = 0;

        /// @brief Renders one glyph at the size set. A code point the font lacks renders its "missing" glyph.
        /// @return false when rendering failed; the bitmap is then empty.
        virtual bool RenderGlyph(const u32 codePoint, GlyphBitmap* bitmap) = 0;

        /// @brief Renders a glyph's shape widened by width pixels on every side, which a text border draws behind the glyph.
        /// @return false when the glyph has no outline to widen, as in a bitmap font; the bitmap is then empty.
        virtual bool RenderGlyphBorder(const u32 codePoint, const f32 width, GlyphBitmap* bitmap) = 0;

        /// @brief Shapes a run of text into the font's glyphs, in the order they're drawn
        /// @return false when the source can't shape
        virtual bool Shape(std::u32string_view text, const ShapeDirection direction, std::vector<ShapedGlyph>* glyphs) = 0;

        /// @brief Renders a glyph found by shaping, by its index in the font, as RenderGlyph renders one by its character.
        virtual bool RenderGlyphIndex(const u32 glyphIndex, GlyphBitmap* bitmap) = 0;

        /// @brief Renders a glyph found by shaping widened for a text border, as RenderGlyphBorder renders one by its character.
        virtual bool RenderGlyphIndexBorder(const u32 glyphIndex, const f32 width, GlyphBitmap* bitmap) = 0;
    };
}