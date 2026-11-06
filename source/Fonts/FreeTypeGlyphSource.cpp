#include "FreeTypeGlyphSource.hpp"
#include "ColorGlyphPainter.hpp"
#include <algorithm>
#include <cmath>
#include <string>

std::unique_ptr<RetroFuturaGUI::FreeTypeGlyphSource> RetroFuturaGUI::FreeTypeGlyphSource::Open(FT_Library library, std::string_view path, const u32 faceIndex, const PlatformBridge::Fonts::Weight weight)
{
    if(!library)
        return nullptr;

    FT_Face face { nullptr };

    if(FT_New_Face(library, std::string(path).c_str(), static_cast<FT_Long>(faceIndex), &face))
        return nullptr;

    setVariableWeight(library, face, weight);
    return std::unique_ptr<FreeTypeGlyphSource>(new FreeTypeGlyphSource(face, path, faceIndex));
}

RetroFuturaGUI::FreeTypeGlyphSource::FreeTypeGlyphSource(FT_Face face, std::string_view path, const u32 faceIndex)
    : _face(face), _path(path), _faceIndex(faceIndex)
{
}

hb_face_t* RetroFuturaGUI::FreeTypeGlyphSource::getSharedFace(std::string_view path, const u32 faceIndex)
{
    const std::string key { std::string(path) + "#" + std::to_string(faceIndex) };
    const auto found { _sharedFaces.find(key) };

    if(_sharedFaces.end() != found)
        return found->second;

    //The file read once into a blob the face reads its tables from; a file HarfBuzz can't read gives an empty face
    hb_blob_t* blob { hb_blob_create_from_file_or_fail(std::string(path).c_str()) };

    if(!blob)
        return nullptr;

    hb_face_t* face { hb_face_create(blob, faceIndex) };
    hb_blob_destroy(blob); //the face holds a reference of its own
    _sharedFaces.emplace(key, face);
    return face;
}

void RetroFuturaGUI::FreeTypeGlyphSource::setVariableWeight(FT_Library library, FT_Face face, const PlatformBridge::Fonts::Weight weight)
{
    if(!FT_HAS_MULTIPLE_MASTERS(face))
        return;

    FT_MM_Var* axes { nullptr };

    if(FT_Get_MM_Var(face, &axes))
        return;

    //Only the weight moves
    std::vector<FT_Fixed> coordinates(axes->num_axis);
    bool hasWeightAxis { false };

    for(FT_UInt axis = 0; axis < axes->num_axis; ++axis)
    {
        const FT_Var_Axis& range { axes->axis[axis] };
        coordinates[axis] = range.def;

        if(FT_MAKE_TAG('w', 'g', 'h', 't') != range.tag)
            continue;

        coordinates[axis] = std::clamp(static_cast<FT_Fixed>(static_cast<i32>(weight)) << 16, range.minimum, range.maximum); //16.16 fixed point
        hasWeightAxis = true;
    }

    if(hasWeightAxis)
        FT_Set_Var_Design_Coordinates(face, axes->num_axis, coordinates.data());

    FT_Done_MM_Var(library, axes);
}

RetroFuturaGUI::FreeTypeGlyphSource::~FreeTypeGlyphSource()
{
    hb_buffer_destroy(_shapingBuffer);
    hb_font_destroy(_paintFont);
    hb_font_destroy(_shapingFont);

    if(!_face)
        return;

    FT_Done_Face(_face);
}

