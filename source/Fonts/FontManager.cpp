#include "FontManager.hpp"
#include <algorithm>
#include <cstddef>
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
        penY { 0 },
        rowHeight { 0 }; // tallest glyph in the row

    for(uSize codePoint = codePointFirst; codePoint <= codePointLast; ++codePoint)
    {
        FT_Load_Char(face, static_cast<FT_ULong>(codePoint), FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
        FT_Bitmap* bitmap = &face->glyph->bitmap;
        const bool fits { bitmap->width < atlasWidth }; //a glyph wider than the atlas keeps its advance but draws nothing

        if(fits && atlasWidth < penX + bitmap->width + 1)
        {
            penX = 0;
            penY += rowHeight + 1;
            rowHeight = 0;
        }

        //The size is estimated from the line height. the atlas grows downward when the rows outrun it
        while(fits && atlasHeight < penY + bitmap->rows)
        {
            atlasHeight *= 2;
            pixels.resize(static_cast<uSize>(atlasWidth) * atlasHeight);
        }

        for(uSize row = 0; fits && row < bitmap->rows; ++row)
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
        glyphsRef[codePoint - codePointFirst]._EndX = penX + (fits ? static_cast<u32>(bitmap->width) : 0);
        glyphsRef[codePoint - codePointFirst]._EndY = penY + (fits ? static_cast<u32>(bitmap->rows) : 0);
        glyphsRef[codePoint - codePointFirst]._BearingX   = face->glyph->bitmap_left;
        glyphsRef[codePoint - codePointFirst]._BearingY   = face->glyph->bitmap_top;
        glyphsRef[codePoint - codePointFirst]._Advance = face->glyph->advance.x >> 6;

        if(!fits)
            continue;

        penX += bitmap->width + 1;
        rowHeight = std::max(rowHeight, static_cast<u32>(bitmap->rows));
    }

    //stbi_write_png((std::string("font_output_") + std::to_string(codePointFirst) + "-" + std::to_string(codePointLast) + ".png").c_str(), atlasWidth, atlasHeight, 1, pixels.data(), atlasWidth);

    //One channel: the font shader reads red only, so RGBA would take four times the memory for the same picture
    u32 textureID { 0 };
    glGenTextures(1, &textureID);
    const UploadState state { beginUpload() };
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    endUpload(state);

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

    if(!_ft)
    {
        initFreeTypeLibrary();

        if(!_ft)
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

    //A big range (e.g. CJK's 20000 ideographs) isn't rendered now.Eeach glyph is rendered when it's first shown
    if(_individualLoadThreshold < codePointLast - codePointFirst + 1)
    {
        FT_Set_Char_Size(face, 0, integralFontSize << 6, 96, 96);
        _fonts.emplace_back();
        _fonts.back()._FontProperty = fontProperty;
        _fonts.back()._Atlasses[integralFontSize]._FontSize = size;
        _fonts.back()._Atlasses[integralFontSize]._GlyphBlocks[codePointFirst] =
        {
            ._TextureID = 0,
            ._Width = 0,
            ._Height = 0,
            ._CodePointFirst = codePointFirst,
            ._CodePointLast = codePointLast,
            ._LoadIndividually = true,
            ._Glyphs = {},
            ._Face = face,
            ._Pages = {},
            ._PageSide = pageSideFor(face)
        };

        return LoadFontResult::Success;
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
        ._Face = nullptr, //closed below: every glyph is made
        ._Pages = {},
        ._PageSide = 0
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
        glyph._TextureID = textureID;
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

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::FindGlyph(const FontInfo& font, const u32 integralSize, const u32 codePoint)
{
    const auto atlas { font._Atlasses.find(integralSize) };

    if(font._Atlasses.end() == atlas)
        return nullptr;

    const auto block { atlas->second._GlyphBlocks.find(blockKeyOf(codePoint)) };

    if(atlas->second._GlyphBlocks.end() == block)
        return nullptr;

    const auto glyph { block->second._Glyphs.find(codePoint) };

    if(block->second._Glyphs.end() != glyph)
        return &glyph->second;

    if(!block->second._LoadIndividually)
        return nullptr;

    return loadGlyph(const_cast<GlyphBlock&>(block->second), codePoint);
}

void RetroFuturaGUI::FontManager::FlushGlyphUploads()
{
    if(!_hasUnflushedUploads)
        return;

    glFlush();
    _hasUnflushedUploads = false;
}

RetroFuturaGUI::FontManager::UploadState RetroFuturaGUI::FontManager::beginUpload()
{
    UploadState state { ._Alignment = 4, ._RowLength = 0, ._Texture = 0 };
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &state._Alignment);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &state._RowLength);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &state._Texture);

    //Rows of single bytes, as long as the glyph is wide; a video upload may have left a longer row length set
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    return state;
}

void RetroFuturaGUI::FontManager::endUpload(const UploadState& state)
{
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(state._Texture));
    glPixelStorei(GL_UNPACK_ALIGNMENT, state._Alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, state._RowLength);
}

