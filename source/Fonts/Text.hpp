#pragma once
#include "FontManager.hpp"
#include "DoubleEncodedString.hpp"
#include "FillStyle.hpp"
#include "FogStyle.hpp"
#include "TextOutline.hpp"
#include "DropShadow.hpp"
#include "Glow.hpp"
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include "ContextObjects.hpp"
#include <memory>
#include <string>
#include <vector>
#include <GL/gl.h>
#include <span>

namespace RetroFuturaGUI
{
    enum class TextAlignment: u32
    {
        Left,
        Right,
        Center,
        //Block
    };

    /// @brief How a text's lines run. Vertical lines are columns read top to bottom, one em per glyph, the glyphs upright
    enum class TextFlow : u32
    {
        Horizontal,
        VerticalLeftToRight, // the first column leftmost, as Latin or Mongolian vertical text runs
        VerticalRightToLeft // the first column rightmost, as Japanese and Chinese vertical text runs
    };

    /// @brief What a text's mask is laid over
    enum class MaskSpan : u32
    {
        WholeText,
        EachLine
    };

    /// @brief Which way an arrow key moves the caret
    enum class CaretMove : u32
    {
        Left,
        Right,
        Up,
        Down
    };

    /// @brief A box of selected text in world space, one per stretch that is selected without a break on its line
    struct TextSelectionArea
    {
        glm::vec2 _Center { 0.0f },
            _Size { 0.0f };
    };

    struct GlyphDraw
    {
        u32 _TextureID { 0 };
        i32 _VertexStart { 0 },
            _VertexCount { 0 };
        bool _IsColor { false }; // color glyphs, drawn in their own colors
    };

    class Text : public FillStyle, public FogStyle
    {
    public:
        /// @brief Constructs a Text object that renders using the given projection
        Text(Projection* projection);
        Text() = delete;
        ~Text();

        /// @brief Draws the text.
        void Draw();

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary
        void SetFontFamily(std::string_view fontFamily, const f32 size, const FontSizeUnit sizeUnit, PlatformBridge::Fonts::Slant slant, PlatformBridge::Fonts::Weight weight);

        /// @brief Returns the font size in the given unit
        f32 GetFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets the horizontal alignment of the text within its parent
        void SetTextAlignment(TextAlignment textAlignment);

        /// @brief Sets the glyph size
        void SetSize(const glm::vec2& glyphSize);

        /// @brief Sets the world position of the text
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the text
        void SetRotation(const glm::vec3& rotation);

    //Fill, glass and raster, from FillStyle, and fog, from FogStyle. White until colors are set
        using FillStyle::SetColors;
        using FillStyle::SetFillType;
        using FillStyle::SetGradientOffset;
        using FillStyle::SetGradientAnimationSpeed;
        using FillStyle::SetGradientDegree;
        using FillStyle::SetGradientRotationSpeed;
        using FillStyle::GetGradientOffset;
        using FillStyle::GetGradientDegree;
        using FillStyle::SetWindowBackgroundImageTextureID;
        using FillStyle::SetBackgroundImageTextureID;
        using FillStyle::SetBackgroundImagePadding;
        using FillStyle::SetBackgroundImageFit;
        using FillStyle::SetMaskTextureID;
        using FillStyle::SetMaskMode;
        using FillStyle::SetMaskFit;
        using FillStyle::SetRasterPattern;
        using FillStyle::SetPrimaryRasterColor;
        using FillStyle::SetSecondaryRasterColor;
        using FillStyle::SetRasterSpacing;
        using FillStyle::SetRasterDegree;
        using FillStyle::SetPrimaryRasterWidthTransfer;
        using FillStyle::SetDotTransparencyTransfer;
        using FillStyle::SetRasterAnimationSpeed;
        using FogStyle::SetFogAlpha;
        using FogStyle::SetFogSpeed;
        using FogStyle::SetFogDensity;
        using FogStyle::SetFogClearing;

        /// @brief Sets the size of the parent widget, used for alignment
        void SetParentSize(const glm::vec2 size);

