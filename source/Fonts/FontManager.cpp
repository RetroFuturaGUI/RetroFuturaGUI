#include "FontManager.hpp"
#include <print>
#include <GL/gl.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

i32 RetroFuturaGUI::FontManager::Init()
{
    return initFreeTypeLibrary();
}

bool RetroFuturaGUI::FontManager::isFontLoaded(std::string_view fontFamily, const u32 integralSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    for(const auto& fontInfo : _fonts)
        if(fontInfo._FontProperty._Name == fontFamily 
            && fontInfo._Atlasses.count(integralSize) > 0
            && fontInfo._FontProperty._Slant == slant
            && fontInfo._FontProperty._Weight == weight)
            return true;
        
    return false;
}

PlatformBridge::Fonts::FontProperty RetroFuturaGUI::FontManager::findFontProperty(std::string_view fontFamily, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    auto fonts = PlatformBridge::Fonts::GetFontProperties();

    auto font = std::find_if(fonts.begin(), fonts.end(), [fontFamily, slant, weight](const auto& property)
    {
        return property._Name == fontFamily && property._Slant == slant && property._Weight == weight;
    });

    if(font != fonts.end())
        return *font;

    font = std::find_if(fonts.begin(), fonts.end(), [fontFamily, slant](const auto& property)
    {
        return property._Name == fontFamily && property._Slant == slant;
    });

    if(font != fonts.end())
        return *font;

    font = std::find_if(fonts.begin(), fonts.end(), [fontFamily, weight](const auto& property)
    {
        return property._Name == fontFamily && property._Weight == weight;
    });

    if(font != fonts.end())
        return *font;

    font = std::find_if(fonts.begin(), fonts.end(), [fontFamily](const auto& property)
    {
        return property._Name == fontFamily;
    });

    if(font != fonts.end())
        return *font;

    return fonts.front();
}

