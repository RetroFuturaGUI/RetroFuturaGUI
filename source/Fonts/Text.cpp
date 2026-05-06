#include "Text.hpp"
#include <algorithm>
#include <print>

#ifndef _MSC_VER
    using std::max;
#endif

namespace RetroFuturaGUI
{
    std::shared_ptr<GlyphAtlas> getAtlas(std::string_view fontPath)
    {
        for (const auto& font : FontManager::GetFonts())
        {
            if (font._path == fontPath)
            {
                return std::make_shared<GlyphAtlas>(font._atlas);
            }
        }
        
        return std::make_shared<GlyphAtlas>(FontManager::GetFonts().front()._atlas);
    }

    Text::Text(const GeometryParams2D& geometry, const TextParams& textParams)
        : _projection(const_cast<Projection&>(geometry._Projection)),
          _atlas(getAtlas(textParams._FontPath)),
          _text(textParams._Text),
          _glyphSize(textParams._GlyphSize),
          _textColor(textParams._TextColor),
          _textAlignment(textParams._TextAlignment),
          _textPadding(textParams._TextPadding),
          _position(geometry._Position),
          _parentSize(geometry._Size),
          _rotation(geometry._Rotation)
    {
        if (!_atlas)
        {
            std::println("ERROR: Atlas not found for font {}", textParams._FontPath);
            return;
        }

        // Initialize VAO/VBO
        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0); // position
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32))); // uv
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        SetRotation(_rotation);
        calculateTextSpan();
        SetTextAlignment(_textAlignment);
        updateMesh();
    }

    Text::~Text()
    {
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
    }

    void Text::Draw()
    {
        if (_vertices.empty() || !_atlas) return;

        ShaderManager::GetFontAtlasFillShader().UseProgram();
        ShaderManager::GetFontAtlasFillShader().SetUniformVec3("textColor", _textColor);
        ShaderManager::GetFontAtlasFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
        ShaderManager::GetFontAtlasFillShader().SetUniformMat4("uPosition", _translationMatrix);
        ShaderManager::GetFontAtlasFillShader().SetUniformMat4("uRotation", _rotationMatrix);
        ShaderManager::GetFontAtlasFillShader().SetUniformMat4("uScaling", glm::mat4(1.0f));
        ShaderManager::GetFontAtlasFillShader().SetUniformInt("uTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _atlas->_textureID);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(f32), _vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_vertices.size() / 4));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Text::SetTextAlignment(TextAlignment textAlignment)
    {
        _textAlignment = textAlignment;
        alignPosition();
    }

    void Text::Resize(const glm::vec2& glyphSize)
    {
        _glyphSize = glyphSize;
        calculateTextSpan();
        SetTextAlignment(_textAlignment);
        updateMesh();
    }

    void Text::SetPosition(const glm::vec2& position)
    {
        _position = position;
        SetTextAlignment(_textAlignment);
    }

    void Text::SetRotation(const f32 rotation)
    {
        _rotation = rotation;
        f32 halfWidth = _textSpan.x * 0.5f;
        f32 halfHeight = _textSpan.y * 0.5f;
        _rotationMatrix =
            glm::translate(glm::mat4(1.0f), glm::vec3(halfWidth, halfHeight, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(_rotation), glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::translate(glm::mat4(1.0f), glm::vec3(-halfWidth, -halfHeight, 0.0f));
    }

    void Text::SetColor(const glm::vec4& color)
    {
        _textColor = color;
    }

    glm::vec4 Text::GetColor() const
    {
        return _textColor;
    }

    void Text::SetText(std::string_view text)
    {
        _text = text;
        calculateTextSpan();
        SetTextAlignment(_textAlignment);
        updateMesh();
    }

    void Text::updateMesh()
    {
        _vertices.clear();
        f32 nativeSize = _atlas->_FontSize > 0 ? static_cast<f32>(_atlas->_FontSize) : 1.0f;
        f32 scale = _glyphSize.y / nativeSize;
        f32 currentX = 0.0f;
        f32 currentY = 0.0f;

        for (char c : _text)
        {
            if (c == '\n')
            {
                currentY -= _glyphSize.y * _lineSpacingFactor;
                currentX = 0.0f;
                continue;
            }

            auto it = _atlas->_Glyphs.find(static_cast<u32>(c));
            if (it == _atlas->_Glyphs.end())
                continue;

            const Glyph& glyph = it->second;

            if (c == ' ')
            {
                currentX += glyph._Advance * scale;
                continue;
            }

            f32 xpos = currentX + glyph._Bearing[0] * scale;
            f32 ypos = currentY - (glyph._Size[1] - glyph._Bearing[1]) * scale;
            f32 w = glyph._Size[0] * scale;
            f32 h = glyph._Size[1] * scale;

            _vertices.reserve(_vertices.size() + 24);
            _vertices.push_back(xpos);
            _vertices.push_back(ypos + h);
            _vertices.push_back(glyph._UV[0]);
            _vertices.push_back(glyph._UV[1]);
            _vertices.push_back(xpos);
            _vertices.push_back(ypos);
            _vertices.push_back(glyph._UV[0]);
            _vertices.push_back(glyph._UV[3]);
            _vertices.push_back(xpos + w);
            _vertices.push_back(ypos);
            _vertices.push_back(glyph._UV[2]);
            _vertices.push_back(glyph._UV[3]);
            _vertices.push_back(xpos);
            _vertices.push_back(ypos + h);
            _vertices.push_back(glyph._UV[0]);
            _vertices.push_back(glyph._UV[1]);
            _vertices.push_back(xpos + w);
            _vertices.push_back(ypos);
            _vertices.push_back(glyph._UV[2]);
            _vertices.push_back(glyph._UV[3]);
            _vertices.push_back(xpos + w);
            _vertices.push_back(ypos + h);
            _vertices.push_back(glyph._UV[2]);
            _vertices.push_back(glyph._UV[1]);

            currentX += glyph._Advance * scale;
        }
    }

    void Text::alignPosition()
    {
        switch(_textAlignment)
        {
            case TextAlignment::CENTER:
            {
                _positionAligned = glm::vec2(_position.x - _textSpan.x * 0.5f, _position.y - _textBaseHeight * 0.5f);
            } break;
            case TextAlignment::RIGHT:
            {
                _positionAligned = glm::vec2(_position.x - _textSpan.x + _parentSize.x * 0.5f - _textPadding, _position.y - _textBaseHeight * 0.5f);
            } break;
            default: //LEFT
            {
                _positionAligned = glm::vec2(_position.x - _parentSize.x * 0.5f + _textPadding, _position.y - _textBaseHeight * 0.5f);
            }
        }

        _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_positionAligned, 0.0f));
    }

    void Text::calculateTextSpan()
    {
        f32 nativeSize = _atlas->_FontSize > 0 ? static_cast<f32>(_atlas->_FontSize) : 1.0f;
        f32 scale = _glyphSize.y / nativeSize;
        f32 maxLineWidth = 0.0f;
        f32 totalHeight = 0.0f;
        f32 currentLineWidth = 0.0f;
        f32 penX = 0.0f;

        auto finalizeLine = [&]()
        {
            totalHeight += _glyphSize.y * _lineSpacingFactor;
            maxLineWidth = max(maxLineWidth, currentLineWidth);
            currentLineWidth = 0.0f;
            penX = 0.0f;
        };

        for (const char c : _text)
        {
            if (c == '\n')
            {
                finalizeLine();
                continue;
            }

            auto it = _atlas->_Glyphs.find(static_cast<u32>(c));
            if (it == _atlas->_Glyphs.end()) continue;

            const Glyph& glyph = it->second;
            f32 advancePx = glyph._Advance * scale;

            if (c == ' ')
            {
                penX += advancePx;
                currentLineWidth = max(currentLineWidth, penX);
                continue;
            }

            f32 glyphRight = penX + glyph._Bearing[0] * scale + glyph._Size[0] * scale;
            currentLineWidth = max(currentLineWidth, glyphRight);
            penX += advancePx;
        }

        finalizeLine();
        _textSpan = glm::vec2(maxLineWidth, totalHeight);
        _textBaseHeight = _glyphSize.y;
    }
}