        /// @brief Sets the padding applied around the text when aligning
        void SetTextPadding(const f32 padding);

        /// @brief Sets the text content from a UTF-8 encoded string
        void SetTextUTF8(std::string_view text);

        /// @brief Sets the text content from a UTF-32 encoded string
        void SetTextUTF32(std::u32string_view text);

        /// @brief Sets the horizontal scroll offset, used when the text overflows its parent
        void SetScrollOffset(const f32 offset);

        /// @brief Returns the current horizontal scroll offset
        f32 GetScrollOffset() const;

        /// @brief Returns the glyph size. The font size in pixels, unless SetSize scaled the glyphs
        f32 GetGlyphSize() const;

        /// @brief Returns one line's height from the font's lowest descender to its highest ascender, without the gap between lines
        f32 GetHeight() const;

        /// @brief Returns the width of the rendered text span
        f32 GetWidth() const;

        /// @brief Returns the height of the rendered text span, line spacing included (covering ascenders and decenders)
        f32 GetSpanHeight() const;

        /// @brief Returns the padding
        f32 GetPadding() const;

        /// @brief Returns the world position of the given glyph boundary, useful for placing a caret
        /// @param boundary The glyph boundary index
        /// @param caretSize The width to offset by so the caret sits on the correct side of the boundary
        /// @return The world position of the boundary
        glm::vec3 GetBoundaryPosition(const uSize boundary, const f32 caretSize) const;

        /// @brief Returns the glyph boundary closest to the given world position: the line under its Y first, then the closest X on that line
        uSize GetBoundaryAtPosition(const glm::vec2& worldPosition) const;
        uSize GetBoundaryAtPosition(const f32 worldX) const;

        /// @brief Returns the glyph boundary on the given line closest to the given world X, or the end of the text if there is no such line
        uSize GetBoundaryOnLine(const uSize line, const f32 worldX) const;

        /// @brief Returns the text content as a UTF-8 encoded string
        const std::string& GetTextUTF8() const;

        /// @brief Returns the text content as a UTF-32 encoded string
        const std::u32string& GetTextUTF32() const;

        /// @brief Returns the number of glyphs in the text
        uSize GetGlyphCount() const;

        /// @brief Converts a glyph index to its corresponding byte position within the UTF-8 string
        /// @return The UTF-8 byte position
        uSize GetUtf8Position(uSize const glyphPosition) const;

        /// @brief Puts the first line just below the parent's top edge instead of centring it on the position for multiline text
        void SetAnchorTop(const bool anchorTop);

        /// @brief Returns the line the given glyph boundary sits on, 0 being the first
        uSize GetBoundaryLine(const uSize boundary) const;

        /// @brief Returns the last glyph boundary of the given line the one right before its '\n'
        uSize GetLineLastBoundary(const uSize line) const;

        /// @brief Returns the distance from one baseline to the next
        f32 GetLineHeight() const;

        /// @brief Returns the boundary an arrow key moves the caret to. On a line, Left and Right go to the nearest boundary that
        ///        way as the glyphs are drawn, so right-to-left text is walked the way it reads, and on past the line's end in
        ///        its reading direction; Up and Down go to the closest boundary on the line above or below, or to the text's
        ///        start and end. In columns, Up and Down step along the column, Left and Right go to the next column that way.
        uSize GetBoundaryBeside(const uSize boundary, const CaretMove move) const;

        /// @brief Returns the boxes covering the characters from one boundary to another, in world space: one per stretch
        ///        drawn without a break, as mixed left-to-right and right-to-left text splits a selection on its line. A
        ///        selected line break shows as a narrow box at its line's end.
        /// @param caretSize As in GetBoundaryPosition, which the boxes line up with.
        std::vector<TextSelectionArea> GetSelectionAreas(const uSize first, const uSize last, const f32 caretSize) const;

        /// @brief Sets how far the outline reaches out from every glyph, in pixels. 0, the default, draws no outline
        void SetOutlineWidth(const f32 width);

