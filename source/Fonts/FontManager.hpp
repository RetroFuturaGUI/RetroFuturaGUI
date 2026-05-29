#pragma once
#include "IncludeHelper.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>
#include <map>
#include <memory>
#include <list>
#include "UnicodeBlocks.hpp"
#include "PlatformBridge.hpp"

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
            _BearingY { 0 };
    };

    struct GlyphBlock
    {
        u32 _TextureID { 0 },
            _Width { 0 },
            _Height { 0 },
            _CodePointFirst { 0 },
            _CodePointLast { 0 };
        bool _LoadIndividually { false };
        std::unordered_map<u32, Glyph> _Glyphs; // keyed by codepoint
        FT_CharMap _CharMap;
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
    public:
        static i32 Init();
        static i32 LoadFont(std::string_view fontName, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast);
        static const std::list<FontInfo>& GetFonts();
        static std::shared_ptr<FontInfo> GetFontInfo(std::string_view fontName, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);
        static void SetDefaultFont(std::string_view fontName, const f32 size = 16.0f, const PlatformBridge::Fonts::Slant slant = PlatformBridge::Fonts::Slant::Roman, const PlatformBridge::Fonts::Weight weight = PlatformBridge::Fonts::Weight::Normal, const u32 codePointFirst = BasicLatinFirst, const u32 codePointLast = BasicLatinLast);
        static u32 FontSizeToIntegral(const f32 size);
        static const Glyph* GetGlyph(const GlyphAtlas& atlas, const u32 codePoint);

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
        static i32 checkFontIntegrity(std::string_view fontName, const u32 integralSize, const u32 fontStyles);
        static bool isFontLoaded(std::string_view fontName, const u32 integralSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);
        static PlatformBridge::Fonts::FontProperty findFontProperty(std::string_view fontName, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);
        static u32 generateGlyphAtlas(FT_Face face, const u32 codePointFirst, const u32 codePointLast, const u32 integralFontSize, std::vector<Glyph>* glyphs, AtlasDimension* atlasDim);
        //static void assignGlyphData(const u32 textureID, f32 fontSize, std::vector<Glyph>* glyphs, AtlasDimension* atlasDim, std::pair<std::string, std::string>* font, FT_Face face);

        static inline std::list<FontInfo> _fonts {};
        static inline FT_Library _ft { nullptr };
        static inline std::string _defaultFontName;
        static inline FontInfo* _defaultFontInfo { nullptr };
    };
}