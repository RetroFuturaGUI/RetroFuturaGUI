#include "Text.hpp"
#include "ShaderManager.hpp"
#include <SheenBidi/SheenBidi.h>
#include <algorithm>
#include <array>

RetroFuturaGUI::Text::Text(Projection* projection)
    : _projection(*projection)
{
    initMesh(&_mesh); //the outline's and the effects' meshes get theirs once they have glyphs to draw
    SetColors(std::span<const glm::vec4>(&_defaultColor, 1));
}

RetroFuturaGUI::Text::~Text()
{
    glDeleteBuffers(1, &_mesh._Vbo);
    glDeleteBuffers(1, &_outlineMesh._Vbo);
    glDeleteBuffers(1, &_shadowMesh._Vbo);
    glDeleteBuffers(1, &_glowMesh._Vbo);
    glDeleteBuffers(1, &_innerGlowMesh._Vbo);
    glDeleteBuffers(1, &_shadowFade._Mesh._Vbo);
    glDeleteBuffers(1, &_glowFade._Mesh._Vbo);
    glDeleteBuffers(1, &_innerGlowFade._Mesh._Vbo);
}

void RetroFuturaGUI::Text::initMesh(GlyphMesh* mesh)
{
    glGenBuffers(1, &mesh->_Vbo);
    mesh->_VertexArray = std::make_unique<VertexArray>([vbo = mesh->_Vbo]()
    {
        constexpr GLsizei stride { static_cast<GLsizei>(GlyphMesh::_FloatsPerVertex * sizeof(f32)) };
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, nullptr); // position
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(f32))); // uv
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(f32))); // the line's middle
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
    });
}

void RetroFuturaGUI::Text::Draw()
{
    if (!_fontInfo)
        return;

    if(_colors.empty())
        return;

    //Shadow, glow and inner glow glyphs are rendered per blur, width and offset, so a change to them lays the glyphs out again
    if(areEffectMeshesStale())
    {
        beginEffectFades();
        updateMesh();
    }

    drawShadow();
    drawGlow();
    drawOutline();

    if(_mesh._Draws.empty())
        return;

    Shader& shader { ShaderManager::GetTextShader(_fillType) };
    uploadPlacement(shader, _translationMatrix);
    uploadFill(shader, getFogFeatures());

    if(0 != getFogFeatures())
        uploadFog(shader);

    uploadImage(shader, glm::max(_textSpan, glm::vec2(1.0f)));
    drawMesh(shader, &_mesh);
    drawInnerGlow();
}

void RetroFuturaGUI::Text::drawOutline()
{
    if(0.0f >= _outlineWidth || _outlineMesh._Draws.empty())
        return;

    //Follows the text's transitions, whichever the widget gave it; a little further back, so the glyphs drawn next pass the depth test
    _outline.SetTransitionSettings(_transitionSettings);
    Shader& shader { ShaderManager::GetTextShader(_outline._fillType) };
    uploadPlacement(shader, glm::translate(_translationMatrix, glm::vec3(0.0f, 0.0f, -_outlineDepthOffset)));
    _outline.uploadFill(shader, 0);
    _outline.uploadImage(shader, glm::max(_textSpan, glm::vec2(1.0f)));
    drawMesh(shader, &_outlineMesh);
}

void RetroFuturaGUI::Text::drawShadow()
{
    if(!_hasShadowMesh || _shadowMesh._Draws.empty())
        return;

    //Scaled around the middle of the text's span, then moved. Behind the outline, which draws next
    const glm::vec3 center { _textSpan.x * 0.5f, (TextFlow::Horizontal == _textFlow ? _ascender : 0.0f) - _textSpan.y * 0.5f, 0.0f };
    const f32 scale { _dropShadow.getShownScale() }; //moved and scaled as drawn, so these follow the transition. The blur is in the glyphs
    const glm::mat4 placement
    {
        glm::translate(_translationMatrix, glm::vec3(_dropShadow.getShownOffset(), -_shadowDepthOffset))
        * glm::translate(glm::mat4(1.0f), center)
        * glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f))
        * glm::translate(glm::mat4(1.0f), -center)
    };

    _dropShadow.SetTransitionSettings(_transitionSettings);
    Shader& shader { ShaderManager::GetTextShader(_dropShadow._fillType) };
    uploadPlacement(shader, placement);
    _dropShadow.uploadFill(shader, 0);
    _dropShadow.uploadImage(shader, glm::max(_textSpan, glm::vec2(1.0f)));
    drawFading(shader, &_shadowMesh, &_shadowFade, _dropShadow.getGlyphProgress());
}