u32 RetroFuturaGUI::FontManager::blockKeyOf(const u32 codePoint)
{
    static_assert(std::ranges::is_sorted(UnicodeBlocks), "the blocks are searched by halving, so they have to be in order");

    //The last block starting at or before the code point, if the code point is within it
    const auto after { std::upper_bound(UnicodeBlocks.begin(), UnicodeBlocks.end(), codePoint, [](const u32 point, const std::pair<u32, u32>& block) { return point < block.first; }) };

    if(UnicodeBlocks.begin() == after)
        return 0;

    const std::pair<u32, u32>& block { *(after - 1) };
    return codePoint <= block.second ? block.first : 0;
}

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::loadGlyph(GlyphBlock& block, const u32 codePoint)
{
    if(!block._Face)
        return nullptr;

    //The glyph goes into a texture, so it waits for a context; without one it's asked for again next time
    if(!glfwGetCurrentContext())
        return nullptr;

    if(block._CodePointFirst > codePoint || block._CodePointLast < codePoint)
        return nullptr;

    Glyph glyph;
    glyph._CodePoint = codePoint;

    //A code point the font lacks renders its empty box, as in a block made at once. Kept like any other glyph, a failed one
    //too, so neither is rendered again
    if(FT_Load_Char(block._Face, static_cast<FT_ULong>(codePoint), FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT))
        return &block._Glyphs.emplace(codePoint, glyph).first->second;

    const FT_GlyphSlot slot { block._Face->glyph };
    const FT_Bitmap& bitmap { slot->bitmap };
    glyph._Advance = static_cast<u32>(slot->advance.x >> 6);
    glyph._Bearing[0] = slot->bitmap_left;
    glyph._Bearing[1] = slot->bitmap_top;
    glyph._BearingX = static_cast<u32>(slot->bitmap_left);
    glyph._BearingY = static_cast<u32>(slot->bitmap_top);

    //A space has no picture, and a glyph bigger than a page draws nothing and; both keep their advance
    GlyphPage* page { 0 < bitmap.width && 0 < bitmap.rows ? pageWithRoom(block, bitmap.width, bitmap.rows) : nullptr };

    if(!page)
        return &block._Glyphs.emplace(codePoint, glyph).first->second;

    //FreeType's rows may be padded, so they're copied tight first
    std::vector<u8> pixels(static_cast<uSize>(bitmap.width) * bitmap.rows);

    for(u32 row = 0; row < bitmap.rows; ++row)
        std::copy_n(bitmap.buffer + static_cast<ptrdiff_t>(row) * bitmap.pitch, bitmap.width, pixels.data() + static_cast<uSize>(row) * bitmap.width);

    const UploadState state { beginUpload() };
    glBindTexture(GL_TEXTURE_2D, page->_TextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(page->_PenX), static_cast<GLint>(page->_PenY), static_cast<GLsizei>(bitmap.width), static_cast<GLsizei>(bitmap.rows), GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    endUpload(state);
    _hasUnflushedUploads = true;

    const f32 side { static_cast<f32>(block._PageSide) };
    glyph._Size[0] = static_cast<i32>(bitmap.width);
    glyph._Size[1] = static_cast<i32>(bitmap.rows);
    glyph._OriginX = page->_PenX;
    glyph._OriginY = page->_PenY;
    glyph._EndX = page->_PenX + bitmap.width;
    glyph._EndY = page->_PenY + bitmap.rows;
    glyph._UV[0] = static_cast<f32>(glyph._OriginX) / side;
    glyph._UV[1] = static_cast<f32>(glyph._OriginY) / side;
    glyph._UV[2] = static_cast<f32>(glyph._EndX) / side;
    glyph._UV[3] = static_cast<f32>(glyph._EndY) / side;
    glyph._TextureID = page->_TextureID;

    page->_PenX += bitmap.width + 1; //a pixel between glyphs, so filtering doesn't pick up the neighbor's edge!
    page->_RowHeight = std::max(page->_RowHeight, static_cast<u32>(bitmap.rows));
    return &block._Glyphs.emplace(codePoint, glyph).first->second;
}

RetroFuturaGUI::GlyphPage* RetroFuturaGUI::FontManager::pageWithRoom(GlyphBlock& block, const u32 width, const u32 height)
{
    const u32 side { block._PageSide };

    if(side < width + 1 || side < height + 1)
        return nullptr;

    if(!block._Pages.empty())
    {
        GlyphPage& page { block._Pages.back() };

        //The next row once this one is full, a new page once the rows reach the bottom
        if(side < page._PenX + width + 1)
        {
            page._PenX = 0;
            page._PenY += page._RowHeight + 1;
            page._RowHeight = 0;
        }

        if(side >= page._PenY + height + 1)
            return &page;
    }

    //Cleared, so the pixel left between glyphs stays empty
    const std::vector<u8> cleared(static_cast<uSize>(side) * side);
    u32 textureID { 0 };
    glGenTextures(1, &textureID);
    const UploadState state { beginUpload() };
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, static_cast<GLsizei>(side), static_cast<GLsizei>(side), 0, GL_RED, GL_UNSIGNED_BYTE, cleared.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    endUpload(state);
    block._Pages.push_back(GlyphPage { ._TextureID = textureID, ._PenX = 0, ._PenY = 0, ._RowHeight = 0 });
    return &block._Pages.back();
}

u32 RetroFuturaGUI::FontManager::pageSideFor(FT_Face face)
{
    const u32 rowHeight { static_cast<u32>(face->size->metrics.height >> 6) + 1 };
    u32 side { _pageSide };

    while(16 * rowHeight > side && _maxPageSide > side)
        side <<= 1;

    return side;
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