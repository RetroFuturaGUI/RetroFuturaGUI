#include "ColorGlyphPainter.hpp"
#include FT_COLOR_H
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <utility>

bool RetroFuturaGUI::ColorGlyphPainter::HasPaint(FT_Face face, const u32 glyphIndex)
{
    if(!face)
        return false;

    //FreeType reads the COLR table as glyphs are asked for, where HarfBuzz checks all of it first (megabytes for Segoe UI Emoji)
    FT_OpaquePaint paint { .p = nullptr, .insert_root_transform = 0 };
    return FT_Get_Color_Glyph_Paint(face, glyphIndex, FT_COLOR_NO_ROOT_TRANSFORM, &paint);
}

bool RetroFuturaGUI::ColorGlyphPainter::Paint(FT_Face face, hb_font_t* font, const u32 glyphIndex, GlyphBitmap* bitmap)
{
    if(!bitmap)
        return false;

    if(!face)
        return false;

    if(!font)
        return false;

    if(!face->size)
        return false;

    //The bounds from the font's clip box, or else from every outline the paint clips to, or else the em square
    ColorGlyphPainter painter { face, true };
    FT_ClipBox clipBox {};

    if(FT_Get_Color_Glyph_ClipBox(face, glyphIndex, &clipBox))
    {
        const std::array<FT_Vector, 4> corners { clipBox.bottom_left, clipBox.top_left, clipBox.top_right, clipBox.bottom_right };
        painter._lowest = glm::vec2(corners[0].x, corners[0].y);
        painter._highest = painter._lowest;

        for(const FT_Vector& corner : corners)
        {
            painter._lowest = glm::min(painter._lowest, glm::vec2(corner.x, corner.y));
            painter._highest = glm::max(painter._highest, glm::vec2(corner.x, corner.y));
        }

        painter._hasBounds = true;
    }
    else
    {
        painter._transforms = { glm::mat3(1.0f) };
        hb_font_paint_glyph(font, glyphIndex, getPaintFuncs(), &painter, 0, HB_COLOR(0, 0, 0, 255));
    }

    if(!painter._hasBounds)
    {
        const FT_Size_Metrics& metrics { face->size->metrics };
        painter._lowest = glm::vec2(0.0f, metrics.descender);
        painter._highest = glm::vec2(metrics.x_ppem * 64.0f, metrics.ascender);
    }

    painter._isMeasuring = false;

    if(!painter.setBounds(painter._lowest, painter._highest))
        return false;

    //A foreground color is the text's to give; black is what the font is drawn against when it has none
    hb_font_paint_glyph(font, glyphIndex, getPaintFuncs(), &painter, 0, HB_COLOR(0, 0, 0, 255));

    if(painter._groups.empty())
        return false;

    const std::vector<glm::vec4>& glyph { painter._groups.front()._Colors };
    *bitmap = GlyphBitmap
    {
        ._Pixels = std::vector<u8>(glyph.size() * 4),
        ._Width = painter._width,
        ._Height = painter._height,
        ._Advance = 0,
        ._BearingX = painter._left,
        ._BearingY = painter._top,
        ._Format = GlyphPixelFormat::Color
    };

    const f32* channels { &glyph.data()->x };
    u8* pixels { bitmap->_Pixels.data() };

    for(uSize channel = 0; channel < glyph.size() * 4; ++channel)
        pixels[channel] = static_cast<u8>(std::clamp(channels[channel], 0.0f, 1.0f) * 255.0f + 0.5f);

    return true;
}

RetroFuturaGUI::ColorGlyphPainter::ColorGlyphPainter(FT_Face face, const bool isMeasuring)
    : _face(face), _isMeasuring(isMeasuring)
{
}

hb_paint_funcs_t* RetroFuturaGUI::ColorGlyphPainter::getPaintFuncs()
{
    //Made once and never changed, so every painting shares it; it lives as long as the program
    static hb_paint_funcs_t* const funcs
    {
        []()
        {
            hb_paint_funcs_t* created { hb_paint_funcs_create() };
            hb_paint_funcs_set_push_transform_func(created, pushTransform, nullptr, nullptr);
            hb_paint_funcs_set_pop_transform_func(created, popTransform, nullptr, nullptr);
            hb_paint_funcs_set_push_clip_glyph_func(created, pushClipGlyph, nullptr, nullptr);
            hb_paint_funcs_set_push_clip_rectangle_func(created, pushClipRectangle, nullptr, nullptr);
            hb_paint_funcs_set_pop_clip_func(created, popClip, nullptr, nullptr);
            hb_paint_funcs_set_color_func(created, paintColor, nullptr, nullptr);
            hb_paint_funcs_set_linear_gradient_func(created, paintLinearGradient, nullptr, nullptr);
            hb_paint_funcs_set_radial_gradient_func(created, paintRadialGradient, nullptr, nullptr);
            hb_paint_funcs_set_sweep_gradient_func(created, paintSweepGradient, nullptr, nullptr);
            hb_paint_funcs_set_push_group_func(created, pushGroup, nullptr, nullptr);
            hb_paint_funcs_set_pop_group_func(created, popGroup, nullptr, nullptr);
            hb_paint_funcs_make_immutable(created);
            return created;
        }()
    };

    return funcs;
}

