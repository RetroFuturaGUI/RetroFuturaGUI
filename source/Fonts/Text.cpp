#include "Text.hpp"

#ifndef _MSC_VER
    using max;
    using min;
#endif

RetroFuturaGUI::Text::Text(Projection* projection)
    : _projection(*projection)
{
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
}

RetroFuturaGUI::Text::~Text()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
}

void RetroFuturaGUI::Text::Draw()
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

void RetroFuturaGUI::Text::SetTextPadding(const f32 padding)
{
    _textPadding = padding;
    alignPosition();
}

void RetroFuturaGUI::Text::SetParentSize(const glm::vec2 size)
{
    _parentSize = size;
    alignPosition();
}

void RetroFuturaGUI::Text::SetScrollOffset(const f32 offset)
{
    if(_scrollOffset == offset)
        return;

    _scrollOffset = offset;
    alignPosition();
    updateMesh(); //glyph clipping is baked into vertex data at mesh-build time, so it must be redone for the new view
}

f32 RetroFuturaGUI::Text::GetScrollOffset() const
{
    return _scrollOffset;
}

void RetroFuturaGUI::Text::SetFontFamily(std::string_view fontFamily, const f32 glyphSize, PlatformBridge::Fonts::Slant slant, PlatformBridge::Fonts::Weight weight)
{
    _glyphSize = glm::vec2(glyphSize);
    _fontSize = glyphSize;
    _fontIndex = FontManager::FontSizeToIntegral(_fontSize);
    _fontSlant = slant;
    _fontWeight = weight;
    _fontInfo = FontManager::GetFontInfo(fontFamily, _fontSize, _fontSlant, _fontWeight);
}

/*void RetroFuturaGUI::Text::SetFontSize(const f32 size)
{
    _glyphSize = glm::vec2(size);
    _fontIndex = FontManager::FontSizeToIntegral(size);
}*/

void RetroFuturaGUI::Text::SetTextAlignment(TextAlignment textAlignment)
{
    _textAlignment = textAlignment;
    calculateTextSpan();
    alignPosition();
}

void RetroFuturaGUI::Text::SetSize(const glm::vec2& glyphSize)
{
    _glyphSize = glyphSize;
    calculateTextSpan();
    alignPosition();
    updateMesh();
}

void RetroFuturaGUI::Text::SetPosition(const glm::vec3& position)
{
    _position = position;
    alignPosition();
}

void RetroFuturaGUI::Text::SetRotation(const glm::vec3& rotation)
{
    f32 
        halfWidth { _textSpan.x * 0.5f },
        halfHeight { _textSpan.y * 0.5f };
    _rotation = rotation;
    glm::vec3 radians = glm::radians(_rotation);
    glm::mat4 rotation3D =
        glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
    _rotationMatrix =
        glm::translate(glm::mat4(1.0f), glm::vec3(halfWidth, halfHeight, 0.0f)) *
        rotation3D *
        glm::translate(glm::mat4(1.0f), glm::vec3(-halfWidth, -halfHeight, 0.0f));
}

void RetroFuturaGUI::Text::SetColor(const glm::vec4& color)
{
    _textColor = color;
}

glm::vec4 RetroFuturaGUI::Text::GetColor() const
{
    return _textColor;
}

void RetroFuturaGUI::Text::SetTextUTF8(std::string_view text)
{
    _text = text;
    calculateTextSpan();
    SetTextAlignment(_textAlignment);
    updateMesh();
}

void RetroFuturaGUI::Text::SetTextUTF32(std::u32string_view text)
{
    _text = text;
    calculateTextSpan();
    SetTextAlignment(_textAlignment);
    updateMesh();
}

u32 RetroFuturaGUI::Text::findGlyphBlockKey(const u32 codePoint)
{
    for (const auto& [blockFirst, blockLast] : UnicodeBlocks)
    {
        if (codePoint >= blockFirst && codePoint <= blockLast)
            return blockFirst;
    }

    return 0;
}

