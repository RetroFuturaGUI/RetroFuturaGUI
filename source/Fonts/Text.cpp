#include "Text.hpp"
#include "ShaderManager.hpp"
#include <algorithm>

RetroFuturaGUI::Text::Text(Projection* projection)
    : _projection(*projection)
{
    initMesh(&_mesh);
    initMesh(&_borderMesh);
    SetColors(std::span<const glm::vec4>(&_defaultColor, 1));
}

RetroFuturaGUI::Text::~Text()
{
    glDeleteBuffers(1, &_mesh._Vbo);
    glDeleteBuffers(1, &_borderMesh._Vbo);
}

void RetroFuturaGUI::Text::initMesh(GlyphMesh* mesh)
{
    glGenBuffers(1, &mesh->_Vbo);
    mesh->_VertexArray = std::make_unique<VertexArray>([vbo = mesh->_Vbo]()
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), nullptr); // position
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32))); // uv
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    });
}

void RetroFuturaGUI::Text::Draw()
{
    if (!_fontInfo)
        return;

    if(_colors.empty())
        return;

    drawBorder();

    if(_mesh._Draws.empty())
        return;

    Shader& shader { ShaderManager::GetTextShader(_fillType) };
    uploadPlacement(shader, _translationMatrix);
    uploadFill(shader, getFogFeatures());

    if(0 != getFogFeatures())
        uploadFog(shader);

    uploadImage(shader, glm::max(_textSpan, glm::vec2(1.0f)));
    drawMesh(shader, &_mesh);
}

void RetroFuturaGUI::Text::drawBorder()
{
    if(0.0f >= _borderWidth || _borderMesh._Draws.empty())
        return;

    //Follows the text's transitions, whichever the widget gave it; a little further back, so the glyphs drawn next pass the depth test
    _border.SetTransitionSettings(_transitionSettings);
    Shader& shader { ShaderManager::GetTextShader(_border._fillType) };
    uploadPlacement(shader, glm::translate(_translationMatrix, glm::vec3(0.0f, 0.0f, -_borderDepthOffset)));
    _border.uploadFill(shader, 0);
    _border.uploadImage(shader, glm::max(_textSpan, glm::vec2(1.0f)));
    drawMesh(shader, &_borderMesh);
}

void RetroFuturaGUI::Text::uploadPlacement(Shader& shader, const glm::mat4& translation) const
{
    shader.UseProgram();
    shader.SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    shader.SetUniformMat4("uPosition", translation);
    shader.SetUniformMat4("uRotation", _rotationMatrix);
    shader.SetUniformMat4("uScaling", glm::mat4(1.0f));
    shader.SetUniformInt("uGlyphs", 0);
    shader.SetUniformFloat("uColorGlyphTint", _colorGlyphTint);

    //The fill, the raster and the fog run across the whole span, whose top is the font's ascender above the first baseline, or for columns the placed position itself
    shader.SetUniformVec2("uScale", glm::max(_textSpan, glm::vec2(1.0f)));
    shader.SetUniformVec2("uSpanCenter", glm::vec2(_textSpan.x * 0.5f, (TextFlow::Horizontal == _textFlow ? _ascender : 0.0f) - _textSpan.y * 0.5f));
}

void RetroFuturaGUI::Text::drawMesh(Shader& shader, GlyphMesh* mesh)
{
    if(!mesh->_VertexArray)
        return;

    mesh->_VertexArray->Bind();

    if(mesh->_IsChanged)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->_Vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh->_Vertices.size() * sizeof(f32), mesh->_Vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        mesh->_IsChanged = false;
    }

    glActiveTexture(GL_TEXTURE0);

    //Glyphs may overlap at the same depth, like the children a font lays over the parents in 👩‍👩‍👧‍👦. GL_LEQUAL lets a later one draw over an earlier one
    GLint depthFunction { GL_LESS };
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
    glDepthFunc(GL_LEQUAL);

    for (const GlyphDraw& draw : mesh->_Draws)
    {
        shader.SetUniformInt("uColorGlyphs", draw._IsColor ? 1 : 0);
        glBindTexture(GL_TEXTURE_2D, draw._TextureID);
        glDrawArrays(GL_TRIANGLES, draw._VertexStart, draw._VertexCount);
    }

    glDepthFunc(static_cast<GLenum>(depthFunction));
    VertexArray::Unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RetroFuturaGUI::Text::SetBorderWidth(const f32 width)
{
    if(_borderWidth == width)
        return;

    _borderWidth = width;
    updateMesh();
}

