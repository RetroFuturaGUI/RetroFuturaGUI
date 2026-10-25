#pragma once
#include "IncludeHelper.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>
#include <map>
#include <memory>
#include <list>
#include <vector>
#include "UnicodeBlocks.hpp"
#include "Fonts.hpp"

namespace RetroFuturaGUI
{
    /*enum FontStyle : u32
    {
        REGULAR = 1,
        BOLD = 1 << 1,
        ITALIC = 1 << 2
    };*/

    struct Glyph
    {
        f32 _UV[4] { 0.0f, 0.0f, 0.0f, 0.0f }; // minU, minV, maxU, maxV
        i32 _Size[2] { 0, 0 };
        i32 _Bearing[2] { 0, 0 };
        u32 _Advance { 0 },
            _CodePoint { 0 },
            _OriginX { 0 },
            _OriginY { 0 },
            _EndX { 0 },
            _EndY { 0 },
            _BearingX { 0 },
            _BearingY { 0 },
            _TextureID { 0 }; // the atlas or page holding the glyph; a block loaded glyph by glyph spreads over several
    };

    /// @brief One texture of a block whose glyphs are rendered when they are first shown, filled row by row.
    struct GlyphPage
    {
        u32 _TextureID { 0 },
            _PenX { 0 }, // where the next glyph goes
            _PenY { 0 },
            _RowHeight { 0 }; // the tallest glyph in the row being filled
    };

    struct GlyphBlock
    {
        u32 _TextureID { 0 }, // a block made at once has one atlas; a block loaded glyph by glyph has pages instead
            _Width { 0 },
            _Height { 0 },
            _CodePointFirst { 0 },
            _CodePointLast { 0 };
        bool _LoadIndividually { false }; // a range of more than 256 code points: each glyph is rendered when first shown
        std::unordered_map<u32, Glyph> _Glyphs; // keyed by codepoint; a glyph keeps its address when others are added
        FT_Face _Face { nullptr }; // open while glyphs are still rendered on first use, until the program ends
        std::vector<GlyphPage> _Pages {};
        u32 _PageSide { 0 }; //  width and height
    };

    struct GlyphAtlas
    {
        f32 _FontSize { 16.0f };
        std::unordered_map<u32, GlyphBlock> _GlyphBlocks; // keyed by first codepoint
    };

    struct FontInfo
    {
        PlatformBridge::Fonts::FontProperty _FontProperty {};
        std::unordered_map<u32, GlyphAtlas> _Atlasses; // keyed by font size
    };

    class FontManager
    {
    private:
        enum LoadFontResult : i32
        {
            Success = 0,
            FontAlreadyLoaded = 1,
            FreeTypeError = 2,
            FontNotFound = 3
        };

    public:
        /// @brief Initializes the FontManager and the underlying FreeType library.
        /// @return 0 on success, or a non-zero FreeType error code.
        static i32 Init();

        /// @brief Loads a font face for the given family, size and style, generating glyph atlasses for the given codepoint range.
        /// @param extendMode If true, glyphs are added to an already-loaded font instead of failing with FontAlreadyLoaded.
        /// @return A LoadFontResult describing the outcome.
        static LoadFontResult LoadFont(std::string_view fontFamily, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast, const bool extendMode = false);

        /// @brief Returns all currently loaded fonts.
        /// @return A constant reference to the list of loaded FontInfo entries.
        static const std::list<FontInfo>& GetFonts();

        /// @brief Finds the loaded FontInfo matching the given family, size and style.
        /// @return A pointer to the matching FontInfo, or nullptr if no such font is loaded.
        static const FontInfo* GetFontInfo(std::string_view fontFamily, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);

        /// @brief Sets the font used when no explicit font is specified, loading it if necessary.
        static void SetDefaultFont(std::string_view fontFamily, const f32 size = 16.0f, const PlatformBridge::Fonts::Slant slant = PlatformBridge::Fonts::Slant::Roman, const PlatformBridge::Fonts::Weight weight = PlatformBridge::Fonts::Weight::Normal, const u32 codePointFirst = BasicLatinFirst, const u32 codePointLast = BasicLatinLast);