        /// @brief Returns how far the outline reaches out from every glyph, in pixels
        f32 GetOutlineWidth() const;

        /// @brief Returns the outline's style: its colors, fill, raster and image, set like the text's own
        TextOutline& GetOutline();

        /// @brief Returns the drop shadow. GetDropShadow().Show(true). Cast by the glyphs and their outline, drawn behind both
        DropShadow& GetDropShadow();
        const DropShadow& GetDropShadow() const;

        /// @brief Returns the glow: GetGlow().Show(true). Around the glyphs and their outline, between them and the drop shadow
        Glow& GetGlow();
        const Glow& GetGlow() const;

        /// @brief Sets how the lines run: horizontally (default), or as columns. Each '\n' starts the next column. Columns are placed by their top edge: centered on the position, or at the parent's top with SetAnchorTop
        void SetTextFlow(const TextFlow textFlow);

        /// @brief Returns how the lines run
        TextFlow GetTextFlow() const;

        /// @brief Sets how far color glyphs (emoji) take on the fill's color, from 0, their own colors (the default), to 1, the
        ///        fill's. The raster, fog, glass and mask apply to them either way; the text's image doesn't, as it would hide them
        void SetColorGlyphTint(const f32 tint);

        /// @brief Returns how far color glyphs take on the fill's color.
        f32 GetColorGlyphTint() const;

        /// @brief Sets what the mask (SetMaskTextureID) is laid over: the whole text, the default, or each line, so a
        ///        chrome mask shades every line of a multiline text as it would a one-line logo. The outline's mask follows
        void SetMaskSpan(const MaskSpan maskSpan);

        /// @brief Returns what the mask is laid over.
        MaskSpan GetMaskSpan() const;

    private:
        /// @brief Quads of glyphs on their pages, batched by page, with the buffer and the vertex arrays drawing them
        struct GlyphMesh
        {
            std::vector<f32> _Vertices {}; // x, y, u, v and the middle of the glyph's line across it, per vertex
            static constexpr uSize _FloatsPerVertex { 5 };
            std::vector<GlyphDraw> _Draws {};
            u32 _Vbo { 0 };
            std::unique_ptr<VertexArray> _VertexArray { nullptr }; //one VAO per context that draws the text, over the same buffer
            bool _IsChanged { false };
        };

        void updateMesh();
        void alignPosition();
        void calculateTextSpan();

        /// @brief Shapes one line running across: split into runs of one direction by the bidirectional algorithm, each
        ///        shaped its way and laid out in the order they're drawn, the clusters counted from the line's start
        /// @param isRightToLeft Set to whether the line reads right to left, from its first strong character
        void shapeLine(std::u32string_view line, std::vector<PlacedGlyph>* glyphs, bool* isRightToLeft) const;
        static void initMesh(GlyphMesh* mesh);

        /// @brief Adds a glyph's quad at the pen (its baseline origin), clipped to the parent, starting a new batch when its page differs
        /// @param lineCenter The middle of the glyph's line across it (y for a line, x for a column), which a mask per line is laid by
        void appendGlyphQuad(const Glyph& glyph, const glm::vec2& pen, const f32 scale, const f32 lineCenter, GlyphMesh* mesh) const;

        /// @brief The middle of a line across it: halfway between its lowest and highest glyph, or a column's middle.
        f32 getLineCenter(const uSize line) const;

        /// @brief Where a line starts across the text: at its left edge, or for a right-to-left line as far in as it leaves
        ///        room, so it ends at the text's right edge, where it starts reading
        f32 getLineStart(const uSize line) const;

        /// @brief Uses the shader and uploads where the text is and the span its fill runs across
        void uploadPlacement(Shader& shader, const glm::mat4& translation) const;
        static void drawMesh(Shader& shader, GlyphMesh* mesh);
        void drawOutline();
        void drawShadow();
        void drawGlow();

        /// @brief How far a glyph placed by itself (its font can't shape) moves the pen along its line: its advance, or one em in a column
        f32 getAdvance(const Glyph& glyph, const f32 scale) const;