void RetroFuturaGUI::ColorGlyphPainter::pushTransform(hb_paint_funcs_t*, void* data, float xx, float yx, float xy, float yy, float dx, float dy, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    //x' = xx x + xy y + dx, y' = yx x + yy y + dy, in glm's columns
    painter->_transforms.push_back(painter->_transforms.back() * glm::mat3(xx, yx, 0.0f, xy, yy, 0.0f, dx, dy, 1.0f));
}

void RetroFuturaGUI::ColorGlyphPainter::popTransform(hb_paint_funcs_t*, void* data, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    if(1 < painter->_transforms.size())
        painter->_transforms.pop_back();
}

void RetroFuturaGUI::ColorGlyphPainter::pushClipGlyph(hb_paint_funcs_t*, void* data, hb_codepoint_t glyph, hb_font_t*, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    //A glyph without an outline clips everything away; the clip is pushed all the same, as its pop will come
    FT_Face face { painter->_face };
    const bool hasOutline { 0 == FT_Load_Glyph(face, glyph, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING) && FT_GLYPH_FORMAT_OUTLINE == face->glyph->format };
    FT_Outline outline {};

    if(!hasOutline || FT_Outline_New(face->glyph->library, face->glyph->outline.n_points, face->glyph->outline.n_contours, &outline))
    {
        if(!painter->_isMeasuring)
            painter->_clips.push_back(Clip {});

        return;
    }

    //The outline at the face's size, unhinted, taken where the current transform puts it
    FT_Outline_Copy(&face->glyph->outline, &outline);
    const glm::mat3& transform { painter->_transforms.back() };
    FT_Matrix matrix
    {
        .xx = static_cast<FT_Fixed>(std::lround(transform[0][0] * 65536.0f)), //16.16 fixed point
        .xy = static_cast<FT_Fixed>(std::lround(transform[1][0] * 65536.0f)),
        .yx = static_cast<FT_Fixed>(std::lround(transform[0][1] * 65536.0f)),
        .yy = static_cast<FT_Fixed>(std::lround(transform[1][1] * 65536.0f))
    };
    FT_Outline_Transform(&outline, &matrix);
    FT_Outline_Translate(&outline, static_cast<FT_Pos>(std::lround(transform[2][0])), static_cast<FT_Pos>(std::lround(transform[2][1])));

    if(painter->_isMeasuring)
    {
        FT_BBox box {};
        FT_Outline_Get_CBox(&outline, &box);
        const glm::vec2
            lowest { box.xMin, box.yMin },
            highest { box.xMax, box.yMax };
        painter->_lowest = painter->_hasBounds ? glm::min(painter->_lowest, lowest) : lowest;
        painter->_highest = painter->_hasBounds ? glm::max(painter->_highest, highest) : highest;
        painter->_hasBounds = true;
    }
    else
        painter->pushClipOutline(&outline);

    FT_Outline_Done(face->glyph->library, &outline);
}

void RetroFuturaGUI::ColorGlyphPainter::pushClipRectangle(hb_paint_funcs_t*, void* data, float xMin, float yMin, float xMax, float yMax, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    //The rectangle's corners where the current transform puts them, as an outline of one contour
    const glm::mat3& transform { painter->_transforms.back() };
    std::array<FT_Vector, 4> points {};
    const std::array<glm::vec2, 4> corners { glm::vec2(xMin, yMin), glm::vec2(xMax, yMin), glm::vec2(xMax, yMax), glm::vec2(xMin, yMax) };

    for(uSize corner = 0; corner < corners.size(); ++corner)
    {
        const glm::vec3 placed { transform * glm::vec3(corners[corner], 1.0f) };
        points[corner] = FT_Vector { .x = static_cast<FT_Pos>(std::lround(placed.x)), .y = static_cast<FT_Pos>(std::lround(placed.y)) };
    }

    if(painter->_isMeasuring)
    {
        for(const FT_Vector& point : points)
        {
            const glm::vec2 corner { point.x, point.y };
            painter->_lowest = painter->_hasBounds ? glm::min(painter->_lowest, corner) : corner;
            painter->_highest = painter->_hasBounds ? glm::max(painter->_highest, corner) : corner;
            painter->_hasBounds = true;
        }

        return;
    }

    std::array<unsigned char, 4> tags { FT_CURVE_TAG_ON, FT_CURVE_TAG_ON, FT_CURVE_TAG_ON, FT_CURVE_TAG_ON };
    std::array<unsigned short, 1> contours { 3 }; //the last point of the only contour
    FT_Outline outline
    {
        .n_contours = 1,
        .n_points = 4,
        .points = points.data(),
        .tags = tags.data(),
        .contours = contours.data(),
        .flags = FT_OUTLINE_NONE
    };
    painter->pushClipOutline(&outline);
}

