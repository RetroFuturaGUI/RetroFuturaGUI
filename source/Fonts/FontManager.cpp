#include "FontManager.hpp"
#include "FreeTypeGlyphSource.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <print>
#include <GL/gl.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

i32 RetroFuturaGUI::FontManager::Init()
{
    return initFreeTypeLibrary();
}

bool RetroFuturaGUI::FontManager::isFontLoaded(std::string_view fontFamily, const u32 sizeKey, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    for(const auto& fontInfo : _fonts)
        if(fontInfo._Family == fontFamily
            && 0 < fontInfo._Atlasses.count(sizeKey)
            && fontInfo._Slant == slant
            && fontInfo._Weight == weight)
            return true;

    return false;
}

bool RetroFuturaGUI::FontManager::findFontProperty(std::string_view fontFamily, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, PlatformBridge::Fonts::FontProperty* property)
{
    const std::vector<PlatformBridge::Fonts::FontProperty>& fonts { PlatformBridge::Fonts::GetFontProperties() };
    const PlatformBridge::Fonts::FontProperty* closest { nullptr };
    i32 closestDistance { std::numeric_limits<i32>::max() };

    //Upright against slanted counts most, italic against oblique less, then each step of weight
    for(const PlatformBridge::Fonts::FontProperty& font : fonts)
    {
        if(font._Name != fontFamily)
            continue;

        const bool
            isSlanted { PlatformBridge::Fonts::Slant::Roman != font._Slant },
            isSlantWanted { PlatformBridge::Fonts::Slant::Roman != slant };
        const i32 distance { (isSlanted != isSlantWanted ? 100000 : font._Slant != slant ? 10000 : 0)
            + std::abs(static_cast<i32>(font._Weight) - static_cast<i32>(weight)) };

        if(distance >= closestDistance)
            continue;

        closest = &font;
        closestDistance = distance;
    }

    if(!closest)
        return false;

    if(property)
        *property = *closest;

    return true;
}

PlatformBridge::Fonts::FontProperty RetroFuturaGUI::FontManager::resolveFontProperty(std::string_view fontFamily, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    PlatformBridge::Fonts::FontProperty property {};

    if(findFontProperty(fontFamily, slant, weight, &property))
        return property;

    //The first loaded font's own file, not the name it was asked for, which may itself have stood in for another
    const std::string firstLoadedFamily { _fonts.empty() ? std::string {} : _fonts.front()._FontProperty._Name };

    for(const std::string_view standIn : { std::string_view(_defaultFontFamily), std::string_view(firstLoadedFamily), _lastResortFamily })
    {
        if(standIn.empty() || standIn == fontFamily)
            continue;

        if(!findFontProperty(standIn, slant, weight, &property))
            continue;

#ifndef NDEBUG
        std::println("Font family \"{0}\" isn't installed; \"{1}\" is shown in its place", fontFamily, standIn);
#endif
        return property;
    }

    const std::vector<PlatformBridge::Fonts::FontProperty>& fonts { PlatformBridge::Fonts::GetFontProperties() };
    return fonts.empty() ? PlatformBridge::Fonts::FontProperty {} : fonts.front();
}

RetroFuturaGUI::FontManager::LoadFontResult RetroFuturaGUI::FontManager::LoadFont(std::string_view fontFamily, const f32 size, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast, const bool extendMode)
{
    const f32 pixels { ToPixels(size, sizeUnit) };
    const u32 sizeKey { FontSizeToKey(pixels) };

    if(!extendMode)
        if(isFontLoaded(fontFamily, sizeKey, slant, weight))
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

    const PlatformBridge::Fonts::FontProperty fontProperty { resolveFontProperty(fontFamily, slant, weight) };

    if (fontProperty._Path.empty())
    {
        std::println("ERROR::FREETYPE: No font path");
        return LoadFontResult::FontNotFound;
    }

    std::unique_ptr<IGlyphSource> source { FreeTypeGlyphSource::Open(_ft, fontProperty._Path, fontProperty._FaceIndex, weight) };

    if (!source || !source->SetPixelSize(pixels))
    {
        std::println("ERROR::FREETYPE: Failed to load font");
        return LoadFontResult::FreeTypeError;
    }

    FontInfo& font { _fonts.emplace_back() };
    font._Family = fontFamily;
    font._Slant = slant;
    font._Weight = weight;
    font._FontProperty = fontProperty;
    GlyphAtlas& atlas { font._Atlasses[sizeKey] };
    atlas._FontSize = source->GetPixelSize(); //a font with fixed sizes renders its closest one, and Text scales it
    atlas._Metrics = source->GetMetrics();
    const UploadContextScope context {};

    //A big range (e.g. CJK's 20000 ideographs) isn't rendered now. Each glyph is rendered when it's first shown; without a
    //context to make the atlas with, or with color glyphs, which the one-channel atlas can't hold, a small range is too
    if(_individualLoadThreshold < codePointLast - codePointFirst + 1 || !context.HasContext() || source->HasColorGlyphs())
    {
        atlas._GlyphBlocks[codePointFirst] =
        {
            ._TextureID = 0,
            ._Width = 0,
            ._Height = 0,
            ._CodePointFirst = codePointFirst,
            ._CodePointLast = codePointLast,
            ._LoadIndividually = true,
            ._Glyphs = {},
            ._OutlineGlyphs = {},
            ._IndexGlyphs = {},
            ._IndexOutlineGlyphs = {},
            ._ShadowGlyphs = {},
            ._InnerGlowGlyphs = {},
            ._FontFile = fontProperty._Path + "#" + std::to_string(fontProperty._FaceIndex),
            ._Source = std::move(source),
            ._Pages = {},
            ._ColorPages = {},
            ._PageSide = pageSideFor(atlas._Metrics),
            ._Scale = 1.0f
        };

        return LoadFontResult::Success;
    }

    std::vector<Glyph> glyphs {};
    AtlasDimension atlasDim {};
    const u32 textureID { generateGlyphAtlas(*source, codePointFirst, codePointLast, &glyphs, &atlasDim) };
    GlyphBlock& block { atlas._GlyphBlocks[codePointFirst] };
    block =
    {
        ._TextureID = textureID,
        ._Width = atlasDim._Width,
        ._Height = atlasDim._Height,
        ._CodePointFirst = codePointFirst,
        ._CodePointLast = codePointLast,
        ._LoadIndividually = false,
        ._Glyphs = {},
        ._OutlineGlyphs = {},
        ._IndexGlyphs = {},
        ._IndexOutlineGlyphs = {},
        ._ShadowGlyphs = {},
        ._InnerGlowGlyphs = {},
        ._FontFile = fontProperty._Path + "#" + std::to_string(fontProperty._FaceIndex),
        ._Source = std::move(source), //every glyph is made, but outlines are rendered from it later.
        ._Pages = {},
        ._ColorPages = {},
        ._PageSide = pageSideFor(atlas._Metrics), //for the outlines.
        ._Scale = 1.0f
    };

    for(uSize i = 0; i < atlasDim._GlyphCount; ++i)
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
        block._Glyphs.insert({ glyphs[i]._CodePoint, glyph });
    }

    return LoadFontResult::Success;
}

