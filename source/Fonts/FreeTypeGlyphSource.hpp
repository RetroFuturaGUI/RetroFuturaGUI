#pragma once
#include "IGlyphSource.hpp"
#include "Fonts.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
#include FT_SIZES_H
#include FT_MULTIPLE_MASTERS_H
#include <hb.h>
#include <hb-ft.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace RetroFuturaGUI
{
    /// @brief Glyphs rendered by FreeType from a font file (.ttf, .otf, one face of a .ttc). Sources drawing from the same
    ///        file share its FreeType face and each keep a size of their own on it, so a font extended by many Unicode blocks
    ///        or shown at many sizes is opened once
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
        bool HasGlyph(const u32 codePoint) const override;
        bool RenderGlyph(const u32 codePoint, GlyphBitmap* bitmap) override;
        bool RenderGlyphOutline(const u32 codePoint, const f32 width, GlyphBitmap* bitmap) override;
        bool Shape(std::u32string_view text, const ShapeDirection direction, std::vector<ShapedGlyph>* glyphs) override;
        bool RenderGlyphIndex(const u32 glyphIndex, GlyphBitmap* bitmap) override;
        bool RenderGlyphIndexOutline(const u32 glyphIndex, const f32 width, GlyphBitmap* bitmap) override;

    private:
        FreeTypeGlyphSource(FT_Face face, FT_Size size, std::string_view path, const u32 faceIndex);

        /// @brief The FreeType face of a font file's face, opened on first use and shared by every source drawing from it. A
        ///        variable font has one per weight, since its weight is set on the face
        /// @return nullptr when FreeType can't open the file
        static FT_Face getSharedFreeTypeFace(FT_Library library, std::string_view path, const u32 faceIndex, const PlatformBridge::Fonts::Weight weight);

        /// @brief Makes this source's size the shared face's active one, which rendering, shaping and setting the size all go by
        void activateSize() const;

        /// @brief The HarfBuzz face of a font file's face made on first use and shared by every source drawing from it, so
        ///        a big table (Segoe UI Emoji's COLR, megabytes) is read and checked once, not once per Unicode block.
        /// @return nullptr when the file can't be read.
        static hb_face_t* getSharedFace(std::string_view path, const u32 faceIndex);

        /// @brief Sets a variable font's weight axis to the weight, clamped to the axis; the other axes keep their defaults.
        static void setVariableWeight(FT_Library library, FT_Face face, const PlatformBridge::Fonts::Weight weight);

        /// @brief How glyphs are loaded, for rendering and for the shaping's advances alike.
        FT_Int32 getLoadFlags() const;

        /// @brief HarfBuzz's view of the face through FreeType made on first use after the size is set.
        hb_font_t* getShapingFont();

        /// @brief HarfBuzz's own reading of the face at the size set (which walks COLR version 1 paint) made on first use.
        hb_font_t* getPaintFont();

        /// @brief Copies FreeType's bitmap into the glyph's tight rows: gray as it is, one bit per pixel widened to a byte,
        ///        BGRA (color layers and color bitmaps) turned into RGBA.
        /// @return false for a pixel layout no font of ours renders.
        static bool copyBitmap(const FT_Bitmap& rendered, GlyphBitmap* bitmap);

        FT_Face _face { nullptr }; //shared, kept until the program ends
        FT_Size _size { nullptr }; //this source's own size on the face
        f32 _pixelSize { 0.0f };
        hb_font_t* _shapingFont { nullptr }; //HarfBuzz's view of the face, made on the first shaping
        hb_buffer_t* _shapingBuffer { nullptr }; //reused by every shaping
        hb_font_t* _paintFont { nullptr }; //64 per pixel, like FreeType's outlines, so painted glyphs line up with them
        std::string _path {}; //the font file and its face which the shared HarfBuzz face is found by
        u32 _faceIndex { 0 };
        static inline std::unordered_map<std::string, hb_face_t*> _sharedFaces {}; //by path and face index, kept until the program ends
        static inline std::unordered_map<std::string, FT_Face> _sharedFreeTypeFaces {}; //by path and face index, a variable font's by its weight too
        static inline std::unordered_set<std::string> _variableFontFiles {}; //path and face index of the variable fonts opened so far
    };
}