void RetroFuturaGUI::ColorGlyphPainter::popClip(hb_paint_funcs_t*, void* data, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    if(painter->_isMeasuring || painter->_clips.empty())
        return;

    painter->_clips.pop_back();
}

void RetroFuturaGUI::ColorGlyphPainter::paintColor(hb_paint_funcs_t*, void* data, hb_bool_t, hb_color_t color, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    const glm::vec4 premultiplied { toPremultiplied(color) };
    const f32 solid[4] { premultiplied.r, premultiplied.g, premultiplied.b, premultiplied.a };
    painter->fill([&solid](const f32, const f32, f32* pixel)
    {
        pixel[0] = solid[0];
        pixel[1] = solid[1];
        pixel[2] = solid[2];
        pixel[3] = solid[3];
    });
}

void RetroFuturaGUI::ColorGlyphPainter::paintLinearGradient(hb_paint_funcs_t*, void* data, hb_color_line_t* colorLine, float x0, float y0, float x1, float y1, float x2, float y2, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    //The colors run from p0 toward p1 but stay the same along lines parallel to p0-p2, so they run along the perpendicular
    //to p0-p2: p3 is p1 moved onto it
    const glm::vec2
        start { x0, y0 },
        toEnd { glm::vec2(x1, y1) - start },
        toSide { glm::vec2(x2, y2) - start };
    const f32 sideLengthSquared { glm::dot(toSide, toSide) };
    const glm::vec2 direction { 0.0f == sideLengthSquared ? toEnd : toEnd - toSide * (glm::dot(toEnd, toSide) / sideLengthSquared) };
    const f32 directionLengthSquared { glm::dot(direction, direction) };

    if(0.0f == directionLengthSquared)
        return;

    //t is the point's distance along the direction, a fraction of its length
    const ColorLine line { readColorLine(colorLine) };
    const f32
        stepX { direction.x / directionLengthSquared },
        stepY { direction.y / directionLengthSquared };
    painter->fill([&](const f32 x, const f32 y, f32* pixel) { colorAt(line, (x - x0) * stepX + (y - y0) * stepY, pixel); });
}

void RetroFuturaGUI::ColorGlyphPainter::paintRadialGradient(hb_paint_funcs_t*, void* data, hb_color_line_t* colorLine, float x0, float y0, float r0, float x1, float y1, float r1, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    const ColorLine line { readColorLine(colorLine) };
    painter->fill([&](const f32 x, const f32 y, f32* pixel)
    {
        f32 t { 0.0f };

        if(getRadialPosition(x, y, x0, y0, r0, x1, y1, r1, &t))
        {
            colorAt(line, t, pixel);
            return;
        }

        pixel[0] = pixel[1] = pixel[2] = pixel[3] = 0.0f;
    });
}

void RetroFuturaGUI::ColorGlyphPainter::paintSweepGradient(hb_paint_funcs_t*, void* data, hb_color_line_t* colorLine, float x0, float y0, float startAngle, float endAngle, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    const f32 sweep { endAngle - startAngle };

    if(0.0f == sweep)
        return;

    //Angles counter-clockwise from the positive x axis, y up, as the font's own space runs
    const ColorLine line { readColorLine(colorLine) };
    painter->fill([&](const f32 x, const f32 y, f32* pixel)
    {
        f32 angle { std::atan2(y - y0, x - x0) };

        if(0.0f > angle)
            angle += 2.0f * std::numbers::pi_v<f32>;

        colorAt(line, (angle - startAngle) / sweep, pixel);
    });
}

void RetroFuturaGUI::ColorGlyphPainter::pushGroup(hb_paint_funcs_t*, void* data, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    if(painter->_isMeasuring)
        return;

    if(painter->_spareGroups.empty())
    {
        painter->_groups.push_back(Group { ._Colors = std::vector<glm::vec4>(static_cast<uSize>(painter->_width) * painter->_height, glm::vec4(0.0f)) });
        return;
    }

    painter->_groups.push_back(std::move(painter->_spareGroups.back()));
    painter->_spareGroups.pop_back();
}

