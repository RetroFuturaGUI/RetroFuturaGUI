#pragma once
#include "IGlyphSource.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include <hb.h>
#include <glm/glm.hpp>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief Draws a glyph's COLR version 1 paint: the gradient emoji of Segoe UI Emoji and other newer color fonts.
    //         HarfBuzz walks the glyph's paint and calls back. the outlines it clips to are filled by FreeType, and the
    //         gradients and blend modes are worked out per pixel here. Coordinates are 1/64 pixels, y up, as FreeType's are.
    class ColorGlyphPainter
    {
    public:
        /// @brief Returns whether the glyph has COLR version 1 paint to draw.
        static bool HasPaint(hb_font_t* font, const u32 glyphIndex);

        /// @brief Draws the glyph's paint into a color bitmap, premultiplied. The font's scale has to be 64 per pixel, the
        ///        size the face is set to, so its paint and the face's outlines line up.
        /// @return false when there is nothing to draw.
        static bool Paint(FT_Face face, hb_font_t* font, const u32 glyphIndex, GlyphBitmap* bitmap);

    private:
        /// @brief A gradient's colors premultiplied at their offsets in order, and how they go on past both ends.
        struct ColorLine
        {
            std::vector<std::pair<f32, glm::vec4>> _Stops {};
            hb_paint_extend_t _Extend { HB_PAINT_EXTEND_PAD };
        };

        /// @brief Where painting may reach: coverage per pixel within a rectangle of the bitmap, nothing outside it. Each clip is already cut by the ones around it, so only the innermost is asked.
        struct Clip
        {
            u32 _Left { 0 }, // the rectangle, in pixels from the bitmap's top left, the right and bottom edges outside it
                _Top { 0 },
                _Right { 0 },
                _Bottom { 0 };
            std::vector<f32> _Coverage {}; // row by row, the rectangle's width per row
        };

        /// @brief What a group has drawn, premultiplied, across the whole bitmap, and the rectangle it drew into so far.
        ///        HarfBuzz puts every layer in a group of its own, so only that rectangle is laid down and cleared again.
        struct Group
        {
            std::vector<glm::vec4> _Colors {};
            u32 _Left { 0 }, // empty until something is drawn: the left edge right of the right one
                _Top { 0 },
                _Right { 0 },
                _Bottom { 0 };
        };

        ColorGlyphPainter(FT_Face face, const bool isMeasuring);

        /// @brief The callbacks HarfBuzz paints through, made once.
        static hb_paint_funcs_t* getPaintFuncs();

        static void pushTransform(hb_paint_funcs_t* funcs, void* data, float xx, float yx, float xy, float yy, float dx, float dy, void* userData);
        static void popTransform(hb_paint_funcs_t* funcs, void* data, void* userData);
        static void pushClipGlyph(hb_paint_funcs_t* funcs, void* data, hb_codepoint_t glyph, hb_font_t* font, void* userData);
        static void pushClipRectangle(hb_paint_funcs_t* funcs, void* data, float xMin, float yMin, float xMax, float yMax, void* userData);
        static void popClip(hb_paint_funcs_t* funcs, void* data, void* userData);
        static void paintColor(hb_paint_funcs_t* funcs, void* data, hb_bool_t isForeground, hb_color_t color, void* userData);
        static void paintLinearGradient(hb_paint_funcs_t* funcs, void* data, hb_color_line_t* colorLine, float x0, float y0, float x1, float y1, float x2, float y2, void* userData);
        static void paintRadialGradient(hb_paint_funcs_t* funcs, void* data, hb_color_line_t* colorLine, float x0, float y0, float r0, float x1, float y1, float r1, void* userData);
        static void paintSweepGradient(hb_paint_funcs_t* funcs, void* data, hb_color_line_t* colorLine, float x0, float y0, float startAngle, float endAngle, void* userData);
        static void pushGroup(hb_paint_funcs_t* funcs, void* data, void* userData);
        static void popGroup(hb_paint_funcs_t* funcs, void* data, hb_paint_composite_mode_t mode, void* userData);

        /// @brief Sets the bitmap's extent from 1/64 pixel bounds, a pixel to spare on every side, and clears what it draws into.
        bool setBounds(const glm::vec2& lowest, const glm::vec2& highest);

        /// @brief Fills the coverage of an outline already in place (1/64 pixels, y up), cut by the clip around it, and pushes it as the new clip.
        void pushClipOutline(FT_Outline* outline);

        /// @brief The innermost clip, or the whole bitmap when nothing clips.
        Clip getCurrentArea() const;

        /// @brief The innermost clip's coverage at a pixel of its rectangle; 1 when nothing clips.
        f32 getCoverage(const u32 column, const u32 row) const;

        /// @brief Grows a group's drawn rectangle to take in an area.
        static void addDrawnArea(Group* group, const Clip& area);

        /// @brief Whether a composite mode leaves the backdrop as it is wherever the group drew nothing.
        static bool keepsUntouchedBackdrop(const hb_paint_composite_mode_t mode);

        /// @brief Lays a paint over the current group inside the current clip. paintAt gives the premultiplied color at a
        ///        point of the paint's own space, which the current transform takes to the bitmap.
        template<typename PaintAt>
        void fill(const PaintAt& paintAt);

        /// @brief Reads a gradient's stops, sorted, their colors premultiplied.
        static ColorLine readColorLine(hb_color_line_t* colorLine);

        /// @brief The color a gradient shows at t, which runs from 0 at its start to 1 at its end.
        static glm::vec4 colorAt(const ColorLine& line, const f32 t);

        static glm::vec4 toPremultiplied(const hb_color_t color);

        /// @brief Where a point lies along a gradient between two circles, the largest t whose circle passes through it.
        /// @return false when no circle of the gradient passes through the point, which then shows nothing.
        static bool getRadialPosition(const glm::vec2& point, const glm::vec2& center0, const f32 radius0, const glm::vec2& center1, const f32 radius1, f32* t);

        /// @brief Lays a group (source) onto the one beneath it (backdrop), both premultiplied, as the composite mode says.
        static glm::vec4 composite(const glm::vec4& source, const glm::vec4& backdrop, const hb_paint_composite_mode_t mode);

        /// @brief The separable blend modes, on one channel of colors not premultiplied.
        static f32 blendChannel(const f32 source, const f32 backdrop, const hb_paint_composite_mode_t mode);

        /// @brief The hue, saturation, color and luminosity blend modes, on colors not premultiplied.
        static glm::vec3 blendNonSeparable(const glm::vec3& source, const glm::vec3& backdrop, const hb_paint_composite_mode_t mode);
        static f32 getLuminosity(const glm::vec3& color);
        static glm::vec3 setLuminosity(const glm::vec3& color, const f32 luminosity);
        static f32 getSaturation(const glm::vec3& color);
        static glm::vec3 setSaturation(const glm::vec3& color, const f32 saturation);

        FT_Face _face { nullptr };
        bool _isMeasuring { false }; // the first pass only finds the bounds, from the outlines clipped to
        glm::vec2
            _lowest { 0.0f }, // the bounds found while measuring, 1/64 pixels
            _highest { 0.0f };
        bool _hasBounds { false };
        i32 _left { 0 }, // the bitmap's left and top edges, whole pixels from the glyph's origin
            _top { 0 };
        u32 _width { 0 },
            _height { 0 };
        std::vector<glm::mat3> _transforms {};
        std::vector<Clip> _clips {};
        std::vector<Group> _groups {}; // the first is the glyph
        std::vector<Group> _spareGroups {}; // groups laid down and cleared, kept for the next push
        static constexpr u32 _maxSide { 4096 }; // a sanity limit for a broken font's bounds
    };
}