f32 RetroFuturaGUI::Text::GetBorderWidth() const
{
    return _borderWidth;
}

RetroFuturaGUI::TextBorder& RetroFuturaGUI::Text::GetBorder()
{
    return _border;
}

void RetroFuturaGUI::Text::SetTextPadding(const f32 padding)
{
    _textPadding = padding;
    alignPosition();
    updateMesh();
}

void RetroFuturaGUI::Text::SetParentSize(const glm::vec2 size)
{
    const bool parentSizeChanged { _parentSize != size };
    _parentSize = size;
    alignPosition();

    if(parentSizeChanged)
        updateMesh();
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

void RetroFuturaGUI::Text::SetFontFamily(std::string_view fontFamily, const f32 size, const FontSizeUnit sizeUnit, PlatformBridge::Fonts::Slant slant, PlatformBridge::Fonts::Weight weight)
{
    _fontSize = FontManager::ToPixels(size, sizeUnit);
    _glyphSize = glm::vec2(_fontSize);
    _fontKey = FontManager::FontSizeToKey(_fontSize);
    _fontSlant = slant;
    _fontWeight = weight;
    _fontInfo = FontManager::GetFontInfo(fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);

    // Lays the text out again: text set before the font was never measured, and other glyphs measure differently
    calculateTextSpan();
    alignPosition();
    updateMesh();
}

f32 RetroFuturaGUI::Text::GetFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_fontSize, sizeUnit);
}