void RetroFuturaGUI::Text::updateMesh()
{
    _vertices.clear();
    _glyphDraws.clear();

    if(!_fontInfo)
        return;

    if(_text.GetUtf32().empty())
    {
        _glyphPositions.clear();
        return;
    }

    const GlyphAtlas& _atlas { _fontInfo->_Atlasses.at(_fontIndex) };

    f32
        nativeSize { _atlas._FontSize },
        scale { _glyphSize.y / nativeSize },
        currentX { 0.0f },
        currentY { 0.0f },
        xPos { 0.0f },
        yPos { 0.0f },
        width { 0.0f },
        height { 0.0f };
    u32 
        lastTextureID { 0 },
        blockKey { 0 },
        foundTextureID { 0 };
    i32 
        vertexStart { 0 },
        vertexCount { 0 };
    bool batching { false };

    _glyphPositions.clear();
    _glyphPositions.push_back(0.0f);

    for (const u32 codepoint : _text.GetUtf32())
    {
        if (codepoint == U'\n')
        {
            currentY -= _glyphSize.y * _lineSpacingFactor;
            currentX = 0.0f;
            _glyphPositions.push_back(currentX); //Every codepoint must occupy exactly one _glyphPosition
            continue;
        }

        blockKey = findGlyphBlockKey(codepoint);
        auto blockIterator { _atlas._GlyphBlocks.find(blockKey) };

        if (blockIterator == _atlas._GlyphBlocks.end())
        {
            //No block covers this codepoint (font doesn't support it) - still needs a slot for making text processable
            _glyphPositions.push_back(currentX);
            continue;
        }

        auto glyphIt { blockIterator->second._Glyphs.find(codepoint) };
        if (glyphIt == blockIterator->second._Glyphs.end())
        {
            _glyphPositions.push_back(currentX); //Block found but this specific codepoint has no glyph in it
            continue;
        }

        const Glyph& glyph { glyphIt->second };
        foundTextureID = blockIterator->second._TextureID;

        xPos = currentX + glyph._Bearing[0] * scale;
        yPos = currentY - (glyph._Size[1] - glyph._Bearing[1]) * scale;
        width = glyph._Size[0] * scale;
        height = glyph._Size[1] * scale;

        // Clipping calculations
        const float 
            widgetLeft { _position.x - _parentSize.x * 0.5f },
            widgetRight { widgetLeft + _parentSize.x },
            widgetBottom { _position.y - _parentSize.y * 0.5f },
            widgetTop { widgetBottom + _parentSize.y },
            glyphWorldLeft { _positionAligned.x + xPos },
            glyphWorldBottom { _positionAligned.y + yPos },
            clippedLocalLeft { max(0.0f, widgetLeft - glyphWorldLeft) },
            clippedLocalRight { min(width, widgetRight - glyphWorldLeft) },
            clippedLocalBottom { max(0.0f, widgetBottom - glyphWorldBottom) },
            clippedLocalTop { min(height, widgetTop - glyphWorldBottom) };

        if (clippedLocalRight <= clippedLocalLeft || clippedLocalTop <= clippedLocalBottom)
        {
            currentX += glyph._Advance * scale;
            _glyphPositions.push_back(currentX);
            continue;
        }

        const float 
            clippedWidth { clippedLocalRight - clippedLocalLeft },
            uSpan { glyph._UV[2] - glyph._UV[0] },
            vSpan { glyph._UV[3] - glyph._UV[1] },
            uMin { glyph._UV[0] + (clippedLocalLeft / width) * uSpan },
            uMax { glyph._UV[0] + (clippedLocalRight / width) * uSpan },
            vMin { glyph._UV[1] + (clippedLocalBottom / height) * vSpan },
            vMax { glyph._UV[1] + (clippedLocalTop / height) * vSpan };
        //clipping calculations done

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
        _vertices.push_back(xPos + clippedLocalLeft);
        _vertices.push_back(yPos + height - clippedLocalTop);
        _vertices.push_back(uMin);
        _vertices.push_back(vMax);
        _vertices.push_back(xPos + clippedLocalLeft);
        _vertices.push_back(yPos + height - clippedLocalBottom);
        _vertices.push_back(uMin);
        _vertices.push_back(vMin);
        _vertices.push_back(xPos + clippedLocalLeft + clippedWidth);
        _vertices.push_back(yPos + height - clippedLocalBottom);
        _vertices.push_back(uMax);
        _vertices.push_back(vMin);
        _vertices.push_back(xPos + clippedLocalLeft);
        _vertices.push_back(yPos + height - clippedLocalTop);
        _vertices.push_back(uMin);
        _vertices.push_back(vMax);
        _vertices.push_back(xPos + clippedLocalLeft + clippedWidth);
        _vertices.push_back(yPos + height - clippedLocalBottom);
        _vertices.push_back(uMax);
        _vertices.push_back(vMin);
        _vertices.push_back(xPos + clippedLocalLeft + clippedWidth);
        _vertices.push_back(yPos + height - clippedLocalTop);
        _vertices.push_back(uMax);
        _vertices.push_back(vMax);

        vertexCount += 6;
        currentX += glyph._Advance * scale;
        _glyphPositions.push_back(currentX);
    }

    // Flush last batch
    if (batching && vertexCount > 0)
        _glyphDraws.push_back({lastTextureID, vertexStart, vertexCount});
}