void RetroFuturaGUI::ColorGlyphPainter::popGroup(hb_paint_funcs_t*, void* data, hb_paint_composite_mode_t mode, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    if(painter->_isMeasuring || 2 > painter->_groups.size())
        return;

    //Laid onto the group beneath inside the current clip only, so a mode like Clear doesn't wipe what lies outside it.
    //Most modes leave the backdrop alone where the group drew nothing, and only need the rectangle it drew into
    Group source { std::move(painter->_groups.back()) };
    painter->_groups.pop_back();
    Group& backdrop { painter->_groups.back() };
    Clip area { painter->getCurrentArea() };

    if(keepsUntouchedBackdrop(mode))
    {
        area._Left = std::max(area._Left, source._Left);
        area._Top = std::max(area._Top, source._Top);
        area._Right = std::min(area._Right, source._Right);
        area._Bottom = std::min(area._Bottom, source._Bottom);
    }

    //Plain floats over the four of each pixel, as the fill does
    f32* sourceColors { &source._Colors.data()->x };
    f32* backdropColors { &backdrop._Colors.data()->x };
    const Clip* clip { painter->_clips.empty() ? nullptr : &painter->_clips.back() };

    for(u32 row = area._Top; row < area._Bottom; ++row)
    {
        const f32* coverageRow { clip != nullptr ? clip->_Coverage.data() + static_cast<uSize>(row - clip->_Top) * (clip->_Right - clip->_Left) : nullptr };

        for(u32 column = area._Left; column < area._Right; ++column)
        {
            const f32 coverage { coverageRow != nullptr ? coverageRow[column - clip->_Left] : 1.0f };

            if(0.0f >= coverage)
                continue;

            const uSize pixel { (static_cast<uSize>(row) * painter->_width + column) * 4 };
            const f32* sourceColor { sourceColors + pixel };
            f32* backdropColor { backdropColors + pixel };

            //Every layer is a group laid over the ones before, so the plain case goes straight; the coverage scales the
            //source, as laying it over a mixed backdrop comes to the same
            if(HB_PAINT_COMPOSITE_MODE_SRC_OVER == mode)
            {
                const f32 keep { 1.0f - sourceColor[3] * coverage };

                for(uSize channel = 0; channel < 4; ++channel)
                    backdropColor[channel] = sourceColor[channel] * coverage + backdropColor[channel] * keep;

                continue;
            }

            const glm::vec4
                under { backdropColor[0], backdropColor[1], backdropColor[2], backdropColor[3] },
                laid { glm::mix(under, composite(glm::vec4(sourceColor[0], sourceColor[1], sourceColor[2], sourceColor[3]), under, mode), coverage) };

            for(glm::length_t channel = 0; channel < 4; ++channel)
                backdropColor[channel] = laid[channel];
        }
    }

    if(area._Left < area._Right && area._Top < area._Bottom)
        addDrawnArea(&backdrop, area);

    //Cleared where it drew, ready for the next push
    for(u32 row = source._Top; row < source._Bottom; ++row)
        std::fill_n(sourceColors + (static_cast<uSize>(row) * painter->_width + source._Left) * 4, static_cast<uSize>(source._Right - source._Left) * 4, 0.0f);

    source._Left = source._Top = source._Right = source._Bottom = 0;
    painter->_spareGroups.push_back(std::move(source));
}

bool RetroFuturaGUI::ColorGlyphPainter::setBounds(const glm::vec2& lowest, const glm::vec2& highest)
{
    const i32
        left { static_cast<i32>(std::floor(lowest.x / 64.0f)) - 1 },
        right { static_cast<i32>(std::ceil(highest.x / 64.0f)) + 1 },
        bottom { static_cast<i32>(std::floor(lowest.y / 64.0f)) - 1 },
        top { static_cast<i32>(std::ceil(highest.y / 64.0f)) + 1 };

    if(left >= right || bottom >= top)
        return false;

    if(static_cast<i32>(_maxSide) < right - left || static_cast<i32>(_maxSide) < top - bottom)
        return false;

    _left = left;
    _top = top;
    _width = static_cast<u32>(right - left);
    _height = static_cast<u32>(top - bottom);
    _transforms = { glm::mat3(1.0f) };
    _clips.clear();
    _groups = { Group { ._Colors = std::vector<glm::vec4>(static_cast<uSize>(_width) * _height, glm::vec4(0.0f)) } };
    _spareGroups.clear();
    return true;
}

void RetroFuturaGUI::ColorGlyphPainter::pushClipOutline(FT_Outline* outline)
{
    if(!outline)
        return;

    //The outline's box in the bitmap's pixels, rows from the top, cut to the clip around it; only that much is filled
    FT_BBox box {};
    FT_Outline_Get_CBox(outline, &box);
    const Clip outer { getCurrentArea() };
    Clip clip
    {
        ._Left = static_cast<u32>(std::clamp<i64>(static_cast<i64>(std::floor(box.xMin / 64.0)) - _left, outer._Left, outer._Right)),
        ._Top = static_cast<u32>(std::clamp<i64>(_top - static_cast<i64>(std::ceil(box.yMax / 64.0)), outer._Top, outer._Bottom)),
        ._Right = static_cast<u32>(std::clamp<i64>(static_cast<i64>(std::ceil(box.xMax / 64.0)) - _left, outer._Left, outer._Right)),
        ._Bottom = static_cast<u32>(std::clamp<i64>(_top - static_cast<i64>(std::floor(box.yMin / 64.0)), outer._Top, outer._Bottom)),
        ._Coverage = {}
    };

    if(clip._Left >= clip._Right || clip._Top >= clip._Bottom)
    {
        _clips.push_back(Clip {});
        return;
    }

    //FreeType fills from the target's bottom left corner up, its rows stored from the top
    const u32
        clipWidth { clip._Right - clip._Left },
        clipHeight { clip._Bottom - clip._Top };
    FT_Outline_Translate(outline, -static_cast<FT_Pos>(_left + static_cast<i32>(clip._Left)) * 64, -static_cast<FT_Pos>(_top - static_cast<i32>(clip._Bottom)) * 64);
    std::vector<u8> coverage(static_cast<uSize>(clipWidth) * clipHeight, 0);
    FT_Bitmap target
    {
        .rows = clipHeight,
        .width = clipWidth,
        .pitch = static_cast<int>(clipWidth),
        .buffer = coverage.data(),
        .num_grays = 256,
        .pixel_mode = static_cast<unsigned char>(FT_PIXEL_MODE_GRAY),
        .palette_mode = 0,
        .palette = nullptr
    };
    FT_Outline_Get_Bitmap(_face->glyph->library, outline, &target);
    clip._Coverage.resize(coverage.size());

    //Cut by the clip around it, which covers this rectangle (it was cut to it above), row by row
    const Clip* outerClip { _clips.empty() ? nullptr : &_clips.back() };
    const u8* filled { coverage.data() };
    f32* combined { clip._Coverage.data() };

    for(u32 row = 0; row < clipHeight; ++row)
    {
        const f32* outerRow { outerClip != nullptr ? outerClip->_Coverage.data() + static_cast<uSize>(clip._Top + row - outerClip->_Top) * (outerClip->_Right - outerClip->_Left) + (clip._Left - outerClip->_Left) : nullptr };

        for(u32 column = 0; column < clipWidth; ++column, ++filled, ++combined)
            *combined = *filled / 255.0f * (outerRow != nullptr ? outerRow[column] : 1.0f);
    }

    _clips.push_back(std::move(clip));
}