void RetroFuturaGUI::Text::drawGlow()
{
    if(!_hasGlowMesh || _glowMesh._Draws.empty())
        return;

    _glow.SetTransitionSettings(_transitionSettings);
    Shader& shader { ShaderManager::GetTextShader(_glow._fillType) };
    uploadPlacement(shader, glm::translate(_translationMatrix, glm::vec3(0.0f, 0.0f, -_glowDepthOffset)));
    _glow.uploadFill(shader, 0);
    _glow.uploadImage(shader, glm::max(_textSpan, glm::vec2(1.0f)));
    drawFading(shader, &_glowMesh, &_glowFade, _glow.getGlyphProgress());
}

void RetroFuturaGUI::Text::drawInnerGlow()
{
    if(!_hasInnerGlowMesh || _innerGlowMesh._Draws.empty())
        return;

    _innerGlow.SetTransitionSettings(_transitionSettings);
    Shader& shader { ShaderManager::GetTextShader(_innerGlow._fillType) };
    uploadPlacement(shader, glm::translate(_translationMatrix, glm::vec3(0.0f, 0.0f, _innerGlowDepthOffset)));
    _innerGlow.uploadFill(shader, 0);
    _innerGlow.uploadImage(shader, glm::max(_textSpan, glm::vec2(1.0f)));
    drawFading(shader, &_innerGlowMesh, &_innerGlowFade, _innerGlow.getGlyphProgress());
}

void RetroFuturaGUI::Text::beginEffectFades()
{
    //Only a shown effect whose glyph values changed. Showing or hiding one switches at once
    if(_hasShadowMesh && _dropShadow.IsShown() && _dropShadow.GetBlur() != _shadowMeshBlur)
        beginFade(&_shadowFade, _dropShadow.getGlyphProgress(), _shadowMeshBlur, glm::vec2(0.0f));

    if(_hasGlowMesh && _glow.IsShown() && _glow.GetWidth() != _glowMeshWidth)
        beginFade(&_glowFade, _glow.getGlyphProgress(), _glowMeshWidth, glm::vec2(0.0f));

    if(_hasInnerGlowMesh && _innerGlow.IsShown() && (_innerGlow.GetWidth() != _innerGlowMeshWidth || _innerGlow.GetOffset() != _innerGlowMeshOffset))
        beginFade(&_innerGlowFade, _innerGlow.getGlyphProgress(), _innerGlowMeshWidth, _innerGlowMeshOffset);
}

void RetroFuturaGUI::Text::beginFade(EffectFade* fade, const f32 progress, const f32 width, const glm::vec2& offset)
{
    if(!fade)
        return;

    if(1.0f <= progress)
    {
        fade->_IsShown = false;
        return;
    }

    //Changed again midway: whichever glyphs show more fade out, the older ones while the newer hadn't taken over yet
    const bool keepsOlderGlyphs { fade->_IsShown && 0.5f > fade->_Progress };
    fade->_Progress = 0.0f;
    fade->_IsShown = true;

    if(keepsOlderGlyphs)
        return;

    fade->_Width = width;
    fade->_Offset = offset;
}

void RetroFuturaGUI::Text::drawFading(Shader& shader, GlyphMesh* mesh, EffectFade* fade, const f32 progress)
{
    if(!fade)
        return;

    if(1.0f <= progress)
        fade->_IsShown = false;

    if(!fade->_IsShown)
    {
        drawMesh(shader, mesh);
        return;
    }

    fade->_Progress = progress;

    //The program again in each scope, which uploads the opacity
    {
        Shader::OpacityScope fadingOut { 1.0f - progress };
        shader.UseProgram();
        drawMesh(shader, &fade->_Mesh);
    }

    Shader::OpacityScope fadingIn { progress };
    shader.UseProgram();
    drawMesh(shader, mesh);
}