u32 RetroFuturaGUI::FontManager::generateGlyphAtlas(FT_Face face, const u32 codePointFirst, const u32 codePointLast, const u32 integralFontSize, std::vector<Glyph>* glyphs, AtlasDimension* atlasDim)
{
    FT_Set_Char_Size(face, 0, integralFontSize << 6, 96, 96);
    std::vector<Glyph>& glyphsRef = *glyphs;
    u32 numGlyphs { codePointLast - codePointFirst + 1 },
        maxGlyphDimension { (u32)(1 + (face->size->metrics.height >> 6)) * (u32)ceilf(sqrtf(static_cast<float>(numGlyphs))) },
        atlasWidth { 1 };
    glyphsRef.resize(numGlyphs);

    while(atlasWidth < maxGlyphDimension) 
        atlasWidth <<= 1;
        
    u32 atlasHeight { atlasWidth };
    std::vector<u8> pixels(atlasWidth * atlasHeight);
    u32 penX { 0 }, 
        penY { 0 };

    for(uSize codePoint = codePointFirst; codePoint <= codePointLast; ++codePoint)
    {
        FT_Load_Char(face, static_cast<FT_ULong>(codePoint), FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
        FT_Bitmap* bitmap = &face->glyph->bitmap;

        if(penX + bitmap->width >= atlasWidth){
            penX = 0;
            penY += ((face->size->metrics.height >> 6) + 1);
        }

        for(uSize row = 0; row < bitmap->rows; ++row)
        {
            for(uSize col = 0; col < bitmap->width; ++col)
            {
                u32 x = penX + static_cast<u32>(col);
                u32 y = penY + static_cast<u32>(row);
                pixels[y * atlasWidth + x] = bitmap->buffer[row * bitmap->pitch + col];
            }
        }

        glyphsRef[codePoint - codePointFirst]._CodePoint = static_cast<u32>(codePoint);
        glyphsRef[codePoint - codePointFirst]._OriginX = penX;
        glyphsRef[codePoint - codePointFirst]._OriginY = penY;
        glyphsRef[codePoint - codePointFirst]._EndX = penX + static_cast<u32>(bitmap->width);
        glyphsRef[codePoint - codePointFirst]._EndY = penY + static_cast<u32>(bitmap->rows);
        glyphsRef[codePoint - codePointFirst]._BearingX   = face->glyph->bitmap_left;
        glyphsRef[codePoint - codePointFirst]._BearingY   = face->glyph->bitmap_top;
        glyphsRef[codePoint - codePointFirst]._Advance = face->glyph->advance.x >> 6;
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

    //stbi_write_png((std::string("font_output_") + std::to_string(codePointFirst) + "-" + std::to_string(codePointLast) + ".png").c_str(), atlasWidth, atlasHeight, 4, imageData.data(), atlasWidth * 4);

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

RetroFuturaGUI::FontManager::LoadFontResult RetroFuturaGUI::FontManager::LoadFont(std::string_view fontFamily, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast, const bool extendMode)
{
    u32 integralFontSize = FontSizeToIntegral(size);

    if(!extendMode)
        if(isFontLoaded(fontFamily, integralFontSize, slant, weight))
            return LoadFontResult::FontAlreadyLoaded;

    if(_ft == nullptr)
    {
        initFreeTypeLibrary();

        if(_ft == nullptr)
        {
            std::println("ERROR::FREETYPE: FreeType Library not initialized");
            return LoadFontResult::FreeTypeError;
        }
    }

    auto fontProperty = findFontProperty(fontFamily, slant, weight);
    
    if (fontProperty._Path.empty())
    {
        std::println("ERROR::FREETYPE: No font path");
        return LoadFontResult::FontNotFound;
    }

    FT_Face face;
    if (FT_New_Face(_ft, fontProperty._Path.data(), 0, &face)) 
    {
        std::println("ERROR::FREETYPE: Failed to load font");
        return LoadFontResult::FreeTypeError;
    }

    std::vector<Glyph> glyphs {};
    AtlasDimension atlasDim {};
    u32 textureID = generateGlyphAtlas(face, codePointFirst, codePointLast, integralFontSize, &glyphs, &atlasDim);

    _fonts.emplace_back();
    _fonts.back()._FontProperty = fontProperty;
    _fonts.back()._Atlasses[integralFontSize]._FontSize = size;
    _fonts.back()._Atlasses[integralFontSize]._GlyphBlocks[codePointFirst] = 
    {
        ._TextureID = textureID,
        ._Width = atlasDim._Width,
        ._Height = atlasDim._Height,
        ._CodePointFirst = codePointFirst,
        ._CodePointLast = codePointLast,
        ._LoadIndividually = false,
        ._Glyphs = {},
        ._CharMap = face->charmap
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
        _fonts.back()._Atlasses[integralFontSize]._GlyphBlocks[codePointFirst]._Glyphs.insert({ glyphs[i]._CodePoint, glyph });
    }

    FT_Done_Face(face);
    return LoadFontResult::Success;
}

i32 RetroFuturaGUI::FontManager::initFreeTypeLibrary()
{
    i32 err = FT_Init_FreeType(&_ft);

    if (err == 0)
        return 0;

    std::println("ERROR::FREETYPE: Could not init FreeType Library");
    return -1;
}

const RetroFuturaGUI::FontInfo* RetroFuturaGUI::FontManager::GetFontInfo(std::string_view fontFamily, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    u32 fontIndex = FontSizeToIntegral(size);

    for (const auto& font : _fonts)
    {
        if (font._FontProperty._Name == fontFamily
            && font._Atlasses.count(fontIndex) > 0
            && font._FontProperty._Slant == slant
            && font._FontProperty._Weight == weight)
        {
            return &font;
        }
    }

    // If not found, load it
    if (LoadFont(fontFamily, size, slant, weight, BasicLatinFirst, BasicLatinLast) == 0)
    {
        for (const auto& font : _fonts)
            if (font._FontProperty._Name == fontFamily && font._Atlasses.count(fontIndex) > 0)
                return &font;
    }

    return nullptr;
}

void RetroFuturaGUI::FontManager::SetDefaultFont(std::string_view fontFamily, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast)
{
    _defaultFontFamily = fontFamily;
    LoadFont(fontFamily, size, slant, weight, codePointFirst, codePointLast);
}

u32 RetroFuturaGUI::FontManager::FontSizeToIntegral(const f32 size)
{
    return size < 1.0f && size > 0.0f ? 1 : static_cast<u32>(size);
}

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::GetGlyph(const GlyphAtlas& atlas, const u32 codePoint)
{
    // Search through all glyph blocks in the atlas for the glyph
    for (const auto& [blockKey, glyphBlock] : atlas._GlyphBlocks)
    {
        auto iter = glyphBlock._Glyphs.find(codePoint);
        if (iter != glyphBlock._Glyphs.end())
            return &iter->second;
    }
    return nullptr;
}

const std::list<RetroFuturaGUI::FontInfo>& RetroFuturaGUI::FontManager::GetFonts()
{
    return _fonts;
}

void RetroFuturaGUI::FontManager::ExtendFontset(std::string_view fontFamily, std::string_view fontFamilyExtension, const f32 size, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast)
{
    u32 integralFontSize = FontSizeToIntegral(size);

    for (auto& font : _fonts)
    {
        if (font._FontProperty._Name == fontFamily
            && font._Atlasses.count(integralFontSize) > 0
            && font._FontProperty._Slant == slant
            && font._FontProperty._Weight == weight)
        {
        
            if(LoadFont(fontFamilyExtension, size, slant, weight, codePointFirst, codePointLast, true) != LoadFontResult::Success)
                return;
            
            font._Atlasses[integralFontSize]._GlyphBlocks[codePointFirst] = _fonts.back()._Atlasses[integralFontSize]._GlyphBlocks[codePointFirst];
            _fonts.pop_back();
            return;
            
        }
    }
}