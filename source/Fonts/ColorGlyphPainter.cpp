#include "ColorGlyphPainter.hpp"
#include FT_COLOR_H
#include <hb-ot.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

bool RetroFuturaGUI::ColorGlyphPainter::HasPaint(hb_font_t* font, const u32 glyphIndex)
{
    if(!font)
        return false;

    return hb_ot_color_glyph_has_paint(hb_font_get_face(font), glyphIndex);
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

    for(uSize pixel = 0; pixel < glyph.size(); ++pixel)
    {
        const glm::vec4 color { glm::clamp(glyph[pixel], 0.0f, 1.0f) };

        for(uSize channel = 0; channel < 4; ++channel)
            bitmap->_Pixels[pixel * 4 + channel] = static_cast<u8>(std::lround(color[static_cast<glm::length_t>(channel)] * 255.0f));
    }

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
    painter->fill([&premultiplied](const glm::vec2&) { return premultiplied; });
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

    const ColorLine line { readColorLine(colorLine) };
    painter->fill([&](const glm::vec2& point) { return colorAt(line, glm::dot(point - start, direction) / directionLengthSquared); });
}

void RetroFuturaGUI::ColorGlyphPainter::paintRadialGradient(hb_paint_funcs_t*, void* data, hb_color_line_t* colorLine, float x0, float y0, float r0, float x1, float y1, float r1, void*)
{
    ColorGlyphPainter* painter { static_cast<ColorGlyphPainter*>(data) };

    if(!painter)
        return;

    const ColorLine line { readColorLine(colorLine) };
    const glm::vec2
        center0 { x0, y0 },
        center1 { x1, y1 };
    painter->fill([&](const glm::vec2& point)
    {
        f32 t { 0.0f };
        return getRadialPosition(point, center0, r0, center1, r1, &t) ? colorAt(line, t) : glm::vec4(0.0f);
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
    painter->fill([&](const glm::vec2& point)
    {
        f32 angle { std::atan2(point.y - y0, point.x - x0) };

        if(0.0f > angle)
            angle += 2.0f * std::numbers::pi_v<f32>;

        return colorAt(line, (angle - startAngle) / sweep);
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

    for(u32 row = area._Top; row < area._Bottom; ++row)
    {
        for(u32 column = area._Left; column < area._Right; ++column)
        {
            const f32 coverage { painter->getCoverage(column, row) };

            if(0.0f >= coverage)
                continue;

            const uSize pixel { static_cast<uSize>(row) * painter->_width + column };
            const glm::vec4& sourceColor { source._Colors[pixel] };
            glm::vec4& backdropColor { backdrop._Colors[pixel] };

            //Every layer is a group laid over the ones before, so the plain case goes straight; the coverage scales the
            //source, as laying it over a mixed backdrop comes to the same
            if(HB_PAINT_COMPOSITE_MODE_SRC_OVER == mode)
            {
                const glm::vec4 covered { sourceColor * coverage };
                backdropColor = covered + backdropColor * (1.0f - covered.a);
                continue;
            }

            backdropColor = glm::mix(backdropColor, composite(sourceColor, backdropColor, mode), coverage);
        }
    }

    if(area._Left < area._Right && area._Top < area._Bottom)
        addDrawnArea(&backdrop, area);

    //Cleared where it drew, ready for the next push
    for(u32 row = source._Top; row < source._Bottom; ++row)
        std::fill_n(source._Colors.begin() + static_cast<std::ptrdiff_t>(static_cast<uSize>(row) * painter->_width + source._Left), source._Right - source._Left, glm::vec4(0.0f));

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

    for(u32 row = 0; row < clipHeight; ++row)
        for(u32 column = 0; column < clipWidth; ++column)
            clip._Coverage[static_cast<uSize>(row) * clipWidth + column] = coverage[static_cast<uSize>(row) * clipWidth + column] / 255.0f * getCoverage(clip._Left + column, clip._Top + row);

    _clips.push_back(std::move(clip));
}

RetroFuturaGUI::ColorGlyphPainter::Clip RetroFuturaGUI::ColorGlyphPainter::getCurrentArea() const
{
    if(_clips.empty())
        return Clip { ._Left = 0, ._Top = 0, ._Right = _width, ._Bottom = _height, ._Coverage = {} };

    const Clip& clip { _clips.back() };
    return Clip { ._Left = clip._Left, ._Top = clip._Top, ._Right = clip._Right, ._Bottom = clip._Bottom, ._Coverage = {} };
}

f32 RetroFuturaGUI::ColorGlyphPainter::getCoverage(const u32 column, const u32 row) const
{
    if(_clips.empty())
        return 1.0f;

    const Clip& clip { _clips.back() };

    if(column < clip._Left || column >= clip._Right || row < clip._Top || row >= clip._Bottom)
        return 0.0f;

    return clip._Coverage[static_cast<uSize>(row - clip._Top) * (clip._Right - clip._Left) + (column - clip._Left)];
}

template<typename PaintAt>
void RetroFuturaGUI::ColorGlyphPainter::fill(const PaintAt& paintAt)
{
    if(_isMeasuring || _groups.empty())
        return;

    //A transform squashing everything flat draws nothing
    const glm::mat3& transform { _transforms.back() };

    if(0.0f == glm::determinant(transform))
        return;

    const glm::mat3 inverse { glm::inverse(transform) };
    std::vector<glm::vec4>& target { _groups.back()._Colors };
    const Clip area { getCurrentArea() };

    if(area._Left >= area._Right || area._Top >= area._Bottom)
        return;

    addDrawnArea(&_groups.back(), area);

    for(u32 row = area._Top; row < area._Bottom; ++row)
    {
        for(u32 column = area._Left; column < area._Right; ++column)
        {
            const f32 coverage { getCoverage(column, row) };

            if(0.0f >= coverage)
                continue;

            const uSize pixel { static_cast<uSize>(row) * _width + column };

            //The pixel's center in 1/64 pixels from the glyph's origin, y up, taken back into the paint's own space
            const glm::vec3 center { (static_cast<f32>(_left) + column + 0.5f) * 64.0f, (static_cast<f32>(_top) - row - 0.5f) * 64.0f, 1.0f };
            const glm::vec4 color { paintAt(glm::vec2(inverse * center)) * coverage };
            target[pixel] = color + target[pixel] * (1.0f - color.a);
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

    unsigned int stopCount { hb_color_line_get_color_stops(colorLine, 0, nullptr, nullptr) };
    std::vector<hb_color_stop_t> stops(stopCount);
    hb_color_line_get_color_stops(colorLine, 0, &stopCount, stops.data());

    for(unsigned int stop = 0; stop < stopCount; ++stop)
        line._Stops.emplace_back(stops[stop].offset, toPremultiplied(stops[stop].color)); //a foreground stop comes with the foreground's color

    std::ranges::stable_sort(line._Stops, {}, &std::pair<f32, glm::vec4>::first);
    line._Extend = hb_color_line_get_extend(colorLine);
    return line;
}

glm::vec4 RetroFuturaGUI::ColorGlyphPainter::colorAt(const ColorLine& line, const f32 t)
{
    if(line._Stops.empty())
        return glm::vec4(0.0f);

    const f32
        first { line._Stops.front().first },
        last { line._Stops.back().first },
        range { last - first };

    if(0.0f >= range)
        return t < first ? line._Stops.front().second : line._Stops.back().second;

    //Past both ends the stops hold their end colors, repeat, or repeat mirrored
    f32 position { t };

    switch(line._Extend)
    {
        case HB_PAINT_EXTEND_REPEAT:
            position = first + (t - first) - range * std::floor((t - first) / range);
            break;
        case HB_PAINT_EXTEND_REFLECT:
        {
            f32 cycle { (t - first) / (2.0f * range) };
            cycle = 2.0f * (cycle - std::floor(cycle));
            position = first + (1.0f < cycle ? 2.0f - cycle : cycle) * range;
            break;
        }
        default:
            position = std::clamp(t, first, last);
            break;
    }

    for(uSize stop = 1; stop < line._Stops.size(); ++stop)
    {
        const std::pair<f32, glm::vec4>
            &before { line._Stops[stop - 1] },
            &after { line._Stops[stop] };

        if(position > after.first)
            continue;

        const f32 width { after.first - before.first };
        return 0.0f >= width ? after.second : glm::mix(before.second, after.second, (position - before.first) / width);
    }

    return line._Stops.back().second;
}

glm::vec4 RetroFuturaGUI::ColorGlyphPainter::toPremultiplied(const hb_color_t color)
{
    const f32 alpha { hb_color_get_alpha(color) / 255.0f };
    return glm::vec4(hb_color_get_red(color) / 255.0f * alpha, hb_color_get_green(color) / 255.0f * alpha, hb_color_get_blue(color) / 255.0f * alpha, alpha);
}

bool RetroFuturaGUI::ColorGlyphPainter::getRadialPosition(const glm::vec2& point, const glm::vec2& center0, const f32 radius0, const glm::vec2& center1, const f32 radius1, f32* t)
{
    if(!t)
        return false;

    //The circle at t has its center at center0 + t (center1 - center0) and the radius radius0 + t (radius1 - radius0);
    //passing through the point makes a quadratic in t
    const glm::vec2
        centerStep { center1 - center0 },
        fromCenter { point - center0 };
    const f32
        radiusStep { radius1 - radius0 },
        a { glm::dot(centerStep, centerStep) - radiusStep * radiusStep },
        b { glm::dot(fromCenter, centerStep) + radius0 * radiusStep },
        c { glm::dot(fromCenter, fromCenter) - radius0 * radius0 };

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
