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
    };

    class Text
    {
    public:
        Text(const GeometryParams2D& geometry, const TextParams& textParams);
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
        u32 _fontSize { 16 };
        std::shared_ptr<FontInfo> _fontInfo;
        std::string _text;
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
        u32 _vao { 0 },
            _vbo { 0 };

        void updateMesh();
        void alignPosition();
        void calculateTextSpan();
    };
}