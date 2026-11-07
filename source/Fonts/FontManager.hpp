#pragma once
#include "IncludeHelper.hpp"
#include "IGlyphSource.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>
#include <map>
#include <memory>
#include <list>
#include <string>
#include <vector>
#include "UnicodeBlocks.hpp"
#include "Fonts.hpp"

namespace RetroFuturaGUI
{
    /// @brief The unit a font size is given in
    enum class FontSizeUnit : u32
    {
        Pixels,
        Points
    };

    struct Glyph
    {
        f32 _UV[4] { 0.0f, 0.0f, 0.0f, 0.0f }; // minU, minV, maxU, maxV
        i32
            _Size[2] { 0, 0 },
            _Bearing[2] { 0, 0 };
        u32 
            _Advance { 0 },
            _CodePoint { 0 },
            _OriginX { 0 },
            _OriginY { 0 },
            _EndX { 0 },
            _EndY { 0 },
            _BearingX { 0 },
            _BearingY { 0 },
            _TextureID { 0 }; // the atlas or page holding the glyph; a block loaded glyph by glyph spreads over several
        bool _IsColor { false }; // on a color page, drawn in its own colors instead of the fill
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
        std::unordered_map<u64, Glyph> _OutlineGlyphs; // glyphs widened for text outlines, keyed by the width (FontSizeToKey) above the codepoint
        std::unordered_map<u32, Glyph> _IndexGlyphs; // glyphs found by shaping, keyed by their index in the source
        std::unordered_map<u64, Glyph> _IndexOutlineGlyphs; // those widened for text outlines, keyed by the width above the index.
        std::string _FontFile {}; // the font file and face the source draws from: neighboring blocks of one file are shaped together
        std::unique_ptr<IGlyphSource> _Source { nullptr }; // kept until the program ends, for glyphs rendered on first use and for outlines
        std::vector<GlyphPage>
            _Pages {}, // one byte per pixel
            _ColorPages {}; // four bytes per pixel, for color glyphs
        u32 _PageSide { 0 }; // width and height
        f32 _Scale { 1.0f }; // the font's size over the source's, for a source extending a font from a fixed size it offers
    };

    struct GlyphAtlas
    {
        f32 _FontSize { 16.0f }; // the size the glyphs were rendered at, in pixels
        FontMetrics _Metrics {}; // of the font itself, not of the fonts extending it
        std::unordered_map<u32, GlyphBlock> _GlyphBlocks; // keyed by first codepoint
        std::list<GlyphBlock> _FallbackBlocks {}; // fonts outside the fontset drawing what it lacks: the default font, opened when first needed
        std::unordered_map<u32, GlyphBlock*> _FallbackFor {}; // the block drawing a character its own block's font lacks, nullptr for none; found once
        bool _IsDefaultFontLookedFor { false };
    };

    struct FontInfo
    {
        std::string _Family {}; // the family it was asked for and is found by; a family that isn't installed shows another font under this name
        PlatformBridge::Fonts::Slant _Slant { PlatformBridge::Fonts::Slant::Roman }; // the style asked for, found by like the family
        PlatformBridge::Fonts::Weight _Weight { PlatformBridge::Fonts::Weight::Normal };
        PlatformBridge::Fonts::FontProperty _FontProperty {}; // the font file shown
        std::unordered_map<u32, GlyphAtlas> _Atlasses; // keyed by FontSizeToKey
    };

    /// @brief One glyph of a shaped text: the block drawing it, which glyph, the character it starts at and how it moves the pen.
    struct PlacedGlyph
    {
        GlyphBlock* _Block { nullptr }; // nullptr when no block of the font covers the character: nothing is drawn
        u32 _GlyphIndex { 0 }, // the glyph in the block's source, when shaped
            _CodePoint { 0 }, // the character, when placed by itself because its source can't shape
            _Cluster { 0 }; // the first character it draws, counted in the text shaped
        glm::vec2
            _Advance { 0.0f }, // pixels at the font's size, y up; when shaped
            _Offset { 0.0f }; // from the pen to the glyph's origin
        bool _IsShaped { false },
            _IsRightToLeft { false }; // shaped right to left: the character's start is the glyph's right edge
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
        ///        A family that isn't installed shows the default font instead, or else the first font loaded, kept under the
        ///        name asked for. A style the family lacks shows its closest one.
        /// @param extendMode If true, glyphs are added to an already-loaded font instead of failing with FontAlreadyLoaded.
        /// @return A LoadFontResult describing the outcome.
        static LoadFontResult LoadFont(std::string_view fontFamily, const f32 size, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast, const bool extendMode = false);

