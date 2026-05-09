#include "FontManager.hpp"
#include "PlatformBridge.hpp"
#include <print>
#include <GL/gl.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

i32 RetroFuturaGUI::FontManager::Init()
{
    return initFreeTypeLibrary();
}

bool RetroFuturaGUI::FontManager::isFontLoaded(std::string_view fontName, const u32 integralSize, [[maybe_unused]] const u32 fontStyles)
{
    for(const auto& fontInfo : _fonts)
        if(fontInfo._name == fontName && fontInfo._atlasses.count(integralSize) > 0)
            return true;
        
    return false;
}

std::pair<std::string, std::string> RetroFuturaGUI::FontManager::findFontPath(std::string_view fontName, const u32 fontStyles)
{
    auto fonts = PlatformBridge::Fonts::GetFontsInformation();

    auto font = std::find_if(fonts.begin(), fonts.end(), [fontName, fontStyles](const auto& pair)
    {
        std::string fontStyle;

        switch(fontStyles)
        {
            case FontStyle::BOLD: 
                fontStyle = "Bold"; 
            break;
            case FontStyle::ITALIC:
                fontStyle = "Italic";
                break;
            default:
                fontStyle = "Regular";
            break;
        }

        std::string fontNameExt = std::string(fontName) + ":style=" + fontStyle;
        return pair.first.find(fontNameExt) != std::string::npos;
    });

    return { font->first, font->second };
}