RetroFuturaGUI::ColorGlyphPainter::Clip RetroFuturaGUI::ColorGlyphPainter::getCurrentArea() const
{
    if(_clips.empty())
        return Clip { ._Left = 0, ._Top = 0, ._Right = _width, ._Bottom = _height, ._Coverage = {} };

    const Clip& clip { _clips.back() };
    return Clip { ._Left = clip._Left, ._Top = clip._Top, ._Right = clip._Right, ._Bottom = clip._Bottom, ._Coverage = {} };
}

template<typename ColorAt>
void RetroFuturaGUI::ColorGlyphPainter::fill(const ColorAt& colorAt)
{
    if(_isMeasuring || _groups.empty())
        return;

    //A transform squashing everything flat draws nothing
    const glm::mat3& transform { _transforms.back() };

    if(0.0f == glm::determinant(transform))
        return;

    const Clip area { getCurrentArea() };

    if(area._Left >= area._Right || area._Top >= area._Bottom)
        return;

    addDrawnArea(&_groups.back(), area);

    //From the bitmap back into the paint's own space, x' = xx x + xy y + dx and y' = yx x + yy y + dy, so a pixel further
    //right is a fixed step further
    const glm::mat3 inverse { glm::inverse(transform) };
    const f32
        xx { inverse[0][0] },
        yx { inverse[0][1] },
        xy { inverse[1][0] },
        yy { inverse[1][1] },
        dx { inverse[2][0] },
        dy { inverse[2][1] },
        stepX { xx * 64.0f },
        stepY { yx * 64.0f };
    f32* colors { &_groups.back()._Colors.data()->x }; //each pixel's four floats in a row, as glm::vec4 keeps them
    const Clip* clip { _clips.empty() ? nullptr : &_clips.back() };
    const u32 areaWidth { area._Right - area._Left };
    f32 color[4] {};

    for(u32 row = area._Top; row < area._Bottom; ++row)
    {
        //The row's first pixel center in 1/64 pixels from the glyph's origin, y up
        const f32
            centerX { (static_cast<f32>(_left) + area._Left + 0.5f) * 64.0f },
            centerY { (static_cast<f32>(_top) - row - 0.5f) * 64.0f };
        f32
            x { xx * centerX + xy * centerY + dx },
            y { yx * centerX + yy * centerY + dy };
        const f32* coverageRow { clip != nullptr ? clip->_Coverage.data() + static_cast<uSize>(row - clip->_Top) * areaWidth : nullptr };
        f32* target { colors + (static_cast<uSize>(row) * _width + area._Left) * 4 };

        for(u32 column = 0; column < areaWidth; ++column, x += stepX, y += stepY, target += 4)
        {
            const f32 coverage { coverageRow != nullptr ? coverageRow[column] : 1.0f };

            if(0.0f >= coverage)
                continue;

            colorAt(x, y, color);
            const f32 keep { 1.0f - color[3] * coverage };
            target[0] = color[0] * coverage + target[0] * keep;
            target[1] = color[1] * coverage + target[1] * keep;
            target[2] = color[2] * coverage + target[2] * keep;
            target[3] = color[3] * coverage + target[3] * keep;
        }
    }
}

void RetroFuturaGUI::ColorGlyphPainter::addDrawnArea(Group* group, const Clip& area)
{
    if(!group)
        return;

    if(group->_Left >= group->_Right)
    {
        group->_Left = area._Left;
        group->_Top = area._Top;
        group->_Right = area._Right;
        group->_Bottom = area._Bottom;
        return;
    }

    group->_Left = std::min(group->_Left, area._Left);
    group->_Top = std::min(group->_Top, area._Top);
    group->_Right = std::max(group->_Right, area._Right);
    group->_Bottom = std::max(group->_Bottom, area._Bottom);
}