        /// @brief Returns all currently loaded fonts.
        /// @return A constant reference to the list of loaded FontInfo entries.
        static const std::list<FontInfo>& GetFonts();

        /// @brief Finds the loaded FontInfo matching the given family, size and style, loading its Basic Latin glyphs when it isn't loaded yet.
        /// @return A pointer to the matching FontInfo, or nullptr if it couldn't be loaded.
        static const FontInfo* GetFontInfo(std::string_view fontFamily, const f32 size, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);

        /// @brief Sets the font shown in place of a family that isn't installed, loading it.
        static void SetDefaultFont(std::string_view fontFamily, const f32 size, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant = PlatformBridge::Fonts::Slant::Roman, const PlatformBridge::Fonts::Weight weight = PlatformBridge::Fonts::Weight::Normal, const u32 codePointFirst = BasicLatinFirst, const u32 codePointLast = BasicLatinLast);

        /// @brief Returns the sizes a font offers in the given unit, smallest first: a bitmap font's fixed sizes. Empty for a
        ///        font shown at any size, and for one that can't be opened.
        static std::vector<f32> GetOfferedSizes(std::string_view fontFamily, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const FontSizeUnit sizeUnit);

        /// @brief Converts a font size to pixels.
        static constexpr f32 ToPixels(const f32 size, const FontSizeUnit sizeUnit)
        {
            return FontSizeUnit::Points == sizeUnit ? size * _pixelsPerInch / _pointsPerInch : size;
        }

        /// @brief Converts a size in pixels to the given unit.
        static constexpr f32 FromPixels(const f32 pixels, const FontSizeUnit sizeUnit)
        {
            return FontSizeUnit::Points == sizeUnit ? pixels * _pointsPerInch / _pixelsPerInch : pixels;
        }

        /// @brief Converts a size in pixels to the key a font's atlasses are found by: 1/64 pixels, so fractional sizes stay apart.
        static u32 FontSizeToKey(const f32 pixels);

        /// @brief Looks up the glyph for the given codepoint within a glyph atlas, among the glyphs made so far.
        /// @return A pointer to the Glyph, or nullptr if the codepoint is not present in the atlas.
        static const Glyph* GetGlyph(const GlyphAtlas& atlas, const u32 codePoint);

        /// @brief Finds the glyph drawing a codepoint in the font at a size (FontSizeToKey). In a block loaded glyph by glyph, a
        ///        glyph not shown before is rendered now, through the upload context when no GL context is current; call
        ///        FlushGlyphUploads once the text is laid out.
        /// @return nullptr when no block of the font covers the code point, or no context is there to render it with.
        static const Glyph* FindGlyph(const FontInfo& font, const u32 sizeKey, const u32 codePoint);

        /// @brief Lets the other windows' contexts see the glyphs rendered since the last call: textures are shared between
        ///        contexts, but another one sees a change only once it's flushed. Does nothing when nothing was rendered.
        static void FlushGlyphUploads();

        /// @brief Finds a glyph widened by width pixels for a text outline, rendering it into the block's pages the first time,
        ///        through the upload context when no GL context is current.
        /// @return nullptr when no block of the font covers the code point, or no context is there to render it with. A glyph
        ///        without an outline to widen (a bitmap font's) gives an empty glyph.
        static const Glyph* FindGlyphOutline(const FontInfo& font, const u32 sizeKey, const u32 codePoint, const f32 width);

        /// @brief Shapes a run of text of one direction with the font at a size (FontSizeToKey), one run per glyph source: the
        ///        characters each block covers go to its font together, and joiners, variation selectors, skin tones and
        ///        combining marks stay with the run before them, so an emoji sequence reaches the emoji font whole. A character
        ///        its block's font lacks is drawn by another font of the fontset that has it, else by the default font. A
        ///        source that can't shape places its characters one by one, unshaped. Right to left, the glyphs come out in the
        ///        order they're drawn, from the left.
        static void ShapeText(const FontInfo& font, const u32 sizeKey, std::u32string_view text, const ShapeDirection direction, std::vector<PlacedGlyph>* glyphs);

        /// @brief Finds the glyph a placed glyph draws, rendering it the first time as FindGlyph does.
        /// @return nullptr when nothing covers it, or no context is there to render it with.
        static const Glyph* FindPlacedGlyph(const FontInfo& font, const u32 sizeKey, const PlacedGlyph& placed);

