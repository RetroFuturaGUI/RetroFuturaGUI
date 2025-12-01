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

    private:
        struct Character
        {
            i32 TextureID;
            glm::ivec2 Size;
            glm::ivec2 Bearing;
            u32 Advance;
        };

        //geometry
        Projection& _projection;
        glm::vec2 _position;
        glm::vec2 _parentSize;
        f32 _rotation;
        //glm::mat4 _scaleMatrix = glm::mat4(0.0f);
        glm::mat4 _translationMatrix = glm::mat4(0.0f);
        glm::mat4 _rotationMatrix = glm::mat4(0.0f);

        const u32 ARRAY_LIMIT = 400;
        std::map<char, Character> _characters;
        u32 _vao;
        u32 _vbo;
        u32 _textureArrayObj;
        std::vector<glm::mat4> _transforms;
        std::vector<i32> _letterMap;
        static inline FT_Library _ft;
        std::string _fontPath = PlatformBridge::Fonts::GetFontsInformation().front().second;
        std::string _text = "";
        glm::vec2 _glyphSize = glm::vec2(1.0f);

        glm::vec4 _textColor = glm::vec4(1.0f);
        glm::vec2 _textSpan = glm::vec2(0.0f);
        const f32 _1emFraction = 0.00390625f;
        f32 _lineSpacingFactor = 1.3f;
        TextAlignment _textAlignment = TextAlignment::LEFT;
        f32 _textPadding = 5.0f;
        f32 _textBaseHeight = 8.0f;

        f32 vertex_data[8] = 
        {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
        };

        i32 initFreeTypeLibrary();
        i32 initFontFace();
        void bind();
        void textRenderCallback(const u32 length);
        void calculateTextSpan();
    };
}