bool RetroFuturaGUI::ColorGlyphPainter::keepsUntouchedBackdrop(const hb_paint_composite_mode_t mode)
{
    //Where the source is transparent these clear the backdrop or replace it
    switch(mode)
    {
        case HB_PAINT_COMPOSITE_MODE_CLEAR:
        case HB_PAINT_COMPOSITE_MODE_SRC:
        case HB_PAINT_COMPOSITE_MODE_SRC_IN:
        case HB_PAINT_COMPOSITE_MODE_DEST_IN:
        case HB_PAINT_COMPOSITE_MODE_SRC_OUT:
        case HB_PAINT_COMPOSITE_MODE_DEST_ATOP:
            return false;
        default:
            return true;
    }
}

RetroFuturaGUI::ColorGlyphPainter::ColorLine RetroFuturaGUI::ColorGlyphPainter::readColorLine(hb_color_line_t* colorLine)
{
    ColorLine line {};

    if(!colorLine)
        return line;

    uint32_t stopCount { hb_color_line_get_color_stops(colorLine, 0, nullptr, nullptr) };
    std::vector<hb_color_stop_t> stops(stopCount);
    hb_color_line_get_color_stops(colorLine, 0, &stopCount, stops.data());

    //In order of their offsets, a foreground stop coming with the foreground's color
    std::vector<std::pair<f32, glm::vec4>> sorted {};

    for(uint32_t stop = 0; stop < stopCount; ++stop)
        sorted.emplace_back(stops[stop].offset, toPremultiplied(stops[stop].color));

    std::ranges::stable_sort(sorted, {}, &std::pair<f32, glm::vec4>::first);
    line._Extend = hb_color_line_get_extend(colorLine);

    for(const auto& [offset, color] : sorted)
    {
        line._Offsets.push_back(offset);
        line._Colors.insert(line._Colors.end(), { color.r, color.g, color.b, color.a });
    }

    if(sorted.empty())
        return line;

    line._First = sorted.front().first;
    line._Range = sorted.back().first - line._First;
    return line;
}

void RetroFuturaGUI::ColorGlyphPainter::colorAt(const ColorLine& line, const f32 t, f32* color)
{
    const uSize stopCount { line._Offsets.size() };

    if(0 == stopCount)
    {
        color[0] = color[1] = color[2] = color[3] = 0.0f;
        return;
    }

    const f32
        *offsets { line._Offsets.data() },
        *colors { line._Colors.data() };

    //Stops all at one offset: the first color before it, the last from it on
    if(0.0f >= line._Range)
    {
        const f32* stopColor { colors + (t < line._First ? 0 : (stopCount - 1) * 4) };
        color[0] = stopColor[0];
        color[1] = stopColor[1];
        color[2] = stopColor[2];
        color[3] = stopColor[3];
        return;
    }

    //From the first stop (0) to the last (1); past both ends the stops hold their end colors, repeat, or repeat mirrored
    f32 position { (t - line._First) / line._Range };

    switch(line._Extend)
    {
        case HB_PAINT_EXTEND_REPEAT:
            position -= std::floor(position);
            break;
        case HB_PAINT_EXTEND_REFLECT:
            position -= 2.0f * std::floor(position * 0.5f);
            position = 1.0f < position ? 2.0f - position : position;
            break;
        default:
            position = 0.0f > position ? 0.0f : 1.0f < position ? 1.0f : position;
            break;
    }

    //The pair of stops around it, the color mixed between them
    const f32 offset { line._First + position * line._Range };
    uSize after { 1 };

    while(after + 1 < stopCount && offset > offsets[after])
        ++after;

    const f32 width { offsets[after] - offsets[after - 1] };
    const f32 mix { 0.0f >= width ? 1.0f : std::clamp((offset - offsets[after - 1]) / width, 0.0f, 1.0f) };
    const f32
        *from { colors + (after - 1) * 4 },
        *to { colors + after * 4 };
    color[0] = from[0] + (to[0] - from[0]) * mix;
    color[1] = from[1] + (to[1] - from[1]) * mix;
    color[2] = from[2] + (to[2] - from[2]) * mix;
    color[3] = from[3] + (to[3] - from[3]) * mix;
}

glm::vec4 RetroFuturaGUI::ColorGlyphPainter::toPremultiplied(const hb_color_t color)
{
    const f32 alpha { hb_color_get_alpha(color) / 255.0f };
    return glm::vec4(hb_color_get_red(color) / 255.0f * alpha, hb_color_get_green(color) / 255.0f * alpha, hb_color_get_blue(color) / 255.0f * alpha, alpha);
}