u32 RetroFuturaGUI::FontManager::generateGlyphAtlas(FT_Face face, const u32 codePointFirst, const u32 codePointLast, const u32 integralFontSize, std::vector<Glyph>* glyphs, AtlasDimension* atlasDim)
{
    FT_Set_Char_Size(face, 0, integralFontSize << 6, 96, 96);

    u32 numGlyphs { codePointLast - codePointFirst + 1 },
        maxGlyphDimension { (u32)(1 + (face->size->metrics.height >> 6)) * (u32)ceilf(sqrtf(numGlyphs)) },
        atlasWidth { 1 };
    glyphs->resize(numGlyphs);

    while(atlasWidth < maxGlyphDimension) 
        atlasWidth <<= 1;
        
    u32 atlasHeight { atlasWidth };
    std::vector<u8> pixels(atlasWidth * atlasHeight);
    u32 penX { 0 }, 
        penY { 0 };

    for(uSize codePoint = codePointFirst; codePoint <= codePointLast; ++codePoint)
    {
        FT_Load_Char(face, codePoint, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
        FT_Bitmap* bitmap = &face->glyph->bitmap;

        if(penX + bitmap->width >= atlasWidth){
            penX = 0;
            penY += ((face->size->metrics.height >> 6) + 1);
        }

        for(uSize row = 0; row < bitmap->rows; ++row)
        {
            for(uSize col = 0; col < bitmap->width; ++col)
            {
                u32 x = penX + col;
                u32 y = penY + row;
                pixels[y * atlasWidth + x] = bitmap->buffer[row * bitmap->pitch + col];
            }
        }

        (*glyphs)[codePoint - codePointFirst]._CodePoint = codePoint;
        (*glyphs)[codePoint - codePointFirst]._OriginX = penX;
        (*glyphs)[codePoint - codePointFirst]._OriginY = penY;
        (*glyphs)[codePoint - codePointFirst]._EndX = penX + bitmap->width;
        (*glyphs)[codePoint - codePointFirst]._EndY = penY + bitmap->rows;
        (*glyphs)[codePoint - codePointFirst]._BearingX   = face->glyph->bitmap_left;
        (*glyphs)[codePoint - codePointFirst]._BearingY   = face->glyph->bitmap_top;
        (*glyphs)[codePoint - codePointFirst]._Advance = face->glyph->advance.x >> 6;
        penX += bitmap->width + 1;
    }

    // Create RGBA image data
    std::vector<u8> imageData(atlasWidth * atlasHeight * 4);
    
    for(uSize i = 0; i < (atlasWidth * atlasHeight); ++i)
    {
        imageData[i * 4 + 0] = pixels[i];
        imageData[i * 4 + 1] = pixels[i];
        imageData[i * 4 + 2] = pixels[i];
        imageData[i * 4 + 3] = 0xFF;
    }

    u32 textureID { 0 };
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(atlasDim)
    {
        atlasDim->_Width = atlasWidth;
        atlasDim->_Height = atlasHeight;
        atlasDim->_NumGlyphs = numGlyphs;
    }

    return textureID;
}

i32 RetroFuturaGUI::FontManager::LoadFont(std::string_view fontName, const f32 size, [[maybe_unused]] const u32 fontStyles, const u32 codePointFirst, const u32 codePointLast)
{
    u32 integralFontSize = FontSizeToIntegral(size);

    if(isFontLoaded(fontName, integralFontSize, fontStyles))
        return 0;

    if(_ft == nullptr)
    {
        initFreeTypeLibrary();

        if(_ft == nullptr)
        {
            std::println("ERROR::FREETYPE: FreeType Library not initialized");
            return -1;
        }
    }

    auto font = findFontPath(fontName, fontStyles);
    
    if (font.second.empty())
    {
        std::println("ERROR::FREETYPE: No font path");
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(_ft, font.second.data(), 0, &face)) 
    {
        std::println("ERROR::FREETYPE: Failed to load font");
        return -1;
    }

    std::vector<Glyph> glyphs {};
    AtlasDimension atlasDim {};
    u32 textureID = generateGlyphAtlas(face, codePointFirst, codePointLast, integralFontSize, &glyphs, &atlasDim);

    bool fontNameExists = false;
    for(const auto& fontInfo : _fonts)
    {
        if(fontInfo._name == fontName)
        {
            fontNameExists = true;
            break;
        }
    }

    if(!fontNameExists)
    { 
        _fonts.emplace_back();
        _fonts.back()._name = fontName;
        _fonts.back()._path = font.second;
        _fonts.back()._currentFontStyleDIP = FontStyle::REGULAR;
        _fonts.back()._fontStylesAvailable = FontStyle::REGULAR;
    }

    _fonts.back()._atlasses[integralFontSize] = 
    {
        ._TextureID = textureID,
        ._Width = atlasDim._Width,
        ._Height = atlasDim._Height,
        ._FontSize = size,
        ._Glyphs = {},
        ._charMap = face->charmap
    };

    for(uSize i = 0; i < atlasDim._NumGlyphs; ++i)
    {
        Glyph glyph;
        glyph._Size[0] = glyphs[i]._EndX - glyphs[i]._OriginX;
        glyph._Size[1] = glyphs[i]._EndY - glyphs[i]._OriginY;
        glyph._Bearing[0] = glyphs[i]._BearingX;
        glyph._Bearing[1] = glyphs[i]._BearingY;
        glyph._Advance = glyphs[i]._Advance;
        glyph._UV[0] = static_cast<f32>(glyphs[i]._OriginX) / atlasDim._Width; // minU
        glyph._UV[1] = static_cast<f32>(glyphs[i]._OriginY) / atlasDim._Height; // minV
        glyph._UV[2] = static_cast<f32>(glyphs[i]._EndX) / atlasDim._Width; // maxU
        glyph._UV[3] = static_cast<f32>(glyphs[i]._EndY) / atlasDim._Height; // maxV 
        _fonts.back()._atlasses[integralFontSize]._Glyphs.insert({ glyphs[i]._CodePoint, glyph });
    }

    FT_Done_Face(face);
	//stbi_write_png("font_output.png", atlasWidth, atlasHeight, 4, imageData.data(), atlasWidth * 4);
    return 0;
}

i32 RetroFuturaGUI::FontManager::initFreeTypeLibrary()
{
    i32 err = FT_Init_FreeType(&_ft);

    if (err == 0)
        return 0;

    std::println("ERROR::FREETYPE: Could not init FreeType Library");
    return -1;
}

std::shared_ptr<RetroFuturaGUI::FontInfo> RetroFuturaGUI::FontManager::GetFontInfo(std::string_view fontName, f32 size)
{
    u32 fontIndex = FontSizeToIntegral(size);

    for (const auto& font : _fonts)
    {
        if (font._name == fontName && font._atlasses.count(fontIndex) > 0)
        {
            return std::make_shared<FontInfo>(font);
        }
    }

    // If not found, load it
    if (LoadFont(fontName, size, FontStyle::REGULAR, BasicLatinFirst, BasicLatinLast) == 0)
    {
        for (const auto& font : _fonts)
            if (font._name == fontName && font._atlasses.count(fontIndex) > 0)
                return std::make_shared<FontInfo>(font);
    }

    return nullptr;
}

void RetroFuturaGUI::FontManager::SetDefaultFont(std::string_view fontName, f32 size, u32 fontStyles, const u32 codePointFirst, const u32 codePointLast)
{
    _defaultFontName = fontName;
    LoadFont(fontName, size, fontStyles, codePointFirst, codePointLast);
}

u32 RetroFuturaGUI::FontManager::FontSizeToIntegral(const f32 size)
{
    return size < 1.0f && size > 0.0f ? 1 : static_cast<u32>(size);
}

const std::list<RetroFuturaGUI::FontInfo>& RetroFuturaGUI::FontManager::GetFonts()
{
    return _fonts;
}