u32 RetroFuturaGUI::FontManager::generateGlyphAtlas(IGlyphSource& source, const u32 codePointFirst, const u32 codePointLast, std::vector<Glyph>* glyphs, AtlasDimension* atlasDim)
{
    std::vector<Glyph>& glyphsRef = *glyphs;
    const uSize glyphCount { codePointLast - codePointFirst + 1 };
    u32 maxGlyphDimension { (1 + static_cast<u32>(source.GetMetrics()._LineHeight)) * (u32)ceilf(sqrtf(static_cast<f32>(glyphCount))) },
        atlasWidth { 1 };
    glyphsRef.resize(glyphCount);

    while(atlasWidth < maxGlyphDimension)
        atlasWidth <<= 1;

    u32 atlasHeight { atlasWidth };
    std::vector<u8> pixels(atlasWidth * atlasHeight);
    u32 penX { 0 },
        penY { 0 },
        rowHeight { 0 }; // tallest glyph in the row
    GlyphBitmap bitmap {};

    for(uSize codePoint = codePointFirst; codePoint <= codePointLast; ++codePoint)
    {
        source.RenderGlyph(static_cast<u32>(codePoint), &bitmap); //a failed glyph is empty, and keeps its slot
        const bool fits { bitmap._Width < atlasWidth }; //a glyph wider than the atlas keeps its advance but draws nothing

        if(fits && atlasWidth < penX + bitmap._Width + 1)
        {
            penX = 0;
            penY += rowHeight + 1;
            rowHeight = 0;
        }

        //The size is estimated from the line height. the atlas grows downward when the rows outrun it
        while(fits && atlasHeight < penY + bitmap._Height)
        {
            atlasHeight *= 2;
            pixels.resize(static_cast<uSize>(atlasWidth) * atlasHeight);
        }

        for(uSize row = 0; fits && row < bitmap._Height; ++row)
            std::copy_n(bitmap._Pixels.data() + row * bitmap._Width, bitmap._Width, pixels.data() + (penY + row) * atlasWidth + penX);

        glyphsRef[codePoint - codePointFirst]._CodePoint = static_cast<u32>(codePoint);
        glyphsRef[codePoint - codePointFirst]._OriginX = penX;
        glyphsRef[codePoint - codePointFirst]._OriginY = penY;
        glyphsRef[codePoint - codePointFirst]._EndX = penX + (fits ? bitmap._Width : 0);
        glyphsRef[codePoint - codePointFirst]._EndY = penY + (fits ? bitmap._Height : 0);
        glyphsRef[codePoint - codePointFirst]._BearingX = static_cast<u32>(bitmap._BearingX);
        glyphsRef[codePoint - codePointFirst]._BearingY = static_cast<u32>(bitmap._BearingY);
        glyphsRef[codePoint - codePointFirst]._Advance = bitmap._Advance;

        if(!fits)
            continue;

        penX += bitmap._Width + 1;
        rowHeight = std::max(rowHeight, bitmap._Height);
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
    _hasUnflushedUploads = true;

    if(atlasDim)
    {
        atlasDim->_Width = atlasWidth;
        atlasDim->_Height = atlasHeight;
        atlasDim->_GlyphCount = glyphCount;
    }

    return textureID;
}

i32 RetroFuturaGUI::FontManager::initFreeTypeLibrary()
{
    i32 err = FT_Init_FreeType(&_ft);

    if (0 == err)
        return 0;

    std::println("ERROR::FREETYPE: Could not init FreeType Library");
    return -1;
}

const RetroFuturaGUI::FontInfo* RetroFuturaGUI::FontManager::GetFontInfo(std::string_view fontFamily, const f32 size, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    const u32 sizeKey { FontSizeToKey(ToPixels(size, sizeUnit)) };

    //Twice at most: the font may only be loaded in between
    for(u32 attempt = 0; attempt < 2; ++attempt)
    {
        for (const auto& font : _fonts)
        {
            if (font._Family == fontFamily
                && 0 < font._Atlasses.count(sizeKey)
                && font._Slant == slant
                && font._Weight == weight)
            {
                return &font;
            }
        }

        if(0 < attempt || Success != LoadFont(fontFamily, size, sizeUnit, slant, weight, BasicLatinFirst, BasicLatinLast))
            break;
    }

    return nullptr;
}

void RetroFuturaGUI::FontManager::SetDefaultFont(std::string_view fontFamily, const f32 size, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast)
{
    _defaultFontFamily = fontFamily;
    LoadFont(fontFamily, size, sizeUnit, slant, weight, codePointFirst, codePointLast);
}

std::vector<f32> RetroFuturaGUI::FontManager::GetOfferedSizes(std::string_view fontFamily, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const FontSizeUnit sizeUnit)
{
    if(!_ft)
        initFreeTypeLibrary();

    const PlatformBridge::Fonts::FontProperty fontProperty { resolveFontProperty(fontFamily, slant, weight) };
    const std::unique_ptr<IGlyphSource> source { FreeTypeGlyphSource::Open(_ft, fontProperty._Path, fontProperty._FaceIndex, weight) };

    if(!source)
        return {};

    std::vector<f32> sizes { source->GetPixelSizes() };

    for(f32& size : sizes)
        size = FromPixels(size, sizeUnit);

    return sizes;
}

u32 RetroFuturaGUI::FontManager::FontSizeToKey(const f32 pixels)
{
    return static_cast<u32>(std::max(1L, std::lround(pixels * 64.0f)));
}

void RetroFuturaGUI::FontManager::SetUploadContext(GLFWwindow* context)
{
    _uploadContext = context;
}

RetroFuturaGUI::FontManager::UploadContextScope::UploadContextScope()
{
    if(glfwGetCurrentContext())
        return;

    if(!_uploadContext)
        return;

    glfwMakeContextCurrent(_uploadContext);
    _isMadeCurrent = true;
}

RetroFuturaGUI::FontManager::UploadContextScope::~UploadContextScope()
{
    if(!_isMadeCurrent)
        return;

    //Flushed now, since FlushGlyphUploads may later run with no context current
    if(_hasUnflushedUploads)
        glFlush();

    _hasUnflushedUploads = false;
    glfwMakeContextCurrent(nullptr);
}

bool RetroFuturaGUI::FontManager::UploadContextScope::HasContext() const
{
    return nullptr != glfwGetCurrentContext();
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

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::FindGlyph(const FontInfo& font, const u32 sizeKey, const u32 codePoint)
{
    const auto atlas { font._Atlasses.find(sizeKey) };

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

    if(!glfwGetCurrentContext())
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
    if(!block._Source)
        return nullptr;

    if(block._CodePointFirst > codePoint || block._CodePointLast < codePoint)
        return nullptr;

    //The glyph goes into a texture, so it waits for a context; without one it's asked for again next time
    const UploadContextScope context {};

    if(!context.HasContext())
        return nullptr;

    Glyph glyph;
    glyph._CodePoint = codePoint;
    GlyphBitmap bitmap {};

    //A code point the font lacks renders its empty box, as in a block made at once. Kept like any other glyph, a failed one
    //too, so neither is rendered again
    if(block._Source->RenderGlyph(codePoint, &bitmap))
        storeGlyph(block, bitmap, &glyph);

    return &block._Glyphs.emplace(codePoint, glyph).first->second;
}

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::FindGlyphOutline(const FontInfo& font, const u32 sizeKey, const u32 codePoint, const f32 width)
{
    const auto atlas { font._Atlasses.find(sizeKey) };

    if(font._Atlasses.end() == atlas)
        return nullptr;

    const auto block { atlas->second._GlyphBlocks.find(blockKeyOf(codePoint)) };

    if(atlas->second._GlyphBlocks.end() == block)
        return nullptr;

    const u64 outlineKey { static_cast<u64>(FontSizeToKey(width)) << 32 | codePoint };
    const auto outline { block->second._OutlineGlyphs.find(outlineKey) };

    if(block->second._OutlineGlyphs.end() != outline)
        return &outline->second;

    GlyphBlock& editableBlock { const_cast<GlyphBlock&>(block->second) };

    if(!editableBlock._Source)
        return nullptr;

    if(editableBlock._CodePointFirst > codePoint || editableBlock._CodePointLast < codePoint)
        return nullptr;

    const UploadContextScope context {};

    if(!context.HasContext())
        return nullptr;

    Glyph glyph;
    glyph._CodePoint = codePoint;
    GlyphBitmap bitmap {};

    //Widened at the source's own size, so the outline is as wide as asked once the glyph is scaled to the font's size; kept
    //when it fails too (a bitmap font has no outline), so it isn't tried again.
    if(editableBlock._Source->RenderGlyphOutline(codePoint, width / editableBlock._Scale, &bitmap))
        storeGlyph(editableBlock, bitmap, &glyph);

    return &editableBlock._OutlineGlyphs.emplace(outlineKey, glyph).first->second;
}

void RetroFuturaGUI::FontManager::storeGlyph(GlyphBlock& block, const GlyphBitmap& bitmap, Glyph* glyph)
{
    //Measured at the font's size: a source drawing at a fixed size of its own is scaled to it
    const f32 scale { block._Scale };
    glyph->_Advance = static_cast<u32>(std::lround(bitmap._Advance * scale));
    glyph->_Bearing[0] = static_cast<i32>(std::lround(bitmap._BearingX * scale));
    glyph->_Bearing[1] = static_cast<i32>(std::lround(bitmap._BearingY * scale));
    glyph->_BearingX = static_cast<u32>(glyph->_Bearing[0]);
    glyph->_BearingY = static_cast<u32>(glyph->_Bearing[1]);
    glyph->_IsColor = GlyphPixelFormat::Color == bitmap._Format;

    //A space has no picture, and a glyph bigger than a page draws nothing; both keep their advance
    GlyphPage* page { 0 < bitmap._Width && 0 < bitmap._Height ? pageWithRoom(block, bitmap._Width, bitmap._Height, bitmap._Format) : nullptr };

    if(!page)
        return;

    const UploadState state { beginUpload() };
    glBindTexture(GL_TEXTURE_2D, page->_TextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(page->_PenX), static_cast<GLint>(page->_PenY), static_cast<GLsizei>(bitmap._Width), static_cast<GLsizei>(bitmap._Height),
        glyph->_IsColor ? GL_RGBA : GL_RED, GL_UNSIGNED_BYTE, bitmap._Pixels.data());
    endUpload(state);
    _hasUnflushedUploads = true;

    const f32 side { static_cast<f32>(block._PageSide) };
    glyph->_Size[0] = static_cast<i32>(std::lround(bitmap._Width * scale));
    glyph->_Size[1] = static_cast<i32>(std::lround(bitmap._Height * scale));
    glyph->_OriginX = page->_PenX;
    glyph->_OriginY = page->_PenY;
    glyph->_EndX = page->_PenX + bitmap._Width;
    glyph->_EndY = page->_PenY + bitmap._Height;
    glyph->_UV[0] = static_cast<f32>(glyph->_OriginX) / side;
    glyph->_UV[1] = static_cast<f32>(glyph->_OriginY) / side;
    glyph->_UV[2] = static_cast<f32>(glyph->_EndX) / side;
    glyph->_UV[3] = static_cast<f32>(glyph->_EndY) / side;
    glyph->_TextureID = page->_TextureID;

    page->_PenX += bitmap._Width + 1; //a pixel between glyphs, so filtering doesn't pick up the neighbor's edge!
    page->_RowHeight = std::max(page->_RowHeight, bitmap._Height);
}

RetroFuturaGUI::GlyphPage* RetroFuturaGUI::FontManager::pageWithRoom(GlyphBlock& block, const u32 width, const u32 height, const GlyphPixelFormat format)
{
    const u32 side { block._PageSide };
    const bool isColor { GlyphPixelFormat::Color == format };
    std::vector<GlyphPage>& pages { isColor ? block._ColorPages : block._Pages };

    if(side < width + 1 || side < height + 1)
        return nullptr;

    if(!pages.empty())
    {
        GlyphPage& page { pages.back() };

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
    const std::vector<u8> cleared(static_cast<uSize>(side) * side * (isColor ? 4 : 1));
    u32 textureID { 0 };
    glGenTextures(1, &textureID);
    const UploadState state { beginUpload() };
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, isColor ? GL_RGBA8 : GL_R8, static_cast<GLsizei>(side), static_cast<GLsizei>(side), 0, isColor ? GL_RGBA : GL_RED, GL_UNSIGNED_BYTE, cleared.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    endUpload(state);
    pages.push_back(GlyphPage { ._TextureID = textureID, ._PenX = 0, ._PenY = 0, ._RowHeight = 0 });
    return &pages.back();
}

u32 RetroFuturaGUI::FontManager::pageSideFor(const FontMetrics& metrics)
{
    const u32 rowHeight { static_cast<u32>(metrics._LineHeight) + 1 };
    u32 side { _pageSide };

    while(16 * rowHeight > side && _maxPageSide > side)
        side <<= 1;

    return side;
}

const std::list<RetroFuturaGUI::FontInfo>& RetroFuturaGUI::FontManager::GetFonts()
{
    return _fonts;
}

void RetroFuturaGUI::FontManager::ExtendFontset(std::string_view fontFamily, std::string_view fontFamilyExtension, const f32 size, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight, const u32 codePointFirst, const u32 codePointLast)
{
    const u32 sizeKey { FontSizeToKey(ToPixels(size, sizeUnit)) };

    for (auto& font : _fonts)
    {
        if (font._Family == fontFamily
            && 0 < font._Atlasses.count(sizeKey)
            && font._Slant == slant
            && font._Weight == weight)
        {
            if(LoadFont(fontFamilyExtension, size, sizeUnit, slant, weight, codePointFirst, codePointLast, true) != LoadFontResult::Success)
                return;

            GlyphAtlas
                &atlas { font._Atlasses[sizeKey] },
                &extension { _fonts.back()._Atlasses[sizeKey] };
            GlyphBlock& block { atlas._GlyphBlocks[codePointFirst] };
            block = std::move(extension._GlyphBlocks[codePointFirst]);

            //An extension drawing at a fixed size of its own, like a bitmap emoji font, is scaled to the font's size: the
            //glyphs made so far now, the others as they are made
            block._Scale = atlas._FontSize / std::max(extension._FontSize, 1.0f);

            for(auto& [glyphCodePoint, glyph] : block._Glyphs)
            {
                glyph._Size[0] = static_cast<i32>(std::lround(glyph._Size[0] * block._Scale));
                glyph._Size[1] = static_cast<i32>(std::lround(glyph._Size[1] * block._Scale));
                glyph._Bearing[0] = static_cast<i32>(std::lround(glyph._Bearing[0] * block._Scale));
                glyph._Bearing[1] = static_cast<i32>(std::lround(glyph._Bearing[1] * block._Scale));
                glyph._Advance = static_cast<u32>(std::lround(glyph._Advance * block._Scale));
            }

            //The new font may have what another font was found to draw
            atlas._FallbackFor.clear();
            _fonts.pop_back();
            return;
        }
    }
}

void RetroFuturaGUI::FontManager::ShapeText(const FontInfo& font, const u32 sizeKey, std::u32string_view text, const ShapeDirection direction, std::vector<PlacedGlyph>* glyphs)
{
    if(!glyphs)
        return;

    glyphs->clear();
    const auto found { font._Atlasses.find(sizeKey) };

    if(font._Atlasses.end() == found)
        return;

    //The atlas is the font's, and the fallbacks found for it are kept in it
    GlyphAtlas& atlas { const_cast<GlyphAtlas&>(found->second) };
    const bool isRightToLeft { ShapeDirection::RightToLeft == direction };

    //The block drawing each character. Joiners, variation selectors, skin tones and marks go with the character before them,
    //and so does whatever follows a joiner, so the font sees a whole sequence (⬛ in 🐈‍⬛ is in a block the text may not load)
    std::vector<GlyphBlock*> drawing(text.size(), nullptr);

    for(uSize character = 0; character < text.size(); ++character)
    {
        const bool isJoined { 0 < character && drawing[character - 1] != nullptr && (isAttaching(text[character]) || 0x200D == text[character - 1]) };
        drawing[character] = isJoined ? drawing[character - 1] : findDrawingBlock(font, atlas, text[character]);
    }

    //One run per font file, shaped at once; right to left the runs are laid out from the last one
    std::vector<std::vector<PlacedGlyph>> runs {};
    std::vector<ShapedGlyph> shaped {};

    for(uSize start = 0; start < text.size();)
    {
        GlyphBlock* block { drawing[start] };
        IGlyphSource* source { block ? block->_Source.get() : nullptr };
        uSize end { start + 1 };

        while(end < text.size() && block && drawing[end] && drawing[end]->_FontFile == block->_FontFile)
            ++end;

        std::vector<PlacedGlyph>& run { runs.emplace_back() };

        if(!source || !source->Shape(text.substr(start, end - start), direction, &shaped))
        {
            for(uSize character = start; character < end; ++character)
            {
                run.push_back(PlacedGlyph
                {
                    ._Block = findBlock(atlas, text[character]),
                    ._GlyphIndex = 0,
                    ._CodePoint = text[character],
                    ._Cluster = static_cast<u32>(character),
                    ._Advance = glm::vec2(0.0f),
                    ._Offset = glm::vec2(0.0f),
                    ._IsShaped = false,
                    ._IsRightToLeft = isRightToLeft
                });
            }

            if(isRightToLeft)
                std::ranges::reverse(run);

            start = end;
            continue;
        }

        //Measured at the font's size, like the glyphs
        for(const ShapedGlyph& glyph : shaped)
        {
            run.push_back(PlacedGlyph
            {
                ._Block = block,
                ._GlyphIndex = glyph._GlyphIndex,
                ._CodePoint = text[start + glyph._Cluster],
                ._Cluster = static_cast<u32>(start + glyph._Cluster),
                ._Advance = glm::vec2(glyph._AdvanceX, glyph._AdvanceY) * block->_Scale,
                ._Offset = glm::vec2(glyph._OffsetX, glyph._OffsetY) * block->_Scale,
                ._IsShaped = true,
                ._IsRightToLeft = isRightToLeft
            });
        }

        start = end;
    }

    if(isRightToLeft)
        std::ranges::reverse(runs);

    for(const std::vector<PlacedGlyph>& run : runs)
        glyphs->insert(glyphs->end(), run.begin(), run.end());
}

RetroFuturaGUI::GlyphBlock* RetroFuturaGUI::FontManager::findDrawingBlock(const FontInfo& font, GlyphAtlas& atlas, const u32 codePoint)
{
    GlyphBlock* own { findBlock(atlas, codePoint) };

    if(own && own->_Source && own->_Source->HasGlyph(codePoint))
        return own;

    const auto known { atlas._FallbackFor.find(codePoint) };

    if(atlas._FallbackFor.end() != known)
        return known->second ? known->second : own;

    //The fontset's other fonts in the order of their blocks, each font file asked once, then the default font
    std::vector<u32> blockKeys {};

    for(const auto& [blockKey, block] : atlas._GlyphBlocks)
        blockKeys.push_back(blockKey);

    std::ranges::sort(blockKeys);
    std::vector<std::string_view> askedFiles {};
    GlyphBlock* fallback { nullptr };

    for(const u32 blockKey : blockKeys)
    {
        GlyphBlock& block { atlas._GlyphBlocks.at(blockKey) };

        if(!block._Source || std::ranges::contains(askedFiles, std::string_view(block._FontFile)))
            continue;

        askedFiles.push_back(block._FontFile);

        if(!block._Source->HasGlyph(codePoint))
            continue;

        fallback = &block;
        break;
    }

    if(!fallback)
    {
        GlyphBlock* defaultFont { getDefaultFontBlock(font, atlas) };

        if(defaultFont && defaultFont->_Source && defaultFont->_Source->HasGlyph(codePoint))
            fallback = defaultFont;
    }

    atlas._FallbackFor[codePoint] = fallback;
    return fallback ? fallback : own;
}

RetroFuturaGUI::GlyphBlock* RetroFuturaGUI::FontManager::getDefaultFontBlock(const FontInfo& font, GlyphAtlas& atlas)
{
    if(atlas._IsDefaultFontLookedFor)
        return atlas._FallbackBlocks.empty() ? nullptr : &atlas._FallbackBlocks.front();

    atlas._IsDefaultFontLookedFor = true;

    //The default font, else the first loaded one, in the style of the font asked for
    const std::string family { _defaultFontFamily.empty() && !_fonts.empty() ? _fonts.front()._FontProperty._Name : _defaultFontFamily };

    if(family.empty())
        return nullptr;

    const PlatformBridge::Fonts::FontProperty property { resolveFontProperty(family, font._Slant, font._Weight) };
    const std::string fontFile { property._Path + "#" + std::to_string(property._FaceIndex) };

    if(property._Path.empty())
        return nullptr;

    for(const auto& [blockKey, block] : atlas._GlyphBlocks)
    {
        if(block._FontFile == fontFile)
            return nullptr;
    }

    std::unique_ptr<IGlyphSource> source { FreeTypeGlyphSource::Open(_ft, property._Path, property._FaceIndex, font._Weight) };

    if(!source || !source->SetPixelSize(atlas._FontSize))
        return nullptr;

    //Glyphs it draws are found by shaping, by their index, and rendered into its own pages as they're first shown
    const f32 scale { atlas._FontSize / std::max(source->GetPixelSize(), 1.0f) };
    atlas._FallbackBlocks.push_back(GlyphBlock
    {
        ._TextureID = 0,
        ._Width = 0,
        ._Height = 0,
        ._CodePointFirst = 0,
        ._CodePointLast = 0,
        ._LoadIndividually = true,
        ._Glyphs = {},
        ._OutlineGlyphs = {},
        ._IndexGlyphs = {},
        ._IndexOutlineGlyphs = {},
        ._ShadowGlyphs = {},
        ._InnerGlowGlyphs = {},
        ._FontFile = fontFile,
        ._Source = std::move(source),
        ._Pages = {},
        ._ColorPages = {},
        ._PageSide = pageSideFor(atlas._Metrics),
        ._Scale = scale
    });

    return &atlas._FallbackBlocks.front();
}

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::FindPlacedGlyph(const FontInfo& font, const u32 sizeKey, const PlacedGlyph& placed)
{
    if(!placed._Block)
        return nullptr;

    if(!placed._IsShaped)
        return FindGlyph(font, sizeKey, placed._CodePoint);

    GlyphBlock& block { *placed._Block };
    const auto found { block._IndexGlyphs.find(placed._GlyphIndex) };

    if(block._IndexGlyphs.end() != found)
        return &found->second;

    if(!block._Source)
        return nullptr;

    //Rendered into the block's pages like a glyph made on first use; without a context it's asked for again next time
    const UploadContextScope context {};

    if(!context.HasContext())
        return nullptr;

    Glyph glyph;
    glyph._CodePoint = placed._CodePoint;
    GlyphBitmap bitmap {};

    if(block._Source->RenderGlyphIndex(placed._GlyphIndex, &bitmap))
        storeGlyph(block, bitmap, &glyph);

    return &block._IndexGlyphs.emplace(placed._GlyphIndex, glyph).first->second;
}

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::FindPlacedGlyphOutline(const FontInfo& font, const u32 sizeKey, const PlacedGlyph& placed, const f32 width)
{
    if(!placed._Block)
        return nullptr;

    if(!placed._IsShaped)
        return FindGlyphOutline(font, sizeKey, placed._CodePoint, width);

    GlyphBlock& block { *placed._Block };
    const u64 outlineKey { static_cast<u64>(FontSizeToKey(width)) << 32 | placed._GlyphIndex };
    const auto found { block._IndexOutlineGlyphs.find(outlineKey) };

    if(block._IndexOutlineGlyphs.end() != found)
        return &found->second;

    if(!block._Source)
        return nullptr;

    const UploadContextScope context {};

    if(!context.HasContext())
        return nullptr;

    Glyph glyph;
    glyph._CodePoint = placed._CodePoint;
    GlyphBitmap bitmap {};

    //Widened at the source's own size, as FindGlyphOutline does.
    if(block._Source->RenderGlyphIndexOutline(placed._GlyphIndex, width / block._Scale, &bitmap))
        storeGlyph(block, bitmap, &glyph);

    return &block._IndexOutlineGlyphs.emplace(outlineKey, glyph).first->second;
}

RetroFuturaGUI::GlyphBlock* RetroFuturaGUI::FontManager::findBlock(const GlyphAtlas& atlas, const u32 codePoint)
{
    const auto block { atlas._GlyphBlocks.find(blockKeyOf(codePoint)) };

    if(atlas._GlyphBlocks.end() == block)
        return nullptr;

    if(block->second._CodePointFirst > codePoint || block->second._CodePointLast < codePoint)
        return nullptr;

    //The atlas is the font's, and the glyphs found in it are rendered into its blocks
    return const_cast<GlyphBlock*>(&block->second);
}

bool RetroFuturaGUI::FontManager::isAttaching(const u32 codePoint)
{
    static constexpr std::array<std::pair<u32, u32>, 10> attaching
    {{
        { 0x0300, 0x036F }, //combining diacritical marks
        { 0x1AB0, 0x1AFF }, //and their extensions
        { 0x1DC0, 0x1DFF },
        { 0x200C, 0x200D }, //zero-width non-joiner and joiner
        { 0x20D0, 0x20FF }, //combining marks for symbols, the keycap among them
        { 0xFE00, 0xFE0F }, //variation selectors: text or emoji look
        { 0xFE20, 0xFE2F }, //combining half marks
        { 0x1F3FB, 0x1F3FF }, //skin tones
        { 0xE0020, 0xE007F }, //tags, which spell out subdivision flags
        { 0xE0100, 0xE01EF } //more variation selectors
    }};

    return std::ranges::any_of(attaching, [codePoint](const std::pair<u32, u32>& range) { return range.first <= codePoint && range.second >= codePoint; });
}

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::FindPlacedGlyphShadow(const PlacedGlyph& placed, const f32 blur, const f32 outlineWidth)
{
    if(!placed._Block)
        return nullptr;

    GlyphBlock& block { *placed._Block };
    const u64 key { shadowGlyphKey(placed, blur, outlineWidth) };
    const auto found { block._ShadowGlyphs.find(key) };

    if(block._ShadowGlyphs.end() != found)
        return &found->second;

    if(!block._Source)
        return nullptr;

    const UploadContextScope context {};

    if(!context.HasContext())
        return nullptr;

    Glyph glyph;
    glyph._CodePoint = placed._CodePoint;
    GlyphBitmap bitmap {};

    //At the source's own size, like an outline, so the outline and the blur are as asked once scaled to the font's size. Kept
    //when it fails too, so it isn't tried again
    if(renderShadowShape(block, placed, outlineWidth / block._Scale, &bitmap))
    {
        castShadow(blur / block._Scale, &bitmap);
        storeGlyph(block, bitmap, &glyph);
    }

    return &block._ShadowGlyphs.emplace(key, glyph).first->second;
}

u64 RetroFuturaGUI::FontManager::shadowGlyphKey(const PlacedGlyph& placed, const f32 blur, const f32 outlineWidth)
{
    const u64
        isShaped { placed._IsShaped ? 1ull : 0ull },
        blurKey { static_cast<u64>(std::clamp(std::lround(blur * 16.0f), 0l, 0x7FFFl)) },
        outlineKey { static_cast<u64>(std::clamp(std::lround(outlineWidth * 16.0f), 0l, 0xFFFFl)) },
        glyphKey { placed._IsShaped ? placed._GlyphIndex : placed._CodePoint };

    return isShaped << 63 | blurKey << 48 | outlineKey << 32 | glyphKey;
}

bool RetroFuturaGUI::FontManager::renderShadowShape(GlyphBlock& block, const PlacedGlyph& placed, const f32 outlineWidth, GlyphBitmap* bitmap)
{
    if(!block._Source)
        return false;

    if(placed._IsShaped)
    {
        if(0.0f < outlineWidth)
            return block._Source->RenderGlyphIndexOutline(placed._GlyphIndex, outlineWidth, bitmap);

        return block._Source->RenderGlyphIndex(placed._GlyphIndex, bitmap);
    }

    if(0.0f < outlineWidth)
        return block._Source->RenderGlyphOutline(placed._CodePoint, outlineWidth, bitmap);

    return block._Source->RenderGlyph(placed._CodePoint, bitmap);
}

void RetroFuturaGUI::FontManager::castShadow(const f32 blur, GlyphBitmap* bitmap)
{
    if(!bitmap)
        return;

    toCoverage(bitmap); //a color glyph casts its shadow by its alpha

    if(0.0f >= blur || 0 == bitmap->_Width || 0 == bitmap->_Height)
        return;

    //A Gaussian reaching two spreads to each side fades an edge over the whole blur, as the rectangles' shadows do. The
    //bitmap grows by that reach, so nothing of the fade is cut off
    const f32 spread { blur * 0.25f };
    const i32 reach { static_cast<i32>(std::ceil(spread * 2.0f)) };
    const std::vector<f32> blurred { blurCoverage(*bitmap, spread, reach) };
    bitmap->_Pixels.resize(blurred.size());

    for(uSize pixel { 0 }; pixel < blurred.size(); ++pixel)
        bitmap->_Pixels[pixel] = static_cast<u8>(std::lround(std::clamp(blurred[pixel], 0.0f, 1.0f) * 255.0f));

    bitmap->_Width += static_cast<u32>(reach) * 2;
    bitmap->_Height += static_cast<u32>(reach) * 2;
    bitmap->_BearingX -= reach;
    bitmap->_BearingY += reach;
}

const RetroFuturaGUI::Glyph* RetroFuturaGUI::FontManager::FindPlacedGlyphInnerGlow(const PlacedGlyph& placed, const f32 width, const glm::vec2& offset)
{
    if(!placed._Block)
        return nullptr;

    GlyphBlock& block { *placed._Block };

    //At the source's own size, so the width and the offset are as asked once scaled to the font's size
    const glm::ivec2 sourceOffset { glm::round(offset / block._Scale) };
    const u64 key { innerGlowGlyphKey(placed, width, sourceOffset) };
    const auto found { block._InnerGlowGlyphs.find(key) };

    if(block._InnerGlowGlyphs.end() != found)
        return &found->second;

    if(!block._Source)
        return nullptr;

    const UploadContextScope context {};

    if(!context.HasContext())
        return nullptr;

    Glyph glyph;
    glyph._CodePoint = placed._CodePoint;
    GlyphBitmap bitmap {};

    //Kept when it fails too, so it isn't tried again.
    if(renderShadowShape(block, placed, 0.0f, &bitmap))
    {
        castInnerGlow(width / block._Scale, sourceOffset, &bitmap);
        storeGlyph(block, bitmap, &glyph);
    }

    return &block._InnerGlowGlyphs.emplace(key, glyph).first->second;
}

u64 RetroFuturaGUI::FontManager::innerGlowGlyphKey(const PlacedGlyph& placed, const f32 width, const glm::ivec2& offset)
{
    const u64
        isShaped { placed._IsShaped ? 1ull : 0ull },
        widthKey { static_cast<u64>(std::clamp(std::lround(width * 16.0f), 0l, 0x1FFFl)) },
        offsetXKey { static_cast<u64>(std::clamp(offset.x, -255, 255) + 256) },
        offsetYKey { static_cast<u64>(std::clamp(offset.y, -255, 255) + 256) },
        glyphKey { placed._IsShaped ? placed._GlyphIndex : placed._CodePoint };

    return isShaped << 63 | widthKey << 50 | offsetXKey << 41 | offsetYKey << 32 | glyphKey;
}

void RetroFuturaGUI::FontManager::castInnerGlow(const f32 width, const glm::ivec2& offset, GlyphBitmap* bitmap)
{
    if(!bitmap)
        return;

    toCoverage(bitmap); //a color glyph glows inside its alpha.

    if(0 == bitmap->_Width || 0 == bitmap->_Height)
        return;

    if(0.0f >= width)
    {
        std::ranges::fill(bitmap->_Pixels, u8 { 0 });
        return;
    }

    //The blurred glyph is a half at its edges and rises to whole within two spreads inside, so twice what it lacks of whole
    //is full at the edge and gone at the width. Read where the moved edges lie, and nothing beyond the blur
    const f32 spread { width * 0.5f };
    const i32
        reach { static_cast<i32>(std::ceil(spread * 2.0f)) },
        glyphWidth { static_cast<i32>(bitmap->_Width) },
        glyphHeight { static_cast<i32>(bitmap->_Height) },
        grownWidth { glyphWidth + reach * 2 },
        grownHeight { glyphHeight + reach * 2 };
    const std::vector<f32> blurred { blurCoverage(*bitmap, spread, reach) };

    for(i32 y { 0 }; y < glyphHeight; ++y)
        for(i32 x { 0 }; x < glyphWidth; ++x)
        {
            u8& pixel { bitmap->_Pixels[static_cast<uSize>(y) * glyphWidth + x] };

            if(0 == pixel)
                continue;

            //Rows run down, the offset up
            const i32
                sourceX { x - offset.x + reach },
                sourceY { y + offset.y + reach };
            const bool isInside { 0 <= sourceX && grownWidth > sourceX && 0 <= sourceY && grownHeight > sourceY };
            const f32
                inside { isInside ? blurred[static_cast<uSize>(sourceY) * grownWidth + sourceX] : 0.0f },
                light { std::clamp((1.0f - inside) * 2.0f, 0.0f, 1.0f) };

            pixel = static_cast<u8>(std::lround(pixel * light));
        }
}

void RetroFuturaGUI::FontManager::toCoverage(GlyphBitmap* bitmap)
{
    if(!bitmap)
        return;

    if(GlyphPixelFormat::Color != bitmap->_Format)
        return;

    const uSize pixelCount { static_cast<uSize>(bitmap->_Width) * bitmap->_Height };
    std::vector<u8> coverage(pixelCount);

    for(uSize pixel { 0 }; pixel < pixelCount; ++pixel)
        coverage[pixel] = bitmap->_Pixels[pixel * 4 + 3];

    bitmap->_Pixels = std::move(coverage);
    bitmap->_Format = GlyphPixelFormat::Coverage;
}

std::vector<f32> RetroFuturaGUI::FontManager::blurCoverage(const GlyphBitmap& bitmap, const f32 spread, const i32 reach)
{
    std::vector<f32> weights(static_cast<uSize>(reach) * 2 + 1);
    f32 weightSum { 0.0f };

    for(i32 offset { -reach }; offset <= reach; ++offset)
    {
        const f32 weight { std::exp(-static_cast<f32>(offset * offset) / (2.0f * spread * spread)) };
        weights[static_cast<uSize>(offset + reach)] = weight;
        weightSum += weight;
    }

    for(f32& weight : weights)
        weight /= weightSum;

    const i32
        width { static_cast<i32>(bitmap._Width) },
        height { static_cast<i32>(bitmap._Height) },
        grownWidth { width + reach * 2 },
        grownHeight { height + reach * 2 };
    std::vector<f32>
        rows(static_cast<uSize>(grownWidth) * height, 0.0f),
        columns(static_cast<uSize>(grownWidth) * grownHeight, 0.0f);

    //Across first, into the grown width, then down, into the grown height.
    for(i32 y { 0 }; y < height; ++y)
        for(i32 x { 0 }; x < width; ++x)
        {
            const f32 value { bitmap._Pixels[static_cast<uSize>(y) * width + x] / 255.0f };

            if(0.0f == value)
                continue;

            for(i32 offset { -reach }; offset <= reach; ++offset)
                rows[static_cast<uSize>(y) * grownWidth + x + reach + offset] += value * weights[static_cast<uSize>(offset + reach)];
        }

    for(i32 y { 0 }; y < height; ++y)
        for(i32 x { 0 }; x < grownWidth; ++x)
        {
            const f32 value { rows[static_cast<uSize>(y) * grownWidth + x] };

            if(0.0f == value)
                continue;

            for(i32 offset { -reach }; offset <= reach; ++offset)
                columns[static_cast<uSize>(y + reach + offset) * grownWidth + x] += value * weights[static_cast<uSize>(offset + reach)];
        }

    return columns;
}