        /// @brief Finds the glyph a placed glyph draws widened for a text outline, as FindGlyphOutline does.
        static const Glyph* FindPlacedGlyphOutline(const FontInfo& font, const u32 sizeKey, const PlacedGlyph& placed, const f32 width);

        /// @brief Extends an already-loaded fontset by loading glyphs for the given codepoint range from a fallback font family.
        static void ExtendFontset(std::string_view fontFamily, std::string_view fontFamilyExtension, const f32 size, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast);

        /// @brief Sets the context glyphs are uploaded with when none is current, which shares its textures with every window.
        ///        Window hands over the first window's context and passes it on when that window goes.
        static void SetUploadContext(GLFWwindow* context);

    private:
        struct AtlasDimension
        {
            u32 _Width { 0 },
                _Height { 0 };
            uSize _GlyphCount { 0 };
        };

        /// @brief Makes the upload context current for its lifetime when no context is, so glyphs can be uploaded at any time.
        struct UploadContextScope
        {
            UploadContextScope();
            ~UploadContextScope();
            UploadContextScope(const UploadContextScope&) = delete;
            UploadContextScope& operator=(const UploadContextScope&) = delete;

            /// @brief Returns whether a context is current, so GL can be called.
            bool HasContext() const;

            bool _isMadeCurrent { false };
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
        static bool isFontLoaded(std::string_view fontFamily, const u32 sizeKey, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);

        /// @brief The installed font file of a family closest to the style: the same slant first, then the closest weight.
        /// @return false when the family isn't installed.
        static bool findFontProperty(std::string_view fontFamily, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, PlatformBridge::Fonts::FontProperty* property);

        /// @brief The font file shown for a family: its own, or for a family that isn't installed the default font's, else the
        ///        first loaded font's, else a font every system of the platform has, else the first one installed.
        static PlatformBridge::Fonts::FontProperty resolveFontProperty(std::string_view fontFamily, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);

        static u32 generateGlyphAtlas(IGlyphSource& source, const u32 codePointFirst, const u32 codePointLast, std::vector<Glyph>* glyphs, AtlasDimension* atlasDim);

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

        /// @brief Puts a rendered glyph onto a page of the block and fills in where it is and how big, scaled to the font's size.
        static void storeGlyph(GlyphBlock& block, const GlyphBitmap& bitmap, Glyph* glyph);

        /// @brief The block of the font covering a character, nullptr when none does.
        static GlyphBlock* findBlock(const GlyphAtlas& atlas, const u32 codePoint);

        /// @brief The block whose font draws a character: its own block's, if that font has it, else the first other font of
        ///        the fontset that has it (in the order of their blocks), else the default font's. When none has it, its own
        ///        block, which draws the font's "missing" glyph, or nullptr when no block covers it.
        static GlyphBlock* findDrawingBlock(const FontInfo& font, GlyphAtlas& atlas, const u32 codePoint);

        /// @brief The default font opened at the atlas's size, kept among its fallback blocks; nullptr when it's one of the
        ///        fontset's own fonts (asked already) or can't be opened.
        static GlyphBlock* getDefaultFontBlock(const FontInfo& font, GlyphAtlas& atlas);

        /// @brief Whether a character belongs to the character before it rather than starting a run of its own: joiners,
        ///        variation selectors, combining marks, skin tones and the tags of subdivision flags.
        static bool isAttaching(const u32 codePoint);

        /// @brief The page a glyph this big goes onto, starting the next row or a new page when the last one has no room.
        /// @return nullptr for a glyph bigger than a page, which then draws nothing.
        static GlyphPage* pageWithRoom(GlyphBlock& block, const u32 width, const u32 height, const GlyphPixelFormat format);

        /// @brief A page's side for the font's line height: 1024, or more until 16 rows of glyphs fit.
        static u32 pageSideFor(const FontMetrics& metrics);

        static inline std::list<FontInfo> _fonts {};
        static inline FT_Library _ft { nullptr };
        static inline std::string _defaultFontFamily;
        static inline GLFWwindow* _uploadContext { nullptr };
        static inline bool _hasUnflushedUploads { false };
        static constexpr u32
            _individualLoadThreshold { 256 }, // a range with more code than 256 codepoints is loaded glyph by glyph
            _pageSide { 1024 },
            _maxPageSide { 8192 };
        static constexpr f32
            _pixelsPerInch { 96.0f },
            _pointsPerInch { 72.0f };
#ifdef _WIN32
        static constexpr std::string_view _lastResortFamily { "Arial" };
#else
        static constexpr std::string_view _lastResortFamily { "DejaVu Sans" };
#endif
    };
}