bool RetroFuturaGUI::FreeTypeGlyphSource::SetPixelSize(const f32 pixels)
{
    if(!_face)
        return false;

    //A font with only fixed sizes (bitmap strikes) takes the closest one
    if(!FT_IS_SCALABLE(_face))
    {
        if(0 >= _face->num_fixed_sizes)
            return false;

        FT_Int closest { 0 };

        for(FT_Int sizeIndex = 1; sizeIndex < _face->num_fixed_sizes; ++sizeIndex)
        {
            if(std::abs(_face->available_sizes[sizeIndex].y_ppem / 64.0f - pixels) < std::abs(_face->available_sizes[closest].y_ppem / 64.0f - pixels))
                closest = sizeIndex;
        }

        if(FT_Select_Size(_face, closest))
            return false;

        _pixelSize = _face->available_sizes[closest].y_ppem / 64.0f;
        return true;
    }

    //In 1/64 pixels at 72 dpi, where a point is a pixel, so a fractional size keeps its fraction
    if(FT_Set_Char_Size(_face, 0, static_cast<FT_F26Dot6>(std::lround(pixels * 64.0f)), 72, 72))
        return false;

    _pixelSize = pixels;

    //HarfBuzz's fonts follow if they were made at another size
    if(_shapingFont)
        hb_ft_font_changed(_shapingFont);

    if(_paintFont)
    {
        const int32_t scale { static_cast<int>(std::lround(_pixelSize * 64.0f)) };
        hb_font_set_scale(_paintFont, scale, scale);
    }

    return true;
}

f32 RetroFuturaGUI::FreeTypeGlyphSource::GetPixelSize() const
{
    return _pixelSize;
}

std::vector<f32> RetroFuturaGUI::FreeTypeGlyphSource::GetPixelSizes() const
{
    if(!_face)
        return {};

    if(FT_IS_SCALABLE(_face))
        return {};

    std::vector<f32> sizes {};

    for(FT_Int sizeIndex = 0; sizeIndex < _face->num_fixed_sizes; ++sizeIndex)
        sizes.push_back(_face->available_sizes[sizeIndex].y_ppem / 64.0f);

    std::ranges::sort(sizes);
    return sizes;
}

RetroFuturaGUI::FontMetrics RetroFuturaGUI::FreeTypeGlyphSource::GetMetrics() const
{
    if(!_face)
        return {};

    if(!_face->size)
        return {};

    const FT_Size_Metrics& metrics { _face->size->metrics };
    return FontMetrics
    {
        ._Ascender = metrics.ascender / 64.0f,
        ._Descender = -metrics.descender / 64.0f, //FreeType's runs negative below the baseline
        ._LineHeight = metrics.height / 64.0f
    };
}

bool RetroFuturaGUI::FreeTypeGlyphSource::HasColorGlyphs() const
{
    if(!_face)
        return false;

    return FT_HAS_COLOR(_face);
}

bool RetroFuturaGUI::FreeTypeGlyphSource::HasGlyph(const u32 codePoint) const
{
    if(!_face)
        return false;

    return 0 != FT_Get_Char_Index(_face, static_cast<FT_ULong>(codePoint));
}

bool RetroFuturaGUI::FreeTypeGlyphSource::RenderGlyph(const u32 codePoint, GlyphBitmap* bitmap)
{
    //A character the font lacks has index 0
    return RenderGlyphIndex(_face ? FT_Get_Char_Index(_face, static_cast<FT_ULong>(codePoint)) : 0, bitmap);
}

bool RetroFuturaGUI::FreeTypeGlyphSource::RenderGlyphBorder(const u32 codePoint, const f32 width, GlyphBitmap* bitmap)
{
    return RenderGlyphIndexBorder(_face ? FT_Get_Char_Index(_face, static_cast<FT_ULong>(codePoint)) : 0, width, bitmap);
}

bool RetroFuturaGUI::FreeTypeGlyphSource::RenderGlyphIndex(const u32 glyphIndex, GlyphBitmap* bitmap)
{
    if(!bitmap)
        return false;

    *bitmap = GlyphBitmap {};

    if(!_face)
        return false;

    //COLR version 1 paint first the gradients the font was drawn with. the flat layers of version 0 are what's left when it fails!
    if(FT_HAS_COLOR(_face) && ColorGlyphPainter::HasPaint(_face, glyphIndex))
    {
        //The advance before painting, which loads the outlines it clips to into the same slot
        if(FT_Load_Glyph(_face, glyphIndex, getLoadFlags()))
            return false;

        const u32 advance { static_cast<u32>(_face->glyph->advance.x >> 6) };

        if(ColorGlyphPainter::Paint(_face, getPaintFont(), glyphIndex, bitmap))
        {
            bitmap->_Advance = advance;
            return true;
        }
    }

    if(FT_Load_Glyph(_face, glyphIndex, FT_LOAD_RENDER | getLoadFlags()))
        return false;

    const FT_GlyphSlot slot { _face->glyph };
    bitmap->_Advance = static_cast<u32>(slot->advance.x >> 6);
    bitmap->_BearingX = slot->bitmap_left;
    bitmap->_BearingY = slot->bitmap_top;

    if(copyBitmap(slot->bitmap, bitmap))
        return true;

    *bitmap = GlyphBitmap {};
    return false;
}

