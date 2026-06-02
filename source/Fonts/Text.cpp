#include "Text.hpp"
#include <algorithm>
#include <print>
#include <cstdint>
#include <stdexcept>

#ifndef _MSC_VER
    using std::max;
#endif

namespace RetroFuturaGUI
{    
    Text::Text(const GeometryParams3D& geometry, const TextParams& textParams)
        : _projection(const_cast<Projection&>(geometry._Projection)),
          _glyphSize(textParams._GlyphSize),
          _fontSize(_glyphSize.x),
          _fontSlant(textParams._FontSlant),
          _fontWeight(textParams._FontWeight),
          _fontIndex(FontManager::FontSizeToIntegral(_fontSize)),
          _fontInfo(FontManager::GetFontInfo(textParams._FontName, _fontSize, _fontSlant, _fontWeight)),
          _textColor(textParams._TextColor),
          _textAlignment(textParams._TextAlignment),
          _textPadding(textParams._TextPadding),
          _position(geometry._Position),
          _parentSize(geometry._Size),
          _rotation(geometry._Rotation)
    {
        if (!_fontInfo)
        {
            std::println("ERROR: Font info not found for font {}", textParams._FontName);
            return;
        }

        _text = textParams._Text;
        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), nullptr); // position
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
        if (_vertices.empty() || !_fontInfo || _glyphDraws.empty())
            return;

        ShaderManager::GetFontAtlasFillShader().UseProgram();
        ShaderManager::GetFontAtlasFillShader().SetUniformVec3("textColor", _textColor);
        ShaderManager::GetFontAtlasFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
        ShaderManager::GetFontAtlasFillShader().SetUniformMat4("uPosition", _translationMatrix);
        ShaderManager::GetFontAtlasFillShader().SetUniformMat4("uRotation", _rotationMatrix);
        ShaderManager::GetFontAtlasFillShader().SetUniformMat4("uScaling", glm::mat4(1.0f));
        ShaderManager::GetFontAtlasFillShader().SetUniformInt("uTexture", 0);

        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(f32), _vertices.data(), GL_DYNAMIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        
        for (const auto& draw : _glyphDraws)
        {
            glBindTexture(GL_TEXTURE_2D, draw._TextureID);
            glDrawArrays(GL_TRIANGLES, draw._VertexStart, draw._VertexCount);
        }

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

    u32 Text::findGlyphBlockKey(const u32 codePoint)
    {
        for (const auto& [blockFirst, blockLast] : UnicodeBlocks)
        {
            if (codePoint >= blockFirst && codePoint <= blockLast)
                return blockFirst;
        }

        return 0;
    }

    void Text::updateMesh()
    {
        _vertices.clear();
        _glyphDraws.clear();
        f32 nativeSize { _fontInfo->_Atlasses[_fontIndex]._FontSize },
            scale { _glyphSize.y / nativeSize },
            currentX { 0.0f },
            currentY { 0.0f },
            xPos { 0.0f },
            yPos { 0.0f },
            width { 0.0f },
            height { 0.0f };
        u32 lastTextureID { 0 },
            blockKey { 0 },
            foundTextureID { 0 };
        i32 vertexStart { 0 },
            vertexCount { 0 };
        bool batching { false };

        for (const u32 codepoint : _text.GetUtf32())
        {
            if (codepoint == U'\n')
            {
                currentY -= _glyphSize.y * _lineSpacingFactor;
                currentX = 0.0f;
                continue;
            }

            blockKey = findGlyphBlockKey(codepoint);
            auto blockIt { _fontInfo->_Atlasses[_fontIndex]._GlyphBlocks.find(blockKey) };
            if (blockIt == _fontInfo->_Atlasses[_fontIndex]._GlyphBlocks.end())
                continue;

            auto glyphIt { blockIt->second._Glyphs.find(codepoint) };
            if (glyphIt == blockIt->second._Glyphs.end())
                continue;

            const Glyph& glyph { glyphIt->second };
            foundTextureID = blockIt->second._TextureID;

            if (codepoint == U' ')
            {
                currentX += glyph._Advance * scale;
                continue;
            }

            xPos = currentX + glyph._Bearing[0] * scale;
            yPos = currentY - (glyph._Size[1] - glyph._Bearing[1]) * scale;
            width = glyph._Size[0] * scale;
            height = glyph._Size[1] * scale;

            // If texture changes, flush previous batch
            if (!batching || foundTextureID != lastTextureID)
            {
                if (batching && vertexCount > 0)
                {
                    _glyphDraws.push_back({lastTextureID, vertexStart, vertexCount});
                    vertexStart += vertexCount;
                    vertexCount = 0;
                }
                lastTextureID = foundTextureID;
                batching = true;
            }

            _vertices.reserve(_vertices.size() + 24);
            _vertices.push_back(xPos);
            _vertices.push_back(yPos + height);
            _vertices.push_back(glyph._UV[0]);
            _vertices.push_back(glyph._UV[1]);
            _vertices.push_back(xPos);
            _vertices.push_back(yPos);
            _vertices.push_back(glyph._UV[0]);
            _vertices.push_back(glyph._UV[3]);
            _vertices.push_back(xPos + width);
            _vertices.push_back(yPos);
            _vertices.push_back(glyph._UV[2]);
            _vertices.push_back(glyph._UV[3]);
            _vertices.push_back(xPos);
            _vertices.push_back(yPos + height);
            _vertices.push_back(glyph._UV[0]);
            _vertices.push_back(glyph._UV[1]);
            _vertices.push_back(xPos + width);
            _vertices.push_back(yPos);
            _vertices.push_back(glyph._UV[2]);
            _vertices.push_back(glyph._UV[3]);
            _vertices.push_back(xPos + width);
            _vertices.push_back(yPos + height);
            _vertices.push_back(glyph._UV[2]);
            _vertices.push_back(glyph._UV[1]);

            vertexCount += 6;
            currentX += glyph._Advance * scale;
        }
        // Flush last batch
        if (batching && vertexCount > 0)
        {
            _glyphDraws.push_back({lastTextureID, vertexStart, vertexCount});
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
        f32 nativeSize = _fontInfo->_Atlasses[_fontIndex]._FontSize > 0 ? static_cast<f32>(_fontInfo->_Atlasses[_fontIndex]._FontSize) : 1.0f;
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

        for (const u32 codepoint : _text.GetUtf32())
        {
            if (codepoint == U'\n')
            {
                finalizeLine();
                continue;
            }

            u32 blockKey = findGlyphBlockKey(codepoint);
            auto blockIt = _fontInfo->_Atlasses[_fontIndex]._GlyphBlocks.find(blockKey);
            if (blockIt == _fontInfo->_Atlasses[_fontIndex]._GlyphBlocks.end())
                continue;

            auto glyphIt = blockIt->second._Glyphs.find(codepoint);
            if (glyphIt == blockIt->second._Glyphs.end())
                continue;

            const Glyph& glyph = glyphIt->second;
            f32 advancePx = glyph._Advance * scale;

            if (codepoint == U' ')
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