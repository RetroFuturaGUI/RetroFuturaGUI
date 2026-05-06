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
        std::string_view _FontPath;
        const glm::vec4& _TextColor;
        const glm::vec2& _GlyphSize;
        TextAlignment _TextAlignment;
        f32 _TextPadding;
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
        std::shared_ptr<GlyphAtlas> _atlas;
        std::string _text;
        glm::vec2 _glyphSize;
        glm::vec4 _textColor;
        TextAlignment _textAlignment;
        f32 _textPadding;

        // geometry
        glm::vec2 _position;
        glm::vec2 _positionAligned;
        glm::vec2 _parentSize;
        f32 _rotation;
        glm::mat4 _translationMatrix;
        glm::mat4 _rotationMatrix;

        // mesh
        std::vector<f32> _vertices; // x,y,u,v per vertex
        GLuint _vao { 0 }, _vbo { 0 };

        static inline constinit const f32 _1emFraction { 0.00390625f };
        f32 _lineSpacingFactor { 1.3f };

        void updateMesh();
        void alignPosition();
        void calculateTextSpan();
        glm::vec2 _textSpan;
        f32 _textBaseHeight;
    };
}