bool RetroFuturaGUI::ColorGlyphPainter::getRadialPosition(const f32 x, const f32 y, const f32 x0, const f32 y0, const f32 radius0, const f32 x1, const f32 y1, const f32 radius1, f32* t)
{
    if(!t)
        return false;

    //The circle at t has its center at center0 + t (center1 - center0) and the radius radius0 + t (radius1 - radius0);
    //passing through the point makes a quadratic in t
    const f32
        centerStepX { x1 - x0 },
        centerStepY { y1 - y0 },
        fromCenterX { x - x0 },
        fromCenterY { y - y0 },
        radiusStep { radius1 - radius0 },
        a { centerStepX * centerStepX + centerStepY * centerStepY - radiusStep * radiusStep },
        b { fromCenterX * centerStepX + fromCenterY * centerStepY + radius0 * radiusStep },
        c { fromCenterX * fromCenterX + fromCenterY * fromCenterY - radius0 * radius0 };

    if(1e-6f > std::abs(a))
    {
        if(0.0f == b)
            return false;

        *t = c / (2.0f * b);
        return 0.0f <= radius0 + *t * radiusStep;
    }

    const f32 discriminant { b * b - a * c };

    if(0.0f > discriminant)
        return false;

    const f32
        root { std::sqrt(discriminant) },
        larger { std::max((b + root) / a, (b - root) / a) },
        smaller { std::min((b + root) / a, (b - root) / a) };

    //The larger t wins, as its circle is drawn over the smaller one's, as long as its radius isn't negative
    if(0.0f <= radius0 + larger * radiusStep)
    {
        *t = larger;
        return true;
    }

    if(0.0f <= radius0 + smaller * radiusStep)
    {
        *t = smaller;
        return true;
    }

    return false;
}

glm::vec4 RetroFuturaGUI::ColorGlyphPainter::composite(const glm::vec4& source, const glm::vec4& backdrop, const hb_paint_composite_mode_t mode)
{
    const f32
        sourceAlpha { source.a },
        backdropAlpha { backdrop.a };

    //Porter and Duff's operators on premultiplied colors, then the blend modes
    switch(mode)
    {
        case HB_PAINT_COMPOSITE_MODE_CLEAR:
            return glm::vec4(0.0f);
        case HB_PAINT_COMPOSITE_MODE_SRC:
            return source;
        case HB_PAINT_COMPOSITE_MODE_DEST:
            return backdrop;
        case HB_PAINT_COMPOSITE_MODE_SRC_OVER:
            return source + backdrop * (1.0f - sourceAlpha);
        case HB_PAINT_COMPOSITE_MODE_DEST_OVER:
            return backdrop + source * (1.0f - backdropAlpha);
        case HB_PAINT_COMPOSITE_MODE_SRC_IN:
            return source * backdropAlpha;
        case HB_PAINT_COMPOSITE_MODE_DEST_IN:
            return backdrop * sourceAlpha;
        case HB_PAINT_COMPOSITE_MODE_SRC_OUT:
            return source * (1.0f - backdropAlpha);
        case HB_PAINT_COMPOSITE_MODE_DEST_OUT:
            return backdrop * (1.0f - sourceAlpha);
        case HB_PAINT_COMPOSITE_MODE_SRC_ATOP:
            return source * backdropAlpha + backdrop * (1.0f - sourceAlpha);
        case HB_PAINT_COMPOSITE_MODE_DEST_ATOP:
            return backdrop * sourceAlpha + source * (1.0f - backdropAlpha);
        case HB_PAINT_COMPOSITE_MODE_XOR:
            return source * (1.0f - backdropAlpha) + backdrop * (1.0f - sourceAlpha);
        case HB_PAINT_COMPOSITE_MODE_PLUS:
            return glm::min(source + backdrop, glm::vec4(1.0f));
        default:
            break;
    }

    //The blend mode mixes the colors themselves, not premultiplied, where both are; elsewhere each shows as it is
    const glm::vec3
        sourceColor { 0.0f < sourceAlpha ? glm::vec3(source) / sourceAlpha : glm::vec3(0.0f) },
        backdropColor { 0.0f < backdropAlpha ? glm::vec3(backdrop) / backdropAlpha : glm::vec3(0.0f) };
    glm::vec3 blended { 0.0f };

    switch(mode)
    {
        case HB_PAINT_COMPOSITE_MODE_HSL_HUE:
        case HB_PAINT_COMPOSITE_MODE_HSL_SATURATION:
        case HB_PAINT_COMPOSITE_MODE_HSL_COLOR:
        case HB_PAINT_COMPOSITE_MODE_HSL_LUMINOSITY:
            blended = blendNonSeparable(sourceColor, backdropColor, mode);
            break;
        default:
            for(glm::length_t channel = 0; channel < 3; ++channel)
                blended[channel] = blendChannel(sourceColor[channel], backdropColor[channel], mode);
            break;
    }

    const glm::vec3 color { glm::vec3(source) * (1.0f - backdropAlpha) + glm::vec3(backdrop) * (1.0f - sourceAlpha) + sourceAlpha * backdropAlpha * blended };
    return glm::vec4(color, sourceAlpha + backdropAlpha - sourceAlpha * backdropAlpha);
}

