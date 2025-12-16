// The code of this class is inspired by johnWRS' implementation of FreeType: https://github.com/johnWRS/LearnOpenGLTextRenderingImprovement

#pragma once
#include "IncludeHelper.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include "PlatformBridge.hpp"
#include "Projection.hpp"
#include <print>
#include "ShaderManager.hpp"
#include <string>
#include <vector>

namespace RetroFuturaGUI
{
    enum class TextAlignment : u32
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
        void Move(const f32 x, const f32 y);
        void Rotate(const f32 rotation);
        void SetColor(const glm::vec4& color);
        glm::vec4 GetColor() const;

    private:
        struct Character
        {
            i32 TextureID;
            glm::ivec2 Size;
            glm::ivec2 Bearing;
            u32 Advance;
        };

        //geometry
        f32 vertex_data[8] = 
        {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
        };

        Projection& _projection;
        u32
            _vao { 0 },
            _vbo { 0 };
        glm::vec2
            _position { 0.0f },
            _parentSize { 0.0f };
        f32 _rotation { 0.0f };
        //glm::mat4 _scaleMatrix = glm::mat4(0.0f);
        glm::mat4
            _translationMatrix { 0.0f },
            _rotationMatrix { 0.0f };

        //text
        const u32 ARRAY_LIMIT { 400 };
        std::map<char, Character> _characters;
        u32 _textureArrayObj { 0 };
        std::vector<glm::mat4> _transforms;
        std::vector<i32> _letterMap;
        static inline FT_Library _ft { nullptr };
        std::string _fontPath { PlatformBridge::Fonts::GetFontsInformation().front().second }; //create FontManager later
        std::string _text;
        glm::vec2 _glyphSize { 1.0f };
        glm::vec2 _textSpan { 0.0f };
        static inline constinit const f32 _1emFraction { 0.00390625f };
        TextAlignment _textAlignment { TextAlignment::LEFT };
        f32
            _lineSpacingFactor { 1.3f },
            _textPadding { 5.0f },
            _textBaseHeight { 8.0f };
        glm::vec4 _textColor { 1.0f };

        i32 initFreeTypeLibrary();
        i32 initFontFace();
        void bind();
        void textRenderCallback(const u32 length);
        void calculateTextSpan();
    };
}