#pragma once
#include "FontManager.hpp"
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include "ShaderManager.hpp"
#include <string>
#include <vector>
#include <GL/gl.h>

namespace RetroFuturaGUI
{
    enum class TextAlignment: u32
    {
        LEFT,
        RIGHT,
        CENTER,
        //BLOCK
    };

    struct TextParams
    {
        std::string_view _Text;
        std::string_view _FontName;
        const glm::vec4& _TextColor;
        const glm::vec2& _GlyphSize;
        TextAlignment _TextAlignment { TextAlignment::LEFT };
        f32 _TextPadding { 0.0f };
        PlatformBridge::Fonts::Slant _FontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _FontWeight { PlatformBridge::Fonts::Weight::Normal };
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
        Text(const GeometryParams3D& geometry, const TextParams& textParams);
        ~Text();
        void Draw();
        void SetTextAlignment(TextAlignment textAlignment);
        void Resize(const glm::vec2& glyphSize);
        void SetPosition(const glm::vec2& position);
        void SetRotation(const f32 rotation);
        void SetColor(const glm::vec4& color);
        glm::vec4 GetColor() const;
        void SetText(std::string_view text);

    private:
        Projection& _projection;
        glm::vec2 _glyphSize { 16.0f };
        f32 _fontSize { 16.0f };
        PlatformBridge::Fonts::Slant _fontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };
        u32 _fontIndex { 0 };
        std::shared_ptr<FontInfo> _fontInfo;
        std::vector<u32> _codepoints;
        glm::vec4 _textColor { 1.0f };
        TextAlignment _textAlignment { TextAlignment::LEFT };
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

        // mesh
        std::vector<f32> _vertices; // x,y,u,v per vertex
        std::vector<GlyphDraw> _glyphDraws;
        u32 _vao { 0 },
            _vbo { 0 };

        void updateMesh();
        void alignPosition();
        void calculateTextSpan();
        u32 findGlyphBlockKey(const u32 codePoint);
        std::vector<uint32_t> utf8ToUtf32(std::string_view utf8);
    };
}