bool RetroFuturaGUI::FreeTypeGlyphSource::RenderGlyphIndexBorder(const u32 glyphIndex, const f32 width, GlyphBitmap* bitmap)
{
    if(!bitmap)
        return false;

    *bitmap = GlyphBitmap {};

    if(!_face)
        return false;

    //The outline, hinted as the glyph itself is, so the border lines up with it
    if(FT_Load_Glyph(_face, glyphIndex, FT_LOAD_NO_BITMAP | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT))
        return false;

    if(FT_GLYPH_FORMAT_OUTLINE != _face->glyph->format)
        return false;

    FT_Glyph glyph { nullptr };

    if(FT_Get_Glyph(_face->glyph, &glyph))
        return false;

    FT_Stroker stroker { nullptr };

    if(FT_Stroker_New(_face->glyph->library, &stroker))
    {
        FT_Done_Glyph(glyph);
        return false;
    }

    //Only the outer side of the stroke: filled, it is the glyph grown by the width, holes shrunk by it
    FT_Stroker_Set(stroker, static_cast<FT_Fixed>(std::lround(width * 64.0f)), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    const bool isStroked { 0 == FT_Glyph_StrokeBorder(&glyph, stroker, false, true) };
    FT_Stroker_Done(stroker);

    if(!isStroked || FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true))
    {
        FT_Done_Glyph(glyph);
        return false;
    }

    const FT_BitmapGlyph rendered { reinterpret_cast<FT_BitmapGlyph>(glyph) };
    bitmap->_Advance = static_cast<u32>(_face->glyph->advance.x >> 6);
    bitmap->_BearingX = rendered->left;
    bitmap->_BearingY = rendered->top;
    const bool isCopied { copyBitmap(rendered->bitmap, bitmap) };
    FT_Done_Glyph(glyph);

    if(isCopied)
        return true;

    *bitmap = GlyphBitmap {};
    return false;
}

bool RetroFuturaGUI::FreeTypeGlyphSource::Shape(std::u32string_view text, const ShapeDirection direction, std::vector<ShapedGlyph>* glyphs)
{
    if(!glyphs)
        return false;

    glyphs->clear();

    if(!_face)
        return false;

    hb_font_t* shapingFont { getShapingFont() };

    if(!shapingFont)
        return false;

    if(!_shapingBuffer)
        _shapingBuffer = hb_buffer_create();

    if(!hb_buffer_allocation_successful(_shapingBuffer))
        return false;

    //The script and language from the text itself, the direction as the text flows
    hb_buffer_clear_contents(_shapingBuffer);
    hb_buffer_add_utf32(_shapingBuffer, reinterpret_cast<const uint32_t*>(text.data()), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));
    hb_buffer_set_direction(_shapingBuffer, ShapeDirection::TopToBottom == direction ? HB_DIRECTION_TTB : ShapeDirection::RightToLeft == direction ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
    hb_buffer_guess_segment_properties(_shapingBuffer);
    hb_shape(shapingFont, _shapingBuffer, nullptr, 0);

    uint32_t glyphCount { 0 };
    const hb_glyph_info_t* infos { hb_buffer_get_glyph_infos(_shapingBuffer, &glyphCount) };
    const hb_glyph_position_t* positions { hb_buffer_get_glyph_positions(_shapingBuffer, &glyphCount) };
    glyphs->reserve(glyphCount);

    for(uint32_t glyph = 0; glyph < glyphCount; ++glyph)
    {
        glyphs->push_back(ShapedGlyph
        {
            ._GlyphIndex = infos[glyph].codepoint, //after shaping it holds the glyph index
            ._Cluster = infos[glyph].cluster,
            ._AdvanceX = positions[glyph].x_advance / 64.0f,
            ._AdvanceY = positions[glyph].y_advance / 64.0f,
            ._OffsetX = positions[glyph].x_offset / 64.0f,
            ._OffsetY = positions[glyph].y_offset / 64.0f
        });
    }

    return true;
}