bool RetroFuturaGUI::Text::areEffectMeshesStale() const
{
    return _dropShadow.IsShown() != _hasShadowMesh || (_hasShadowMesh && _dropShadow.GetBlur() != _shadowMeshBlur)
        || _glow.IsShown() != _hasGlowMesh || (_hasGlowMesh && _glow.GetWidth() != _glowMeshWidth)
        || _innerGlow.IsShown() != _hasInnerGlowMesh
        || (_hasInnerGlowMesh && (_innerGlow.GetWidth() != _innerGlowMeshWidth || _innerGlow.GetOffset() != _innerGlowMeshOffset));
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

    //A mask per line spans a line from its lowest to its highest glyph, or a column's width
    const bool isVertical { TextFlow::Horizontal != _textFlow };
    shader.SetUniformInt("uMaskPerLine", MaskSpan::EachLine == _maskSpan ? 1 : 0);
    shader.SetUniformInt("uIsVertical", isVertical ? 1 : 0);
    shader.SetUniformFloat("uLineExtent", std::max(isVertical ? _lineHeight : _ascender + _descender, 1.0f));
}

void RetroFuturaGUI::Text::drawMesh(Shader& shader, GlyphMesh* mesh)
{
    if(!mesh)
        return;

    if(!mesh->_VertexArray)
        initMesh(mesh); //an outline's or effect's mesh, the first time it is drawn

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

void RetroFuturaGUI::Text::SetOutlineWidth(const f32 width)
{
    if(_outlineWidth == width)
        return;

    _outlineWidth = width;
    updateMesh();
}

f32 RetroFuturaGUI::Text::GetOutlineWidth() const
{
    return _outlineWidth;
}

RetroFuturaGUI::TextOutline& RetroFuturaGUI::Text::GetOutline()
{
    return _outline;
}

RetroFuturaGUI::DropShadow& RetroFuturaGUI::Text::GetDropShadow()
{
    return _dropShadow;
}

const RetroFuturaGUI::DropShadow& RetroFuturaGUI::Text::GetDropShadow() const
{
    return _dropShadow;
}

RetroFuturaGUI::Glow& RetroFuturaGUI::Text::GetGlow()
{
    return _glow;
}

const RetroFuturaGUI::Glow& RetroFuturaGUI::Text::GetGlow() const
{
    return _glow;
}

RetroFuturaGUI::InnerGlow& RetroFuturaGUI::Text::GetInnerGlow()
{
    return _innerGlow;
}

const RetroFuturaGUI::InnerGlow& RetroFuturaGUI::Text::GetInnerGlow() const
{
    return _innerGlow;
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

RetroFuturaGUI::TextAlignment RetroFuturaGUI::Text::GetTextAlignment() const
{
    return _textAlignment;
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
    _outlineMesh._Vertices.clear();
    _outlineMesh._Draws.clear();
    _shadowMesh._Vertices.clear();
    _shadowMesh._Draws.clear();
    _hasShadowMesh = _dropShadow.IsShown();
    _shadowMeshBlur = _dropShadow.GetBlur();
    _shadowMesh._IsChanged = true;
    _glowMesh._Vertices.clear();
    _glowMesh._Draws.clear();
    _hasGlowMesh = _glow.IsShown();
    _glowMeshWidth = _glow.GetWidth();
    _glowMesh._IsChanged = true;
    _innerGlowMesh._Vertices.clear();
    _innerGlowMesh._Draws.clear();
    _hasInnerGlowMesh = _innerGlow.IsShown();
    _innerGlowMeshWidth = _innerGlow.GetWidth();
    _innerGlowMeshOffset = _innerGlow.GetOffset();
    _innerGlowMesh._IsChanged = true;

    for(EffectFade* fade : { &_shadowFade, &_glowFade, &_innerGlowFade })
    {
        fade->_Mesh._Vertices.clear();
        fade->_Mesh._Draws.clear();
        fade->_Mesh._IsChanged = true;
    }

    if(!_fontInfo)
        return;

    if(_text.GetUtf32().empty())
    {
        _glyphPositions.clear();
        _glyphLines.clear();
        _characterSpans.clear();
        _isInsideCluster.clear();
        return;
    }

    const GlyphAtlas& atlas { _fontInfo->_Atlasses.at(_fontKey) };

    const f32
        nativeSize { atlas._FontSize },
        scale { _glyphSize.y / nativeSize },
        outlineWidth { _outlineWidth / scale }, //widened at the glyphs' own size, so it's as wide as asked once they're scaled
        shadowBlur { _dropShadow.GetBlur() / scale }, //blurred there too.
        glowWidth { _glow.GetWidth() / scale },
        innerGlowWidth { _innerGlow.GetWidth() / scale },
        fadingShadowBlur { _shadowFade._Width / scale }, //the old values' glyphs, while they fade out
        fadingGlowWidth { _glowFade._Width / scale },
        fadingInnerGlowWidth { _innerGlowFade._Width / scale };
    const glm::vec2
        innerGlowOffset { _innerGlow.GetOffset() / scale },
        fadingInnerGlowOffset { _innerGlowFade._Offset / scale };
    const f32 glyphOutlineWidth { 0.0f < _outlineWidth ? outlineWidth : 0.0f };
    const std::u32string& text { _text.GetUtf32() };
    uSize lineStart { 0 };

    _glyphPositions.clear();
    _glyphLines.clear();
    _characterSpans.clear();
    _isInsideCluster.clear();

    for(uSize line = 0; line < _shapedLines.size(); ++line)
    {
        const uSize
            lineEnd { std::min(text.find(U'\n', lineStart), text.size()) },
            lineLength { lineEnd - lineStart };
        const f32 lineLeft { getLineStart(line) };
        f32 along { lineLeft }; //how far along its line the pen is: rightward on a horizontal line, downward in a column
        std::vector<glm::vec2> clusterSpans(lineLength, glm::vec2(-1.0f)); //per character starting a cluster, the stretch its glyphs cover
        std::vector<bool> isClusterRightToLeft(lineLength, false);

        for(const PlacedGlyph& placed : _shapedLines[line])
        {
            const Glyph* glyph { FontManager::FindPlacedGlyph(*_fontInfo, _fontKey, placed) };
            const glm::vec2 pen { getPlacedPen(line, along, placed, glyph, scale) };
            const f32 start { along };
            along += getPlacedAdvance(placed, glyph, scale);

            //A character not covered by any registered font draws nothing but keeps its slot
            if(lineLength > placed._Cluster)
            {
                glm::vec2& span { clusterSpans[placed._Cluster] };
                span = 0.0f > span.x ? glm::vec2(std::min(start, along), std::max(start, along)) : glm::vec2(std::min(span.x, start), std::max(span.y, along));
                isClusterRightToLeft[placed._Cluster] = placed._IsRightToLeft;
            }

            if(!glyph)
                continue;

            //The shadow and the outline first, so the glyph's quad comes after them in the same order.
            const Glyph* shadow { _hasShadowMesh ? FontManager::FindPlacedGlyphShadow(placed, shadowBlur, 0.0f < _outlineWidth ? outlineWidth : 0.0f) : nullptr };

            if(shadow)
                appendGlyphQuad(*shadow, pen, scale, getLineCenter(line), &_shadowMesh);

            const Glyph* fadingShadow { _shadowFade._IsShown ? FontManager::FindPlacedGlyphShadow(placed, fadingShadowBlur, glyphOutlineWidth) : nullptr };

            if(fadingShadow)
                appendGlyphQuad(*fadingShadow, pen, scale, getLineCenter(line), &_shadowFade._Mesh);

            //A glow is the glyph widened by half its width and blurred over all of it, so it is full at the glyph's edge and gone at the width.
            const Glyph* glow { _hasGlowMesh ? FontManager::FindPlacedGlyphShadow(placed, glowWidth, (0.0f < _outlineWidth ? outlineWidth : 0.0f) + glowWidth * 0.5f) : nullptr };

            if(glow)
                appendGlyphQuad(*glow, pen, scale, getLineCenter(line), &_glowMesh);

            const Glyph* fadingGlow { _glowFade._IsShown ? FontManager::FindPlacedGlyphShadow(placed, fadingGlowWidth, glyphOutlineWidth + fadingGlowWidth * 0.5f) : nullptr };

            if(fadingGlow)
                appendGlyphQuad(*fadingGlow, pen, scale, getLineCenter(line), &_glowFade._Mesh);

            const Glyph* outline { 0.0f < _outlineWidth ? FontManager::FindPlacedGlyphOutline(*_fontInfo, _fontKey, placed, outlineWidth) : nullptr };

            if(outline)
                appendGlyphQuad(*outline, pen, scale, getLineCenter(line), &_outlineMesh);

            appendGlyphQuad(*glyph, pen, scale, getLineCenter(line), &_mesh);

            //Over the glyph, the same size: it never leaves the glyph
            const Glyph* innerGlow { _hasInnerGlowMesh ? FontManager::FindPlacedGlyphInnerGlow(placed, innerGlowWidth, innerGlowOffset) : nullptr };

            if(innerGlow)
                appendGlyphQuad(*innerGlow, pen, scale, getLineCenter(line), &_innerGlowMesh);

            const Glyph* fadingInnerGlow { _innerGlowFade._IsShown ? FontManager::FindPlacedGlyphInnerGlow(placed, fadingInnerGlowWidth, fadingInnerGlowOffset) : nullptr };

            if(fadingInnerGlow)
                appendGlyphQuad(*fadingInnerGlow, pen, scale, getLineCenter(line), &_innerGlowFade._Mesh);
        }

        //A boundary sits where its character starts: the left edge of its cluster, the right edge right to left. The
        //characters inside a cluster (an emoji sequence, a ligature) sit at the cluster's end, and the line ends at its
        //end as it reads: the right end, or the left end of a right-to-left line
        uSize clusterStart { 0 };

        for(uSize character = 0; character < lineLength; ++character)
        {
            if(0.0f <= clusterSpans[character].x)
                clusterStart = character;

            const glm::vec2 span { 0.0f <= clusterSpans[clusterStart].x ? clusterSpans[clusterStart] : glm::vec2(0.0f) };
            const bool isStart { clusterStart == character };
            const bool isRightToLeft { isClusterRightToLeft[clusterStart] };
            _glyphPositions.push_back(isStart != isRightToLeft ? span.x : span.y);
            _glyphLines.push_back(line);
            _characterSpans.push_back(span);
            _isInsideCluster.push_back(!isStart);
        }

        //The first boundary of a later line is the one after its '\n'
        const bool isLineRightToLeft { line < _isLineRightToLeft.size() && _isLineRightToLeft[line] };
        _glyphPositions.push_back(isLineRightToLeft ? lineLeft : along);
        _glyphLines.push_back(line);
        _characterSpans.push_back(glm::vec2(_glyphPositions.back()));
        _isInsideCluster.push_back(false);
        lineStart = lineEnd + 1;
    }

    _mesh._IsChanged = true;
    _outlineMesh._IsChanged = true;
    _shadowMesh._IsChanged = true;
    _glowMesh._IsChanged = true;
    _innerGlowMesh._IsChanged = true;
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

f32 RetroFuturaGUI::Text::getLineStart(const uSize line) const
{
    if(TextFlow::Horizontal != _textFlow || line >= _isLineRightToLeft.size() || line >= _lineWidths.size())
        return 0.0f;

    return _isLineRightToLeft[line] ? std::round(_textSpan.x - _lineWidths[line]) : 0.0f;
}

f32 RetroFuturaGUI::Text::getLineCenter(const uSize line) const
{
    if(TextFlow::Horizontal != _textFlow)
        return getColumnLeft(line) + _lineHeight * 0.5f;

    return -static_cast<f32>(line) * _lineHeight + (_ascender - _descender) * 0.5f;
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

void RetroFuturaGUI::Text::SetMaskSpan(const MaskSpan maskSpan)
{
    _maskSpan = maskSpan;
}

RetroFuturaGUI::MaskSpan RetroFuturaGUI::Text::GetMaskSpan() const
{
    return _maskSpan;
}

void RetroFuturaGUI::Text::appendGlyphQuad(const Glyph& glyph, const glm::vec2& pen, const f32 scale, const f32 lineCenter, GlyphMesh* mesh) const
{
    const f32
        xPos { pen.x + glyph._Bearing[0] * scale },
        yPos { pen.y - (glyph._Size[1] - glyph._Bearing[1]) * scale },
        width { glyph._Size[0] * scale },
        height { glyph._Size[1] * scale };

    // Clipping calculations
    const f32
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

    const f32
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
        mesh->_Draws.push_back(GlyphDraw { ._TextureID = glyph._TextureID, ._VertexStart = static_cast<i32>(mesh->_Vertices.size() / GlyphMesh::_FloatsPerVertex), ._VertexCount = 0, ._IsColor = glyph._IsColor });

    //Two triangles, each corner's position, glyph page coordinates and its line's middle
    const f32
        left { xPos + clippedLocalLeft },
        right { left + clippedWidth },
        bottom { yPos + height - clippedLocalTop },
        top { yPos + height - clippedLocalBottom };
    const std::array<std::array<f32, GlyphMesh::_FloatsPerVertex>, 6> corners
    {{
        { left, bottom, uMin, vMax, lineCenter },
        { left, top, uMin, vMin, lineCenter },
        { right, top, uMax, vMin, lineCenter },
        { left, bottom, uMin, vMax, lineCenter },
        { right, top, uMax, vMin, lineCenter },
        { right, bottom, uMax, vMax, lineCenter }
    }};

    for(const std::array<f32, GlyphMesh::_FloatsPerVertex>& corner : corners)
        mesh->_Vertices.insert(mesh->_Vertices.end(), corner.begin(), corner.end());

    mesh->_Draws.back()._VertexCount += static_cast<i32>(corners.size());
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
        if(isInsideCluster(i))
            continue;

        const f32 distance { std::abs(_glyphPositions[i] - along) };

        if(distance < closestDistance)
        {
            closestDistance = distance;
            closest = i;
        }
    }

    return closest;
}

bool RetroFuturaGUI::Text::isInsideCluster(const uSize boundary) const
{
    return boundary < _isInsideCluster.size() && _isInsideCluster[boundary];
}

void RetroFuturaGUI::Text::alignPosition()
{
    //A horizontal text is placed by its first baseline, a vertical one by its top edge
    const f32
        parentTop { _position.y + _parentSize.y * 0.5f - _textPadding },
        parentBottom { _position.y - _parentSize.y * 0.5f + _textPadding };
    f32 originY { 0.0f };

    if(TextFlow::Horizontal != _textFlow)
    {
        switch(_anchor)
        {
            case TextAnchor::Top:
                originY = parentTop;
            break;
            case TextAnchor::Bottom:
                originY = parentBottom + _textSpan.y; //the longest column's end touches the bottom edge
            break;
            case TextAnchor::Center:
            default:
                originY = _position.y + _textSpan.y * 0.5f;
            break;
        }
    }
    else
    {
        switch(_anchor)
        {
            case TextAnchor::Top:
                originY = parentTop - _ascender; //the first line's highest glyphs touch the top edge
            break;
            case TextAnchor::Bottom:
                originY = parentBottom + _descender + static_cast<f32>(getLineCount() - 1) * _lineHeight; //the last line's lowest glyphs touch the bottom edge
            break;
            case TextAnchor::Center:
            default:
                originY = _position.y - (_ascender - _descender) * 0.5f; //the line from its lowest to its highest glyph centered
            break;
        }
    }

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

    //Each line shaped once here, for updateMesh to place: across, in runs of one direction; down a column, as it is
    const std::u32string& text { _text.GetUtf32() };
    _shapedLines.clear();
    _isLineRightToLeft.clear();

    for(uSize lineStart = 0; lineStart <= text.size();)
    {
        const uSize lineEnd { std::min(text.find(U'\n', lineStart), text.size()) };
        const std::u32string_view lineText { std::u32string_view(text).substr(lineStart, lineEnd - lineStart) };
        bool isRightToLeft { false };
        _shapedLines.emplace_back();

        if(TextFlow::Horizontal == _textFlow)
            shapeLine(lineText, &_shapedLines.back(), &isRightToLeft);
        else
            FontManager::ShapeText(*_fontInfo, _fontKey, lineText, ShapeDirection::TopToBottom, &_shapedLines.back());

        _isLineRightToLeft.push_back(isRightToLeft);
        lineStart = lineEnd + 1;
    }

    _lineWidths.clear();

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
        _lineWidths.push_back(lineLength);
    }

    //Columns side by side, each a line height wide, as long as the longest
    const f32 lineCount { static_cast<f32>(_shapedLines.size()) };
    _textSpan = TextFlow::Horizontal == _textFlow ? glm::vec2(longestLine, lineCount * _lineHeight) : glm::vec2(lineCount * _lineHeight, longestLine);
    FontManager::FlushGlyphUploads();
}

void RetroFuturaGUI::Text::shapeLine(std::u32string_view line, std::vector<PlacedGlyph>* glyphs, bool* isRightToLeft) const
{
    if(!glyphs)
        return;

    if(!isRightToLeft)
        return;

    glyphs->clear();
    *isRightToLeft = false;

    if(!_fontInfo || line.empty())
        return;

    //The line's direction from its first strong character, left to right when it has none
    const SBCodepointSequence sequence { .stringEncoding = SBStringEncodingUTF32, .stringBuffer = line.data(), .stringLength = line.size() };
    SBAlgorithmRef algorithm { SBAlgorithmCreate(&sequence) };
    SBParagraphRef paragraph { algorithm ? SBAlgorithmCreateParagraph(algorithm, 0, line.size(), SBLevelDefaultLTR) : nullptr };
    SBLineRef bidiLine { paragraph ? SBParagraphCreateLine(paragraph, 0, SBParagraphGetLength(paragraph)) : nullptr };

    if(!bidiLine)
    {
        FontManager::ShapeText(*_fontInfo, _fontKey, line, ShapeDirection::LeftToRight, glyphs);
        SBParagraphRelease(paragraph);
        SBAlgorithmRelease(algorithm);
        return;
    }

    //The runs come in the order they're drawn; an odd level reads right to left
    *isRightToLeft = 0 != (SBParagraphGetBaseLevel(paragraph) & 1);
    const SBRun* runs { SBLineGetRunsPtr(bidiLine) };
    std::vector<PlacedGlyph> runGlyphs {};

    for(SBUInteger run = 0; run < SBLineGetRunCount(bidiLine); ++run)
    {
        const ShapeDirection direction { 0 != (runs[run].level & 1) ? ShapeDirection::RightToLeft : ShapeDirection::LeftToRight };
        FontManager::ShapeText(*_fontInfo, _fontKey, line.substr(runs[run].offset, runs[run].length), direction, &runGlyphs);

        for(PlacedGlyph& glyph : runGlyphs)
            glyph._Cluster += static_cast<u32>(runs[run].offset);

        glyphs->insert(glyphs->end(), runGlyphs.begin(), runGlyphs.end());
    }

    SBLineRelease(bidiLine);
    SBParagraphRelease(paragraph);
    SBAlgorithmRelease(algorithm);
}

uSize RetroFuturaGUI::Text::GetBoundaryBeside(const uSize boundary, const CaretMove move) const
{
    if(_glyphPositions.empty())
        return 0;

    const uSize
        glyphCount { GetGlyphCount() },
        current { std::min(boundary, glyphCount) },
        line { _glyphLines[current] },
        lastLine { _glyphLines.back() };
    const f32 along { _glyphPositions[current] };

    //Down a column a step is a character; across, the next column that way, as far down as the caret is
    if(TextFlow::Horizontal != _textFlow)
    {
        //Past the characters inside a cluster, which the caret never stops between
        if(CaretMove::Up == move)
        {
            uSize above { 0 < current ? current - 1 : 0 };

            while(isInsideCluster(above))
                --above;

            return above;
        }

        if(CaretMove::Down == move)
        {
            uSize below { std::min(current + 1, glyphCount) };

            while(isInsideCluster(below))
                ++below;

            return below;
        }

        const bool isTowardNextColumn { (CaretMove::Left == move) == (TextFlow::VerticalRightToLeft == _textFlow) };

        if(isTowardNextColumn)
            return line == lastLine ? current : getClosestOnLine(line + 1, along);

        return 0 == line ? current : getClosestOnLine(line - 1, along);
    }

    if(CaretMove::Up == move)
        return 0 == line ? 0 : getClosestOnLine(line - 1, along);

    if(CaretMove::Down == move)
        return line == lastLine ? glyphCount : getClosestOnLine(line + 1, along);

    //The nearest boundary that way as the line is drawn, the logically closest one where several sit together
    const bool isRightward { CaretMove::Right == move };
    const uSize
        first { static_cast<uSize>(std::lower_bound(_glyphLines.begin(), _glyphLines.end(), line) - _glyphLines.begin()) },
        last { GetLineLastBoundary(line) };
    uSize nearest { current };

    for(uSize candidate = first; candidate <= last; ++candidate)
    {
        const f32 candidateAlong { _glyphPositions[candidate] };

        if(isInsideCluster(candidate) || (isRightward ? candidateAlong <= along + _sameSpotDistance : candidateAlong >= along - _sameSpotDistance))
            continue;

        const bool isCloser { nearest == current
            || (isRightward ? candidateAlong < _glyphPositions[nearest] : candidateAlong > _glyphPositions[nearest])
            || (std::abs(candidateAlong - _glyphPositions[nearest]) <= _sameSpotDistance && std::max(candidate, current) - std::min(candidate, current) < std::max(nearest, current) - std::min(nearest, current)) };

        if(isCloser)
            nearest = candidate;
    }

    if(nearest != current)
        return nearest;

    //At the line's edge, on to the next line as the line reads, or back to the one before
    const bool isForward { isRightward != (line < _isLineRightToLeft.size() && _isLineRightToLeft[line]) };

    if(isForward)
        return line == lastLine ? current : last + 1;

    return 0 == line ? current : first - 1;
}

std::vector<RetroFuturaGUI::TextSelectionArea> RetroFuturaGUI::Text::GetSelectionAreas(const uSize first, const uSize last, const f32 caretSize) const
{
    std::vector<TextSelectionArea> areas {};

    if(_glyphPositions.empty())
        return areas;

    const uSize
        start { std::min(first, last) },
        end { std::min(std::max(first, last), GetGlyphCount()) };
    const f32 lineBreakWidth { _glyphSize.y * 0.3f }; //so a selected empty line still shows

    for(uSize line = GetBoundaryLine(start); start < end && line <= GetBoundaryLine(end); ++line)
    {
        //The stretches of the selected characters on this line, joined where they touch
        const uSize lineLast { GetLineLastBoundary(line) };
        std::vector<glm::vec2> spans {};

        for(uSize character = start; character < end; ++character)
        {
            if(_glyphLines[character] != line)
                continue;

            if(character != lineLast)
            {
                if(_characterSpans[character].y > _characterSpans[character].x)
                    spans.push_back(_characterSpans[character]);

                continue;
            }

            //The line break, at the line's end as it reads
            const f32 lineEnd { _glyphPositions[lineLast] };
            spans.push_back(line < _isLineRightToLeft.size() && _isLineRightToLeft[line] ? glm::vec2(lineEnd - lineBreakWidth, lineEnd) : glm::vec2(lineEnd, lineEnd + lineBreakWidth));
        }

        std::ranges::sort(spans, {}, [](const glm::vec2& span) { return span.x; });
        std::vector<glm::vec2> joined {};

        for(const glm::vec2& span : spans)
        {
            if(!joined.empty() && span.x <= joined.back().y + _sameSpotDistance)
                joined.back().y = std::max(joined.back().y, span.y);
            else
                joined.push_back(span);
        }

        //Across, a line height tall where the caret sits; in a column, the column's width, downward along it
        for(const glm::vec2& span : joined)
        {
            if(TextFlow::Horizontal == _textFlow)
            {
                areas.push_back(TextSelectionArea
                {
                    ._Center = _positionAligned + glm::vec2((span.x + span.y) * 0.5f, -static_cast<f32>(line) * _lineHeight + caretSize * 0.25f),
                    ._Size = glm::vec2(span.y - span.x, _lineHeight)
                });
                continue;
            }

            areas.push_back(TextSelectionArea
            {
                ._Center = _positionAligned + glm::vec2(getColumnLeft(line) + _lineHeight * 0.5f, -(span.x + span.y) * 0.5f),
                ._Size = glm::vec2(_lineHeight, span.y - span.x)
            });
        }
    }

    return areas;
}

void RetroFuturaGUI::Text::SetAnchor(const TextAnchor anchor)
{
    _anchor = anchor;
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

f32 RetroFuturaGUI::Text::GetGlyphSize() const
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