        /// @brief Converts a floating point font size to the integral key used to index glyph atlasses.
        /// @return The integral font size.
        static u32 FontSizeToIntegral(const f32 size);

        /// @brief Looks up the glyph for the given codepoint within a glyph atlas, among the glyphs made so far.
        /// @return A pointer to the Glyph, or nullptr if the codepoint is not present in the atlas.
        static const Glyph* GetGlyph(const GlyphAtlas& atlas, const u32 codePoint);

        /// @brief Finds the glyph drawing a codepoint in the font at a size. In a block loaded glyph by glyph, a glyph not
        ///        shown before is rendered now, which needs a GL context current; call FlushGlyphUploads once the text is laid out.
        /// @return nullptr when no block of the font covers the code point, or a glyph can't be rendered without a context.
        static const Glyph* FindGlyph(const FontInfo& font, const u32 integralSize, const u32 codePoint);

        /// @brief Lets the other windows' contexts see the glyphs rendered since the last call: textures are shared between
        ///        contexts, but another one sees a change only once it's flushed. Does nothing when nothing was rendered.
        static void FlushGlyphUploads();

        /// @brief Extends an already-loaded fontset by loading glyphs for the given codepoint range from a fallback font family.
        static void ExtendFontset(std::string_view fontFamily, std::string_view fontFamilyExtension, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast);

    private:
        struct AtlasDimension
        {
            u32 _Width { 0 },
                _Height { 0 },
                _NumGlyphs { 0 };
        };

        FontManager() = default;
        static FontManager& getInstance()
        {
            static FontManager instance;
            return instance;
        }

        ~FontManager() 
        {
            if(_ft)
                FT_Done_FreeType(_ft);
        }

        static i32 initFreeTypeLibrary();
        static i32 checkFontIntegrity(std::string_view fontFamily, const u32 integralSize, const u32 fontStyles);
        static bool isFontLoaded(std::string_view fontFamily, const u32 integralSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);
        static PlatformBridge::Fonts::FontProperty findFontProperty(std::string_view fontFamily, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);
        static u32 generateGlyphAtlas(FT_Face face, const u32 codePointFirst, const u32 codePointLast, const u32 integralFontSize, std::vector<Glyph>* glyphs, AtlasDimension* atlasDim);

        /// @brief GL's unpacking state and texture binding, as an upload found them.
        struct UploadState
        {
            i32 _Alignment { 4 },
                _RowLength { 0 },
                _Texture { 0 };
        };

        /// @brief Sets the unpacking for tightly packed single-byte rows, and returns what it was, for endUpload.
        static UploadState beginUpload();
        static void endUpload(const UploadState& state);

        /// @brief The first code point of the Unicode block holding the code point, which keys the font's glyph blocks; 0 when none holds it.
        static u32 blockKeyOf(const u32 codePoint);

        /// @brief Renders one glyph of a block loaded glyph by glyph into its pages, and keeps it.
        static const Glyph* loadGlyph(GlyphBlock& block, const u32 codePoint);

        /// @brief The page a glyph this big goes onto, starting the next row or a new page when the last one has no room.
        /// @return nullptr for a glyph bigger than a page, which then draws nothing.
        static GlyphPage* pageWithRoom(GlyphBlock& block, const u32 width, const u32 height);

        /// @brief A page's side for the face's size: 1024, or more until 16 rows of glyphs fit.
        static u32 pageSideFor(FT_Face face);
        //static void assignGlyphData(const u32 textureID, f32 fontSize, std::vector<Glyph>* glyphs, AtlasDimension* atlasDim, std::pair<std::string, std::string>* font, FT_Face face);

        static inline std::list<FontInfo> _fonts {};
        static inline FT_Library _ft { nullptr };
        static inline std::string _defaultFontFamily;
        static inline FontInfo* _defaultFontInfo { nullptr };
        static inline bool _hasUnflushedUploads { false };
        static constexpr u32
            _individualLoadThreshold { 256 }, // a range with more code than 256 codepoints is loaded glyph by glyph
            _pageSide { 1024 },
            _maxPageSide { 8192 };
    };
}