// The code of this class is inspired by johnWRS' implementation of FreeType: https://github.com/johnWRS/LearnOpenGLTextRenderingImprovement

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include "PlatformBridge.hpp"
#include <print>
#include "Projection.hpp"
#include "ShaderManager.hpp"
#include <string>
#include <vector>

namespace RetroFuturaGUI
{
    class Text
    {
    public:
        enum class TextAlignment : u32
        {
            LEFT,
            RIGHT,
            CENTER,
            //BLOCK
        };

        Text(Projection& projection, const std::string& text, const std::string& fontPath, const glm::vec4& textColor, const f32 width, const f32 height, const f32 x, const f32 y, const f32 rotation = 0.0f, const TextAlignment textAlignment = TextAlignment::LEFT, const f32 textPadding = 5.0f, const glm::vec2& parentSpan = glm::vec2(0.0f));
        ~Text();
        void Draw();
        void SetTextAlignment(TextAlignment textAlignment);
        void Resize(const f32 width, const f32 height);
        void Move(const f32 x, const f32 y);
        void Rotate(const f32 rotation);

        struct Character
        {
            i32 TextureID;
            glm::ivec2 Size;
            glm::ivec2 Bearing;
            u32 Advance;
        };

    private:
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
        Projection& _projection;
        glm::vec2 _scale = glm::vec2(1.0f);
        glm::vec2 _position = glm::vec3(0.0f);
        f32 _rotation = 0.0f;
        glm::mat4 _scaleMatrix = glm::mat4(0.0f);
        glm::mat4 _translationMatrix = glm::mat4(0.0f);
        glm::mat4 _rotationMatrix = glm::mat4(0.0f);
        glm::vec2 _resolution = glm::vec2(1.0f);
        glm::vec4 _textColor = glm::vec4(1.0f);
        glm::vec2 _textSpan = glm::vec2(0.0f);
        const f32 _1emFraction = 0.00390625f;
        f32 _lineSpacingFactor = 1.3f;
        TextAlignment _textAlignment = TextAlignment::LEFT;
        f32 _textPadding = 5.0f;
        f32 _textBaseHeight = 8.0f;
        glm::vec2 _parentSpan = glm::vec2(0.0f);

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