void RetroFuturaGUI::Text::SetTextAlignment(TextAlignment textAlignment)
{
    _textAlignment = textAlignment;
    alignPosition();
    updateMesh();
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

void RetroFuturaGUI::Text::SetTextUTF8(std::string_view text)
{
    _text = text;
    calculateTextSpan();
    alignPosition();
    updateMesh();
}

void RetroFuturaGUI::Text::SetTextUTF32(std::u32string_view text)
{
    _text = text;
    calculateTextSpan();
    alignPosition();
    updateMesh();
}

void RetroFuturaGUI::Text::updateMesh()
{
    _mesh._Vertices.clear();
    _mesh._Draws.clear();
    _borderMesh._Vertices.clear();
    _borderMesh._Draws.clear();

    if(!_fontInfo)
        return;

    if(_text.GetUtf32().empty())
    {
        _glyphPositions.clear();
        _glyphLines.clear();
        return;
    }

    const GlyphAtlas& atlas { _fontInfo->_Atlasses.at(_fontKey) };

    const f32
        nativeSize { atlas._FontSize },
        scale { _glyphSize.y / nativeSize },
        borderWidth { _borderWidth / scale }; //widened at the glyphs' own size, so it's as wide as asked once they're scaled
    const std::u32string& text { _text.GetUtf32() };
    uSize lineStart { 0 };

    _glyphPositions.clear();
    _glyphLines.clear();

    for(uSize line = 0; line < _shapedLines.size(); ++line)
    {
        const uSize
            lineEnd { std::min(text.find(U'\n', lineStart), text.size()) },
            lineLength { lineEnd - lineStart };
        f32 along { 0.0f }; //how far along its line the pen is: rightward on a horizontal line, downward in a column
        std::vector<f32> boundaries(lineLength + 1, -1.0f); //where each boundary sits along the line, -1 until known

        for(const PlacedGlyph& placed : _shapedLines[line])
        {
            //A glyph starts its cluster. A character not covered by any registered font draws nothing but keeps its slot
            if(lineLength > placed._Cluster && 0.0f > boundaries[placed._Cluster])
                boundaries[placed._Cluster] = along;

            const Glyph* glyph { FontManager::FindPlacedGlyph(*_fontInfo, _fontKey, placed) };
            const glm::vec2 pen { getPlacedPen(line, along, placed, glyph, scale) };
            along += getPlacedAdvance(placed, glyph, scale);

            if(!glyph)
                continue;

            //The border first, so the glyph's quad comes after it in the same order
            const Glyph* border { 0.0f < _borderWidth ? FontManager::FindPlacedGlyphBorder(*_fontInfo, _fontKey, placed, borderWidth) : nullptr };

            if(border)
                appendGlyphQuad(*border, pen, scale, &_borderMesh);

            appendGlyphQuad(*glyph, pen, scale, &_mesh);
        }

        //The characters inside a cluster (an emoji sequence, a ligature) share the boundary after it
        boundaries[lineLength] = along;

        for(uSize boundary = lineLength; 0 < boundary; --boundary)
        {
            if(0.0f > boundaries[boundary - 1])
                boundaries[boundary - 1] = boundaries[boundary];
        }

        //The first boundary of a later line is the one after its '\n'
        for(const f32 boundary : boundaries)
        {
            _glyphPositions.push_back(boundary);
            _glyphLines.push_back(line);
        }

        lineStart = lineEnd + 1;
    }

    _mesh._IsChanged = true;
    _borderMesh._IsChanged = true;
    FontManager::FlushGlyphUploads();
}

glm::vec2 RetroFuturaGUI::Text::getPlacedPen(const uSize line, const f32 along, const PlacedGlyph& placed, const Glyph* glyph, const f32 scale) const
{
    if(!placed._IsShaped)
        return getPen(line, along, glyph ? glyph->_Advance * scale : 0.0f);

    //Shaped glyphs carry their offsets: in a column from the middle of its top (vertical origin)
    const glm::vec2
        offset { placed._Offset * scale },
        origin { TextFlow::Horizontal == _textFlow
            ? glm::vec2(along, -static_cast<f32>(line) * _lineHeight)
            : glm::vec2(getColumnLeft(line) + _lineHeight * 0.5f, -along) };
    return glm::round(origin + offset);
}

f32 RetroFuturaGUI::Text::getPlacedAdvance(const PlacedGlyph& placed, const Glyph* glyph, const f32 scale) const
{
    //A column's advance runs downward, against y
    if(placed._IsShaped)
        return TextFlow::Horizontal == _textFlow ? placed._Advance.x * scale : -placed._Advance.y * scale;

    return glyph ? getAdvance(*glyph, scale) : 0.0f;
}

f32 RetroFuturaGUI::Text::getAdvance(const Glyph& glyph, const f32 scale) const
{
    //A column steps one em per glyph, as CJK fonts set their ideographs
    return TextFlow::Horizontal == _textFlow ? glyph._Advance * scale : _glyphSize.y;
}

glm::vec2 RetroFuturaGUI::Text::getPen(const uSize line, const f32 along, const f32 horizontalAdvance) const
{
    if(TextFlow::Horizontal == _textFlow)
        return glm::vec2(along, -static_cast<f32>(line) * _lineHeight);

    //Centered in its column, the baseline dropped below the column's top by the ascender's share of the em
    const f32 baselineDrop { _glyphSize.y * _ascender / std::max(_ascender + _descender, 1.0f) };
    return glm::vec2(getColumnLeft(line) + (_lineHeight - horizontalAdvance) * 0.5f, -along - baselineDrop);
}

f32 RetroFuturaGUI::Text::getColumnLeft(const uSize line) const
{
    //Columns are a line height wide; the first one is leftmost, or rightmost when they run right to left
    const uSize column { TextFlow::VerticalRightToLeft == _textFlow ? getLineCount() - 1 - std::min(line, getLineCount() - 1) : line };
    return static_cast<f32>(column) * _lineHeight;
}

uSize RetroFuturaGUI::Text::getLineCount() const
{
    return 1 + static_cast<uSize>(std::ranges::count(_text.GetUtf32(), U'\n'));
}

void RetroFuturaGUI::Text::SetTextFlow(const TextFlow textFlow)
{
    if(_textFlow == textFlow)
        return;

    _textFlow = textFlow;
    calculateTextSpan();
    alignPosition();
    updateMesh();
}

RetroFuturaGUI::TextFlow RetroFuturaGUI::Text::GetTextFlow() const
{
    return _textFlow;
}

void RetroFuturaGUI::Text::SetColorGlyphTint(const f32 tint)
{
    _colorGlyphTint = std::clamp(tint, 0.0f, 1.0f);
}

f32 RetroFuturaGUI::Text::GetColorGlyphTint() const
{
    return _colorGlyphTint;
}

void RetroFuturaGUI::Text::appendGlyphQuad(const Glyph& glyph, const glm::vec2& pen, const f32 scale, GlyphMesh* mesh) const
{
    const f32
        xPos { pen.x + glyph._Bearing[0] * scale },
        yPos { pen.y - (glyph._Size[1] - glyph._Bearing[1]) * scale },
        width { glyph._Size[0] * scale },
        height { glyph._Size[1] * scale };

    // Clipping calculations
    const float
        widgetLeft { _position.x - _parentSize.x * 0.5f },
        widgetRight { widgetLeft + _parentSize.x },
        widgetBottom { _position.y - _parentSize.y * 0.5f },
        widgetTop { widgetBottom + _parentSize.y },
        glyphWorldLeft { _positionAligned.x + xPos },
        glyphWorldBottom { _positionAligned.y + yPos },
        clippedLocalLeft { std::max(0.0f, widgetLeft - glyphWorldLeft) },
        clippedLocalRight { std::min(width, widgetRight - glyphWorldLeft) },
        clippedLocalBottom { std::max(0.0f, widgetBottom - glyphWorldBottom) },
        clippedLocalTop { std::min(height, widgetTop - glyphWorldBottom) };

    if (clippedLocalRight <= clippedLocalLeft || clippedLocalTop <= clippedLocalBottom)
        return;

    const float
        clippedWidth { clippedLocalRight - clippedLocalLeft },
        uSpan { glyph._UV[2] - glyph._UV[0] },
        vSpan { glyph._UV[3] - glyph._UV[1] },
        uMin { glyph._UV[0] + (clippedLocalLeft / width) * uSpan },
        uMax { glyph._UV[0] + (clippedLocalRight / width) * uSpan },
        vMin { glyph._UV[1] + (clippedLocalBottom / height) * vSpan },
        vMax { glyph._UV[1] + (clippedLocalTop / height) * vSpan };
    //clipping calculations done

    //A new batch whenever the page changes: a big block's glyphs spread over several, and a page holds either color glyphs or gray ones
    if(mesh->_Draws.empty() || mesh->_Draws.back()._TextureID != glyph._TextureID)
        mesh->_Draws.push_back(GlyphDraw { ._TextureID = glyph._TextureID, ._VertexStart = static_cast<i32>(mesh->_Vertices.size() / 4), ._VertexCount = 0, ._IsColor = glyph._IsColor });

    std::vector<f32>& vertices { mesh->_Vertices };
    vertices.reserve(vertices.size() + 24);
    vertices.push_back(xPos + clippedLocalLeft);
    vertices.push_back(yPos + height - clippedLocalTop);
    vertices.push_back(uMin);
    vertices.push_back(vMax);
    vertices.push_back(xPos + clippedLocalLeft);
    vertices.push_back(yPos + height - clippedLocalBottom);
    vertices.push_back(uMin);
    vertices.push_back(vMin);
    vertices.push_back(xPos + clippedLocalLeft + clippedWidth);
    vertices.push_back(yPos + height - clippedLocalBottom);
    vertices.push_back(uMax);
    vertices.push_back(vMin);
    vertices.push_back(xPos + clippedLocalLeft);
    vertices.push_back(yPos + height - clippedLocalTop);
    vertices.push_back(uMin);
    vertices.push_back(vMax);
    vertices.push_back(xPos + clippedLocalLeft + clippedWidth);
    vertices.push_back(yPos + height - clippedLocalBottom);
    vertices.push_back(uMax);
    vertices.push_back(vMin);
    vertices.push_back(xPos + clippedLocalLeft + clippedWidth);
    vertices.push_back(yPos + height - clippedLocalTop);
    vertices.push_back(uMax);
    vertices.push_back(vMax);
    mesh->_Draws.back()._VertexCount += 6;
}

glm::vec3 RetroFuturaGUI::Text::GetBoundaryPosition(const uSize boundary, const f32 caretSize) const
{
    f32
        x { 0.0f },
        y { 0.0f };

    if(!_glyphPositions.empty())
    {
        const uSize index { std::min(boundary, _glyphPositions.size() - 1) };

        //In a column, the middle of the column, as far down as the boundary
        if(TextFlow::Horizontal != _textFlow)
            return glm::vec3(getColumnLeft(_glyphLines[index]) + _lineHeight * 0.5f, -_glyphPositions[index], 0) + glm::vec3(_positionAligned, _position.z + 0.01f);

        x = _glyphPositions[index];
        y = -static_cast<f32>(_glyphLines[index]) * _lineHeight; //the same step updateMesh moves down per '\n'
    }

    return glm::vec3(x, y + caretSize * 0.25f, 0) + glm::vec3(_positionAligned, _position.z + 0.01f);
}

uSize RetroFuturaGUI::Text::GetBoundaryAtPosition(const glm::vec2& worldPosition) const
{
    if(_glyphLines.empty())
        return 0;

    //The column under the X, then the boundary closest down it
    if(TextFlow::Horizontal != _textFlow)
    {
        const f32 columnsAcross { std::max(0.0f, (worldPosition.x - _positionAligned.x) / std::max(_lineHeight, 1.0f)) };
        const uSize column { std::min(static_cast<uSize>(columnsAcross), _glyphLines.back()) };
        const uSize line { TextFlow::VerticalRightToLeft == _textFlow ? _glyphLines.back() - column : column };
        return getClosestOnLine(line, _positionAligned.y - worldPosition.y);
    }

    const f32 linesDown { (_positionAligned.y + _ascender - worldPosition.y) / std::max(_lineHeight, 1.0f) };

    if(0.0f > linesDown)
        return GetBoundaryOnLine(0, worldPosition.x);

    return GetBoundaryOnLine(std::min(static_cast<uSize>(linesDown), _glyphLines.back()), worldPosition.x);
}

uSize RetroFuturaGUI::Text::GetBoundaryAtPosition(const f32 worldX) const
{
    return GetBoundaryOnLine(0, worldX);
}

uSize RetroFuturaGUI::Text::GetBoundaryOnLine(const uSize line, const f32 worldX) const
{
    return getClosestOnLine(line, worldX - _positionAligned.x);
}

uSize RetroFuturaGUI::Text::getClosestOnLine(const uSize line, const f32 along) const
{
    const uSize
        first { static_cast<uSize>(std::lower_bound(_glyphLines.begin(), _glyphLines.end(), line) - _glyphLines.begin()) },
        end { static_cast<uSize>(std::upper_bound(_glyphLines.begin(), _glyphLines.end(), line) - _glyphLines.begin()) };

    if(first == end)
        return GetGlyphCount();

    uSize closest { first };
    f32 closestDistance { std::abs(_glyphPositions[first] - along) };

    for(uSize i = first + 1; i < end; ++i)
    {
        const f32 distance { std::abs(_glyphPositions[i] - along) };

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
    //A horizontal text is placed by its first baseline, a vertical one by its top edge
    const f32 originY { TextFlow::Horizontal != _textFlow
        ? (_anchorTop ? _position.y + _parentSize.y * 0.5f - _textPadding : _position.y + _textSpan.y * 0.5f)
        : _anchorTop
            ? _position.y + _parentSize.y * 0.5f - _textPadding - _ascender //the first line's highest glyphs touch the top edge
            : _position.y - (_ascender - _descender) * 0.5f }; //the line from its lowest to its highest glyph centered

    switch(_textAlignment)
    {
        case TextAlignment::Center:
        {
            _positionAligned = glm::vec2(_position.x - _textSpan.x * 0.5f, originY);
        } break;
        case TextAlignment::Right:
        {
            _positionAligned = glm::vec2(_position.x - _textSpan.x + _parentSize.x * 0.5f - _textPadding, originY);
        } break;
        default: //LEFT
        {
            _positionAligned = glm::vec2(_position.x - _parentSize.x * 0.5f + _textPadding, originY);
        }
    }

    _positionAligned.x -= _scrollOffset;

    _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_positionAligned, _position.z));
}

