#pragma once
#include "IGlyphSource.hpp"
#include "Fonts.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
#include FT_MULTIPLE_MASTERS_H
#include <hb.h>
#include <hb-ft.h>
#include <memory>
#include <string_view>

namespace RetroFuturaGUI
{
    /// @brief Glyphs rendered by FreeType from a font file (.ttf, .otf, one face of a .ttc).
    class FreeTypeGlyphSource final : public IGlyphSource
    {
    public:
        /// @brief Opens one face of a font file. A variable font is set to the given weight, as far as its weight axis reaches;
        ///        other fonts draw the weight they were made in.
        /// @return nullptr when FreeType can't open it.
        static std::unique_ptr<FreeTypeGlyphSource> Open(FT_Library library, std::string_view path, const u32 faceIndex, const PlatformBridge::Fonts::Weight weight);

        ~FreeTypeGlyphSource() override;
        FreeTypeGlyphSource(const FreeTypeGlyphSource&) = delete;
        FreeTypeGlyphSource& operator=(const FreeTypeGlyphSource&) = delete;

        bool SetPixelSize(const f32 pixels) override;
        f32 GetPixelSize() const override;
        std::vector<f32> GetPixelSizes() const override;
        FontMetrics GetMetrics() const override;
        bool HasColorGlyphs() const override;
        bool RenderGlyph(const u32 codePoint, GlyphBitmap* bitmap) override;
        bool RenderGlyphBorder(const u32 codePoint, const f32 width, GlyphBitmap* bitmap) override;
        bool Shape(std::u32string_view text, const ShapeDirection direction, std::vector<ShapedGlyph>* glyphs) override;
        bool RenderGlyphIndex(const u32 glyphIndex, GlyphBitmap* bitmap) override;
        bool RenderGlyphIndexBorder(const u32 glyphIndex, const f32 width, GlyphBitmap* bitmap) override;

    private:
        explicit FreeTypeGlyphSource(FT_Face face);

        /// @brief Sets a variable font's weight axis to the weight, clamped to the axis; the other axes keep their defaults.
        static void setVariableWeight(FT_Library library, FT_Face face, const PlatformBridge::Fonts::Weight weight);

        /// @brief How glyphs are loaded, for rendering and for the shaping's advances alike.
        FT_Int32 getLoadFlags() const;

        /// @brief Copies FreeType's bitmap into the glyph's tight rows: gray as it is, one bit per pixel widened to a byte,
        ///        BGRA (color layers and color bitmaps) turned into RGBA.
        /// @return false for a pixel layout no font of ours renders.
        static bool copyBitmap(const FT_Bitmap& rendered, GlyphBitmap* bitmap);

        FT_Face _face { nullptr };
        f32 _pixelSize { 0.0f };
        hb_font_t* _shapingFont { nullptr }; //HarfBuzz's view of the face, made on the first shaping
        hb_buffer_t* _shapingBuffer { nullptr }; //reused by every shaping
    };
}