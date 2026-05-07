#pragma once
#include "IncludeHelper.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>
#include <map>
#include <memory>
#include <list>
#include "UnicodeBlocks.hpp"

namespace RetroFuturaGUI
{
    enum FontStyle : u32
    {
        REGULAR = 1,
        BOLD = 1 << 1,
        ITALIC = 1 << 2
    };

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

    struct GlyphAtlas
    {
        u32 _textureID { 0 };
        u32 _Width { 0 };
        u32 _Height { 0 };
        u32 _FontSize { 0 };
        std::unordered_map<u32, Glyph> _Glyphs; // keyed by codepoint
        FT_CharMap _charMap;
    };

    struct FontInfo
    {
        std::string 
            _name,
            _path;
        u32
            _currentFontStyleDIP { FontStyle::REGULAR },
            _fontStylesAvailable { FontStyle::REGULAR };
        std::unordered_map<u32, GlyphAtlas> _atlasses; // keyed by font size
    };

    class FontManager
    {
    public:
        static i32 Init();
        static i32 LoadFont(std::string_view fontName, const u32 size, const u32 fontStyles, const u32 codePointFirst, const u32 codePointLast);
        static const std::list<FontInfo>& GetFonts() { return _fonts; }
        static std::shared_ptr<FontInfo> GetFontInfo(std::string_view fontName, u32 size);
        static void SetDefaultFont(std::string_view fontName, u32 size = 16, u32 fontStyles = FontStyle::REGULAR, const u32 codePointFirst = BasicLatinFirst, const u32 codePointLast = BasicLatinLast);

    private:
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
        
        static inline std::list<FontInfo> _fonts {};
        static inline FT_Library _ft { nullptr };
        static inline std::string _defaultFontName;
        static inline FontInfo* _defaultFontInfo { nullptr };
    };
}