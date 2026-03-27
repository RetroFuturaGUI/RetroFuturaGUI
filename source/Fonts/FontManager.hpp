#pragma once
#include "IncludeHelper.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>
#include <map>
#include <memory>
#include <list>

namespace RetroFuturaGUI
{
    enum FontStyle : u32
    {
        REGULAR = 1,
        BOLD = 1 << 1,
        ITALIC = 1 << 2
    };

    struct Character
    {
        i32 _TextureID,
            _Width { 0 },
            _Height { 0 },
            _BearingX { 0 },
            _BearingY { 0 };
        u32 _Advance;
    };

    struct Glyph
    {
        f32 _UV[4] { 0.0f, 0.0f, 0.0f, 0.0f };
        i32 _Size[2] { 0, 0} ;
        i32 _Bearing[2] { 0, 0 };
        u32 _Advance { 0 },
            _AtlasIndex { 0 };
    };

    struct GlyphMeta
    {
        u32 _CodePoint { 0 },
            _OriginX { 0 },
            _OriginY { 0 },
            _EndX { 0 },
            _EndY { 0 },
            _BearingX { 0 },
            _BearingY { 0 },
            _Advance { 0 };
    };

    struct GlyphAtlas
    {
        i32 _textureID,
            _Width { 0 },
            _Height { 0 };

        std::unordered_map<u8, Glyph> _Glyphs;
    };

    struct FontInfo
    {
        std::string 
            _name,
            _path;
        u32
            _currentFontStyleDIP { FontStyle::REGULAR },
            _fontStylesAvailable { FontStyle::REGULAR };
        FT_CharMap _charMap;
        std::map<f32, std::shared_ptr<std::vector<Character>>> _glyphsBySize;
    };

    class FontManager
    {
    public:
        static i32 Init();
        static i32 LoadFont(std::string_view fontPath, const u32 size, const u32 fontStyles, const u32 codePointFirst, const u32 codePointLast);
        static const std::list<FontInfo>& GetFonts() { return _fonts; }

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
        
        static inline std::list<FontInfo> _fonts;
        static inline FT_Library _ft;

    };
}