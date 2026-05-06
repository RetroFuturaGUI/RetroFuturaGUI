#include "FontManager.hpp"
#include "UnicodeBlocks.hpp"
#include <print>
#include <GL/gl.h> // Assuming OpenGL headers are available
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

i32 RetroFuturaGUI::FontManager::Init()
{
    return initFreeTypeLibrary();
}

i32 RetroFuturaGUI::FontManager::LoadFont(std::string_view fontPath, const u32 size, [[maybe_unused]] const u32 fontStyles, const u32 codePointFirst, const u32 codePointLast)
{
    if(_ft == nullptr)
    {
        initFreeTypeLibrary();

        if(_ft == nullptr)
        {
            std::println("ERROR::FREETYPE: FreeType Library not initialized");
            return -1;
        }
    }

    if (fontPath.empty())
    {
        std::println("ERROR::FREETYPE: Failed to load font_name");
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(_ft, fontPath.data(), 0, &face)) 
    {
        std::println("ERROR::FREETYPE: Failed to load font");
        return -1;
    }

    FT_Set_Char_Size(face, 0, size << 6, 96, 96);

    u32 numGlyphs = codePointLast - codePointFirst + 1;
    u32 maxGlyphDimension = (1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(numGlyphs));
    u32 atlasWidth = 1;

    while(atlasWidth < maxGlyphDimension) 
        atlasWidth <<= 1;
        
    u32 atlasHeight = atlasWidth;
    std::vector<u8> pixels(atlasWidth * atlasHeight);
    u32 penX = 0, penY = 0;
    std::vector<Glyph> info(numGlyphs);

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

        info[codePoint - codePointFirst]._CodePoint = codePoint;
        info[codePoint - codePointFirst]._OriginX = penX;
        info[codePoint - codePointFirst]._OriginY = penY;
        info[codePoint - codePointFirst]._EndX = penX + bitmap->width;
        info[codePoint - codePointFirst]._EndY = penY + bitmap->rows;
        info[codePoint - codePointFirst]._BearingX   = face->glyph->bitmap_left;
        info[codePoint - codePointFirst]._BearingY   = face->glyph->bitmap_top;
        info[codePoint - codePointFirst]._Advance = face->glyph->advance.x >> 6;
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

    _fonts.push_back(
        { 
            std::string(fontPath), std::string(fontPath), FontStyle::REGULAR, FontStyle::REGULAR, 
            {
                textureID,
                atlasWidth,
                atlasHeight,
                size,
                {},
                face->charmap
            }
        });

    for(uSize i = 0; i < numGlyphs; ++i)
    {
        Glyph glyph;
        glyph._Size[0] = info[i]._EndX - info[i]._OriginX;
        glyph._Size[1] = info[i]._EndY - info[i]._OriginY;
        glyph._Bearing[0] = info[i]._BearingX;
        glyph._Bearing[1] = info[i]._BearingY;
        glyph._Advance = info[i]._Advance;
        glyph._UV[0] = static_cast<f32>(info[i]._OriginX) / atlasWidth; // minU
        glyph._UV[1] = static_cast<f32>(info[i]._OriginY) / atlasHeight; // minV
        glyph._UV[2] = static_cast<f32>(info[i]._EndX) / atlasWidth; // maxU
        glyph._UV[3] = static_cast<f32>(info[i]._EndY) / atlasHeight; // maxV

        _fonts.back()._atlas._Glyphs.insert({ info[i]._CodePoint, glyph });
    }

    FT_Done_Face(face);
	stbi_write_png("font_output.png", atlasWidth, atlasHeight, 4, imageData.data(), atlasWidth * 4);

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