void RetroFuturaGUI::Text::calculateTextSpan()
{
    if(!_fontInfo)
        return;

    const GlyphAtlas& atlas { _fontInfo->_Atlasses.at(_fontKey) };

    const f32
        nativeSize { 0 < atlas._FontSize ? static_cast<f32>(atlas._FontSize) : 1.0f },
        scale { _glyphSize.y / nativeSize };
    f32 longestLine { 0.0f };

    //The font's metrics scale with its glyphs; set first, since the lines below step by them
    _ascender = atlas._Metrics._Ascender * scale;
    _descender = atlas._Metrics._Descender * scale;
    _lineHeight = atlas._Metrics._LineHeight * scale;

    //Each line shaped once here, for updateMesh to place
    const std::u32string& text { _text.GetUtf32() };
    const ShapeDirection direction { TextFlow::Horizontal == _textFlow ? ShapeDirection::LeftToRight : ShapeDirection::TopToBottom };
    _shapedLines.clear();

    for(uSize lineStart = 0; lineStart <= text.size();)
    {
        const uSize lineEnd { std::min(text.find(U'\n', lineStart), text.size()) };
        _shapedLines.emplace_back();
        FontManager::ShapeText(*_fontInfo, _fontKey, std::u32string_view(text).substr(lineStart, lineEnd - lineStart), direction, &_shapedLines.back());
        lineStart = lineEnd + 1;
    }

    for(uSize line = 0; line < _shapedLines.size(); ++line)
    {
        f32
            along { 0.0f },
            lineLength { 0.0f };

        //A horizontal line reaches to its last glyph's ink, or past a trailing space; a column is as long as its steps
        for(const PlacedGlyph& placed : _shapedLines[line])
        {
            const Glyph* glyph { FontManager::FindPlacedGlyph(*_fontInfo, _fontKey, placed) };
            const glm::vec2 pen { getPlacedPen(line, along, placed, glyph, scale) };
            along += getPlacedAdvance(placed, glyph, scale);

            if(TextFlow::Horizontal != _textFlow || U' ' == placed._CodePoint)
                lineLength = std::max(lineLength, along);
            else if(glyph)
                lineLength = std::max(lineLength, pen.x + (glyph->_Bearing[0] + glyph->_Size[0]) * scale);
        }

        longestLine = std::max(longestLine, lineLength);
    }

    //Columns side by side, each a line height wide, as long as the longest
    const f32 lineCount { static_cast<f32>(_shapedLines.size()) };
    _textSpan = TextFlow::Horizontal == _textFlow ? glm::vec2(longestLine, lineCount * _lineHeight) : glm::vec2(lineCount * _lineHeight, longestLine);
    FontManager::FlushGlyphUploads();
}

void RetroFuturaGUI::Text::SetAnchorTop(const bool anchorTop)
{
    _anchorTop = anchorTop;
    alignPosition();
    updateMesh();
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
    return _ascender + _descender;
}

f32 RetroFuturaGUI::Text::GetWidth() const
{
    return _textSpan.x;
}

f32 RetroFuturaGUI::Text::GetSpanHeight() const
{
    return _textSpan.y;
}

f32 RetroFuturaGUI::Text::GetPadding() const
{
    return _textPadding;
}

uSize RetroFuturaGUI::Text::GetBoundaryLine(const uSize boundary) const
{
    if(_glyphLines.empty())
        return 0;

    return _glyphLines[std::min(boundary, _glyphLines.size() - 1)];
}

uSize RetroFuturaGUI::Text::GetLineLastBoundary(const uSize line) const
{
    if(_glyphLines.empty())
        return 0;

    //_glyphLines never decreases, so the line ends right before the first boundary of the next one
    return static_cast<uSize>(std::upper_bound(_glyphLines.begin(), _glyphLines.end(), line) - _glyphLines.begin()) - 1;
}

f32 RetroFuturaGUI::Text::GetLineHeight() const
{
    return _lineHeight;
}