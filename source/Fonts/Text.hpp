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

    enum CaretRelativePosition : u32
    {
        Left,
        Right
    };

    class Text
    {
    public:
        Text(Projection* projection);
        Text() = delete;
        ~Text();
        void Draw();
        void SetFontFamily(std::string_view fontFamily, const f32 glyphSize, PlatformBridge::Fonts::Slant slant, PlatformBridge::Fonts::Weight weight);
        void SetTextAlignment(TextAlignment textAlignment);
        void SetSize(const glm::vec2& glyphSize);
        void SetPosition(const glm::vec2& position);
        void SetRotation(const f32 rotation);
        void SetColor(const glm::vec4& color);
        void SetParentSize(const glm::vec2 size);
        void SetTextPadding(const f32 padding);
        void SetText(std::string_view text);
        glm::vec4 GetColor() const;
        float GetGlyphSize() const;
        glm::vec3 GetGlyphPosition(const uSize index, const CaretRelativePosition relativePosition, const f32 caretSize) const;
        const std::string& GetText() const;
        uSize GetGlyphCount() const;

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

        // geometry
        glm::vec2 _position { 0.0f },
            _positionAligned { 0.0f },
            _parentSize { 0.0f },
            _textSpan { 0.0f };
        f32 _rotation { 0.0f },
            _textBaseHeight { 0.0f },
            _lineSpacingFactor { 1.3f };
        glm::mat4 _translationMatrix { 0.0f },
            _rotationMatrix { 0.0f };
        static inline constinit const f32 _1emFraction { 0.00390625f };
        std::vector<f32> _glyphPositions {};

        // mesh
        std::vector<f32> _vertices; // x,y,u,v per vertex
        std::vector<GlyphDraw> _glyphDraws;
        u32 _vao { 0 },
            _vbo { 0 };

        void updateMesh();
        void alignPosition();
        void calculateTextSpan();
        u32 findGlyphBlockKey(const u32 codePoint);
    };
}