f32 RetroFuturaGUI::ColorGlyphPainter::blendChannel(const f32 source, const f32 backdrop, const hb_paint_composite_mode_t mode)
{
    switch(mode)
    {
        case HB_PAINT_COMPOSITE_MODE_MULTIPLY:
            return source * backdrop;
        case HB_PAINT_COMPOSITE_MODE_SCREEN:
            return source + backdrop - source * backdrop;
        case HB_PAINT_COMPOSITE_MODE_OVERLAY: //hard light with the two swapped
            return blendChannel(backdrop, source, HB_PAINT_COMPOSITE_MODE_HARD_LIGHT);
        case HB_PAINT_COMPOSITE_MODE_DARKEN:
            return std::min(source, backdrop);
        case HB_PAINT_COMPOSITE_MODE_LIGHTEN:
            return std::max(source, backdrop);
        case HB_PAINT_COMPOSITE_MODE_COLOR_DODGE:
            if(0.0f >= backdrop)
                return 0.0f;

            return 1.0f <= source ? 1.0f : std::min(1.0f, backdrop / (1.0f - source));
        case HB_PAINT_COMPOSITE_MODE_COLOR_BURN:
            if(1.0f <= backdrop)
                return 1.0f;

            return 0.0f >= source ? 0.0f : 1.0f - std::min(1.0f, (1.0f - backdrop) / source);
        case HB_PAINT_COMPOSITE_MODE_HARD_LIGHT:
            return 0.5f >= source ? backdrop * 2.0f * source : blendChannel(2.0f * source - 1.0f, backdrop, HB_PAINT_COMPOSITE_MODE_SCREEN);
        case HB_PAINT_COMPOSITE_MODE_SOFT_LIGHT:
        {
            if(0.5f >= source)
                return backdrop - (1.0f - 2.0f * source) * backdrop * (1.0f - backdrop);

            const f32 lifted { 0.25f >= backdrop ? ((16.0f * backdrop - 12.0f) * backdrop + 4.0f) * backdrop : std::sqrt(backdrop) };
            return backdrop + (2.0f * source - 1.0f) * (lifted - backdrop);
        }
        case HB_PAINT_COMPOSITE_MODE_DIFFERENCE:
            return std::abs(source - backdrop);
        case HB_PAINT_COMPOSITE_MODE_EXCLUSION:
            return source + backdrop - 2.0f * source * backdrop;
        default:
            return source;
    }
}

glm::vec3 RetroFuturaGUI::ColorGlyphPainter::blendNonSeparable(const glm::vec3& source, const glm::vec3& backdrop, const hb_paint_composite_mode_t mode)
{
    switch(mode)
    {
        case HB_PAINT_COMPOSITE_MODE_HSL_HUE:
            return setLuminosity(setSaturation(source, getSaturation(backdrop)), getLuminosity(backdrop));
        case HB_PAINT_COMPOSITE_MODE_HSL_SATURATION:
            return setLuminosity(setSaturation(backdrop, getSaturation(source)), getLuminosity(backdrop));
        case HB_PAINT_COMPOSITE_MODE_HSL_COLOR:
            return setLuminosity(source, getLuminosity(backdrop));
        default: //luminosity
            return setLuminosity(backdrop, getLuminosity(source));
    }
}

f32 RetroFuturaGUI::ColorGlyphPainter::getLuminosity(const glm::vec3& color)
{
    return 0.3f * color.r + 0.59f * color.g + 0.11f * color.b;
}

glm::vec3 RetroFuturaGUI::ColorGlyphPainter::setLuminosity(const glm::vec3& color, const f32 luminosity)
{
    //Shifted to the luminosity, then pulled back toward it where a channel left the range from 0 to 1
    glm::vec3 shifted { color + (luminosity - getLuminosity(color)) };
    const f32
        shiftedLuminosity { getLuminosity(shifted) },
        lowest { std::min({ shifted.r, shifted.g, shifted.b }) },
        highest { std::max({ shifted.r, shifted.g, shifted.b }) };

    if(0.0f > lowest && shiftedLuminosity > lowest)
        shifted = shiftedLuminosity + (shifted - shiftedLuminosity) * shiftedLuminosity / (shiftedLuminosity - lowest);

    if(1.0f < highest && highest > shiftedLuminosity)
        shifted = shiftedLuminosity + (shifted - shiftedLuminosity) * (1.0f - shiftedLuminosity) / (highest - shiftedLuminosity);

    return shifted;
}

f32 RetroFuturaGUI::ColorGlyphPainter::getSaturation(const glm::vec3& color)
{
    return std::max({ color.r, color.g, color.b }) - std::min({ color.r, color.g, color.b });
}

glm::vec3 RetroFuturaGUI::ColorGlyphPainter::setSaturation(const glm::vec3& color, const f32 saturation)
{
    //The lowest channel goes to 0, the highest to the saturation, the middle one keeps its place between them
    const f32
        lowest { std::min({ color.r, color.g, color.b }) },
        highest { std::max({ color.r, color.g, color.b }) };

    if(highest <= lowest)
        return glm::vec3(0.0f);

    return (color - lowest) * saturation / (highest - lowest);
}
