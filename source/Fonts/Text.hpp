#pragma once
#include "FontManager.hpp"
#include "DoubleEncodedString.hpp"
#include "Projection.hpp"
#include "ShaderManager.hpp"
#include <string>
#include <vector>
#include <GL/gl.h>

namespace RetroFuturaGUI
{
    enum class TextAlignment: u32
    {
        Left,
        Right,
        Center,
        //Block
    };

    struct GlyphDraw
    {
        u32 _TextureID { 0 };
        i32 _VertexStart { 0 },
            _VertexCount { 0 };
    };

    class Text
    {
    public:
        /// @brief Constructs a Text object that renders using the given projection.
        Text(Projection* projection);
        Text() = delete;
        ~Text();

        /// @brief Draws the text.
        void Draw();

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 glyphSize, PlatformBridge::Fonts::Slant slant, PlatformBridge::Fonts::Weight weight);

        /// @brief Sets the horizontal alignment of the text within its parent.
        void SetTextAlignment(TextAlignment textAlignment);

        /// @brief Sets the glyph size.
        void SetSize(const glm::vec2& glyphSize);

        /// @brief Sets the world position of the text.
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the text.
        void SetRotation(const glm::vec3& rotation);

        /// @brief Sets the text color.
        void SetColor(const glm::vec4& color);

        /// @brief Sets the size of the parent widget, used for alignment.
        void SetParentSize(const glm::vec2 size);

        /// @brief Sets the padding applied around the text when aligning.
        void SetTextPadding(const f32 padding);

        /// @brief Sets the text content from a UTF-8 encoded string.
        void SetTextUTF8(std::string_view text);

        /// @brief Sets the text content from a UTF-32 encoded string.
        void SetTextUTF32(std::u32string_view text);

        /// @brief Sets the horizontal scroll offset, used when the text overflows its parent.
        void SetScrollOffset(const f32 offset);

        /// @brief Returns the current horizontal scroll offset.
        f32 GetScrollOffset() const;

        /// @brief Returns the text color.
        glm::vec4 GetColor() const;

        /// @brief Returns the glyph size.
        float GetGlyphSize() const;

        /// @brief Returns the glyph height.
        float GetHeight() const;

        /// @brief Returns the width of the rendered text span.
        float GetWidth() const;

        /// @brief Returns the padding
        float GetPadding() const;

        /// @brief Returns the world position of the given glyph boundary, useful for placing a caret.
        /// @param boundary The glyph boundary index.
        /// @param caretSize The width to offset by so the caret sits on the correct side of the boundary.
        /// @return The world position of the boundary.
        glm::vec3 GetBoundaryPosition(const uSize boundary, const f32 caretSize) const;

        /// @brief Returns the glyph boundary closest to the given world X position.
        /// @return The glyph boundary index.
        uSize GetBoundaryAtPosition(const f32 worldX) const;

        /// @brief Returns the text content as a UTF-8 encoded string.
        const std::string& GetTextUTF8() const;

        /// @brief Returns the text content as a UTF-32 encoded string.
        const std::u32string& GetTextUTF32() const;

        /// @brief Returns the number of glyphs in the text.
        uSize GetGlyphCount() const;

        /// @brief Converts a glyph index to its corresponding byte position within the UTF-8 string.
        /// @return The UTF-8 byte position.
        uSize GetUtf8Position(uSize const glyphPosition) const;

    private:
        Projection& _projection;
        glm::vec2 _glyphSize { 16.0f };
        f32 _fontSize { 16.0f };
        PlatformBridge::Fonts::Slant _fontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };
        u32 _fontIndex { 0 };
        std::shared_ptr<FontInfo> _fontInfo;
        DoubleEncodedString _text;
        glm::vec4 _textColor { 1.0f };
        TextAlignment _textAlignment { TextAlignment::Left };
        f32 _textPadding { 0.0f };
        f32 _scrollOffset { 0.0f };

        // geometry
        glm::vec3 _position { 0.0f };
        glm::vec2 _positionAligned { 0.0f },
            _parentSize { 0.0f },
            _textSpan { 0.0f };
        glm::vec3 _rotation { 0.0f };
        f32 _textBaseHeight { 0.0f },
            _lineSpacingFactor { 1.3f };
        glm::mat4 _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
        static inline constinit const f32 _1emFraction { 0.00390625f };
        std::vector<f32> _glyphPositions {};

        // mesh
        std::vector<f32> _vertices; // x,y,u,v per vertex
        std::vector<GlyphDraw> _glyphDraws;
        u32 
            _vao { 0 },
            _vbo { 0 };

        void updateMesh();
        void alignPosition();
        void calculateTextSpan();
        u32 findGlyphBlockKey(const u32 codePoint);
    };
}