        /// @brief The baseline origin of a placed glyph so far along a line: from its shaped offsets, or as getPen places one unshaped
        glm::vec2 getPlacedPen(const uSize line, const f32 along, const PlacedGlyph& placed, const Glyph* glyph, const f32 scale) const;

        /// @brief How far a placed glyph moves the pen along its line.
        f32 getPlacedAdvance(const PlacedGlyph& placed, const Glyph* glyph, const f32 scale) const;

        /// @brief The baseline origin of a glyph so far along a line; in a column the glyph is centered across it
        glm::vec2 getPen(const uSize line, const f32 along, const f32 horizontalAdvance) const;

        /// @brief The left edge of a line's column, from the text's left edge
        f32 getColumnLeft(const uSize line) const;
        uSize getLineCount() const;

        /// @brief The boundary on a line closest to a distance along it, or the end of the text if there is no such line
        uSize getClosestOnLine(const uSize line, const f32 along) const;

        Projection& _projection;
        glm::vec2 _glyphSize { 16.0f };
        f32 _fontSize { 16.0f };
        PlatformBridge::Fonts::Slant _fontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };
        u32 _fontKey { 0 }; // the size's key among the font's atlasses
        const FontInfo* _fontInfo { nullptr };
        DoubleEncodedString _text;
        TextAlignment _textAlignment { TextAlignment::Left };
        TextFlow _textFlow { TextFlow::Horizontal };
        f32 _colorGlyphTint { 0.0f };
        MaskSpan _maskSpan { MaskSpan::WholeText };
        f32 _textPadding { 0.0f };
        f32 _scrollOffset { 0.0f };

    // geometry
        glm::vec3 _position { 0.0f };
        glm::vec2 _positionAligned { 0.0f },
            _parentSize { 0.0f },
            _textSpan { 0.0f };
        glm::vec3 _rotation { 0.0f };
        f32 _ascender { 0.0f }, // the font's line metrics at the glyph size
            _descender { 0.0f },
            _lineHeight { 0.0f };
        glm::mat4 _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
        static inline constinit const f32 _1emFraction { 0.00390625f };
        static constexpr f32 _sameSpotDistance { 0.01f }; //boundaries closer than this along a line are at the same spot
        static inline const glm::vec4 _defaultColor { 1.0f }; //what a text shows before it is given colors
        std::vector<std::vector<PlacedGlyph>> _shapedLines {}; //each line's glyphs, shaped when the text, the font or the flow changes
        std::vector<bool> _isLineRightToLeft {}; //per line, whether it reads right to left, which sets it against the text's right edge
        std::vector<f32> _lineWidths {}; //per line, how far it reaches across
        std::vector<f32> _glyphPositions {}; //where each boundary sits along its line
        std::vector<uSize> _glyphLines {};
        std::vector<glm::vec2> _characterSpans {}; //per boundary, the stretch along the line its character's glyphs cover
        bool _anchorTop { false };

    // mesh
        GlyphMesh
            _mesh {},
            _outlineMesh {},
            _shadowMesh {},
            _glowMesh {};

    // outline
        TextOutline _outline {};
        f32 _outlineWidth { 0.0f };
        static constexpr f32 _outlineDepthOffset { 0.001f }; //behind the glyphs, well within the step between widgets

    // drop shadow
        DropShadow _dropShadow {};
        f32 _shadowMeshBlur { 0.0f }; //the blur the shadow mesh was built with
        bool _hasShadowMesh { false }; //whether the mesh was built with the shadow shown
        static constexpr f32 _shadowDepthOffset { 0.002f }; //behind the outline

    // glow
        Glow _glow {};
        f32 _glowMeshWidth { 0.0f }; //the width the glow mesh was built with
        bool _hasGlowMesh { false }; //whether the mesh was built with the glow shown
        static constexpr f32 _glowDepthOffset { 0.0015f }; //between the outline and the drop shadow
    };
}