FT_Int32 RetroFuturaGUI::FreeTypeGlyphSource::getLoadFlags() const
{
    //A color font's layers are drawn as the font made them, unhinted; every other font keeps the light autohinting
    return _face && FT_HAS_COLOR(_face) ? FT_LOAD_COLOR : FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;
}

hb_font_t* RetroFuturaGUI::FreeTypeGlyphSource::getShapingFont()
{
    if(!_face)
        return nullptr;

    //Its advances come from the same hinting the glyphs are rendered with
    if(!_shapingFont)
    {
        _shapingFont = hb_ft_font_create_referenced(_face);
        hb_ft_font_set_load_flags(_shapingFont, getLoadFlags());
    }

    return _shapingFont;
}

hb_font_t* RetroFuturaGUI::FreeTypeGlyphSource::getPaintFont()
{
    if(_paintFont)
        return _paintFont;

    hb_font_t* shapingFont { getShapingFont() };

    if(!shapingFont)
        return nullptr;

    hb_face_t* face { getSharedFace(_path, _faceIndex) };

    if(!face)
        return nullptr;

    //The face every source of this font file shares, at this source's size and variable font settings
    _paintFont = hb_font_create(face);
    const int32_t scale { static_cast<int>(std::lround(_pixelSize * 64.0f)) };
    hb_font_set_scale(_paintFont, scale, scale);
    uint32_t coordinateCount { 0 };
    const int* coordinates { hb_font_get_var_coords_normalized(shapingFont, &coordinateCount) };
    hb_font_set_var_coords_normalized(_paintFont, coordinates, coordinateCount);
    return _paintFont;
}

bool RetroFuturaGUI::FreeTypeGlyphSource::copyBitmap(const FT_Bitmap& rendered, GlyphBitmap* bitmap)
{
    const uSize
        width { rendered.width },
        height { rendered.rows };
    bitmap->_Width = rendered.width;
    bitmap->_Height = rendered.rows;

    switch(rendered.pixel_mode)
    {
        case FT_PIXEL_MODE_GRAY:
        {
            bitmap->_Format = GlyphPixelFormat::Coverage;
            bitmap->_Pixels.resize(width * height);

            //FreeType's rows may be padded, so they're copied tight
            for(uSize row = 0; row < height; ++row)
                std::copy_n(rendered.buffer + static_cast<ptrdiff_t>(row) * rendered.pitch, width, bitmap->_Pixels.data() + row * width);

            return true;
        }
        case FT_PIXEL_MODE_MONO: //a bitmap strike drawn one bit per pixel, the leftmost pixel in the highest bit
        {
            bitmap->_Format = GlyphPixelFormat::Coverage;
            bitmap->_Pixels.resize(width * height);

            for(uSize row = 0; row < height; ++row)
                for(uSize column = 0; column < width; ++column)
                    bitmap->_Pixels[row * width + column] = (rendered.buffer[static_cast<ptrdiff_t>(row) * rendered.pitch + column / 8] & (0x80 >> (column % 8))) ? 255 : 0;

            return true;
        }
        case FT_PIXEL_MODE_BGRA:
        {
            bitmap->_Format = GlyphPixelFormat::Color;
            bitmap->_Pixels.resize(width * height * 4);

            //Blue and red swapped. the colors stay premultiplied, as FreeType blends the layers
            for(uSize row = 0; row < height; ++row)
            {
                for(uSize column = 0; column < width; ++column)
                {
                    const u8* source { rendered.buffer + static_cast<ptrdiff_t>(row) * rendered.pitch + column * 4 };
                    u8* target { bitmap->_Pixels.data() + (row * width + column) * 4 };
                    target[0] = source[2];
                    target[1] = source[1];
                    target[2] = source[0];
                    target[3] = source[3];
                }
            }

            return true;
        }
        default:
            return false;
    }
}