glm::vec3 RetroFuturaGUI::Text::GetBoundaryPosition(const uSize boundary, const f32 caretSize) const
{
    f32 x { 0.0f };

    if(!_glyphPositions.empty())
        x = _glyphPositions[min(boundary, _glyphPositions.size() - 1)];

    return glm::vec3(x, caretSize * 0.25f, 0) + glm::vec3(_positionAligned, _position.z + 0.01f);
}

uSize RetroFuturaGUI::Text::GetBoundaryAtPosition(const f32 worldX) const
{
    if(_glyphPositions.empty())
        return 0;

    const f32 localX { worldX - _positionAligned.x };
    uSize closest { 0 };
    f32 closestDistance { std::abs(_glyphPositions[0] - localX) };

    for(uSize i = 1; i < _glyphPositions.size(); ++i)
    {
        const f32 distance { std::abs(_glyphPositions[i] - localX) };

        if(distance < closestDistance)
        {
            closestDistance = distance;
            closest = i;
        }
    }

    return closest;
}

void RetroFuturaGUI::Text::alignPosition()
{
    switch(_textAlignment)
    {
        case TextAlignment::Center:
        {
            _positionAligned = glm::vec2(_position.x - _textSpan.x * 0.5f, _position.y - _textBaseHeight * 0.5f);
        } break;
        case TextAlignment::Right:
        {
            _positionAligned = glm::vec2(_position.x - _textSpan.x + _parentSize.x * 0.5f - _textPadding, _position.y - _textBaseHeight * 0.5f);
        } break;
        default: //LEFT
        {
            _positionAligned = glm::vec2(_position.x - _parentSize.x * 0.5f + _textPadding, _position.y - _textBaseHeight * 0.5f);
        }
    }

    _positionAligned.x -= _scrollOffset;

    _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_positionAligned, _position.z));
}

void RetroFuturaGUI::Text::calculateTextSpan()
{
    if(!_fontInfo)
        return;

    const GlyphAtlas& _atlas { _fontInfo->_Atlasses.at(_fontIndex) };

    f32
        nativeSize { _atlas._FontSize > 0 ? static_cast<f32>(_atlas._FontSize) : 1.0f },
        scale { _glyphSize.y / nativeSize },
        maxLineWidth { 0.0f },
        totalHeight { 0.0f },
        currentLineWidth { 0.0f },
        penX { 0.0f };

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
        auto blockIterator = _atlas._GlyphBlocks.find(blockKey);
        
        if (blockIterator == _atlas._GlyphBlocks.end())
            continue;

        auto glyphIt = blockIterator->second._Glyphs.find(codepoint);
        if (glyphIt == blockIterator->second._Glyphs.end())
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

const std::string& RetroFuturaGUI::Text::GetTextUTF8() const
{
    return _text.GetUtf8();
}

const std::u32string& RetroFuturaGUI::Text::GetTextUTF32() const
{
    return _text.GetUtf32();
}

float RetroFuturaGUI::Text::GetGlyphSize() const
{
    return _glyphSize.x;
}

uSize RetroFuturaGUI::Text::GetGlyphCount() const
{
    return _glyphPositions.empty() ? 0 : _glyphPositions.size() - 1;
}

uSize RetroFuturaGUI::Text::GetUtf8Position(uSize const glyphPosition) const
{
    return glyphPosition;
}

f32 RetroFuturaGUI::Text::GetHeight() const
{
    return _textBaseHeight;
}

f32 RetroFuturaGUI::Text::GetWidth() const
{
    return _textSpan.x;
}

f32 RetroFuturaGUI::Text::GetPadding() const
{
    return _textPadding;
}