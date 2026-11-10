#include "LineDiagram.hpp"
#include "FillStyle.hpp"
#include "config.hpp"
#include "Input.hpp"
#include "PlatformBridge.hpp"
#include <algorithm>
#include <cmath>
#include <format>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::LineDiagram::LineDiagram(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IRangedMultiValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::LineDiagram;

    if(!_elementProjection)
        return;

    _hoverText.SetTransitionSettings(&_transitionSettings);
    useSubPartFlavors(_hoverText.GetBackground(), StylePart::HoverTextBackground);
    useSubPartFlavors(_hoverText.GetText(), StylePart::HoverTextText);
    _indicator.SetSize(glm::vec2(10.0f), ElementSizing::Pixels);
    _indicator.SetType(IndicatorType::Circle);
    _indicator.GetBorder().SetBuiltInWidth(styleKey(), 2.0f);
}

void RetroFuturaGUI::LineDiagram::Draw()
{
    updatePoints(); //again, since the Update loop may have handed over new data after Interact (just like a playback its frequency bands)
    _background.Draw();
    _border.Draw();
    drawGraph();
    drawLineLabels();
    drawHoverPointer();
}

void RetroFuturaGUI::LineDiagram::SetLabel(const uSize index, std::string_view labelText)
{
    growLinesTo(index + 1);
    alignElementsToTrack();

    if(index >= _labels.size())
        return;

    if(!_labels[index])
        return;

    _labels[index]->SetTextUTF8(labelText);
    fitClipBox(*_labels[index]);
}

void RetroFuturaGUI::LineDiagram::SetLineColor(const uSize lineIndex, const glm::vec4 color)
{
    growLinesTo(lineIndex + 1);
    alignElementsToTrack();

    if(lineIndex >= _lines.size())
        return;

    if(!_lines[lineIndex])
        return;

    _lines[lineIndex]->SetColor(color);
}

void RetroFuturaGUI::LineDiagram::SetLineThickness(const f32 thickness)
{
    _lineThickness = thickness;
    alignElementsToTrack();
}

void RetroFuturaGUI::LineDiagram::SetLabelFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _labelFontFamily = fontFamily;
    _labelFontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _labelFontSlant = slant;
    _labelFontWeight = fontWeight;

    for(const std::unique_ptr<Text>& label : _labels)
        applyLabelFont(label.get());

    applyLabelFont(&_hoverText.GetText().GetElement());
}

f32 RetroFuturaGUI::LineDiagram::GetLabelFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_labelFontSize, sizeUnit);
}

void RetroFuturaGUI::LineDiagram::ShowLineLabels(const bool show)
{
    _showLineLabels = show;
}

void RetroFuturaGUI::LineDiagram::EnableHoverLineLabel(const bool enable)
{
    _showHoverLineLabel = enable;
}

void RetroFuturaGUI::LineDiagram::EnableHoverValue(const bool enable)
{
    _showHoverValue = enable;
}

RetroFuturaGUI::HoverText& RetroFuturaGUI::LineDiagram::GetHoverText()
{
    return _hoverText;
}

const RetroFuturaGUI::HoverText& RetroFuturaGUI::LineDiagram::GetHoverText() const
{
    return _hoverText;
}

void RetroFuturaGUI::LineDiagram::alignElementsToTrack()
{
    growLinesTo(_dataCount.size());

    const Background* track { getTrack() };

    if(!track)
        return;

    const f32 inset { ( _border.GetWidth() * 2.0f) + _lineThickness + _hoverThicknessIncrease };
    _plotSize = (glm::max)(track->GetSize() - glm::vec2(inset), glm::vec2(0.0f)); // parenthesized: glfw3native.h brings in windows.h and its max macro

    for(uSize lineIndex = 0; lineIndex < _lines.size(); ++lineIndex)
        alignLine(lineIndex, *track);
}

void RetroFuturaGUI::LineDiagram::growLinesTo(const uSize lineCount)
{
    if(!_elementProjection)
        return;

    while(_lines.size() < lineCount)
        _lines.push_back(std::make_unique<Polyline>(_elementProjection));

    while(_labels.size() < _lines.size())
    {
        std::unique_ptr<Text> label { std::make_unique<Text>(_elementProjection) };
        label->SetTextAlignment(TextAlignment::Center);
        applyLabelFont(label.get());
        _labels.push_back(std::move(label));
    }

    _points.resize(_lines.size());
}

void RetroFuturaGUI::LineDiagram::alignLine(const uSize lineIndex, const Background& track)
{
    if(lineIndex >= _lines.size())
        return;

    if(!_lines[lineIndex])
        return;

    _lines[lineIndex]->SetPosition(track.GetPosition() + glm::vec3(0.0f, 0.0f, 0.02f));
    _lines[lineIndex]->SetSize(glm::vec3(_plotSize, 1.0f));
    _lines[lineIndex]->SetRotation(track.GetRotation());
}

void RetroFuturaGUI::LineDiagram::updatePoints()
{
    // A set that holds too few values draws no line, so every line starts out empty
    for(std::vector<glm::vec2>& points : _points)
        points.clear();

    for(uSize lineIndex = 0; lineIndex < _points.size(); ++lineIndex)
        updateLinePoints(lineIndex);
}

void RetroFuturaGUI::LineDiagram::updateLinePoints(const uSize lineIndex)
{
    if(lineIndex >= _points.size() || lineIndex >= _dataCount.size())
        return;

    if(0 == _dataCount[lineIndex])
        return;

    // Stretched, every line spans the track by its own length. Spaced by the longest set, a shorter one either ends where its
    // data does or holds its last value up to the end of the track
    const bool isStretched { LineSpacing::Stretched == _lineSpacing };
    const uSize
        dataCount { _dataCount[lineIndex] },
        pointCount { !isStretched && _clampToLastValue ? _maxDataCount : dataCount },
        spacingCount { isStretched ? dataCount : _maxDataCount };

    if(2 > pointCount)
        return;

    std::vector<glm::vec2>& points { _points[lineIndex] };
    const f32 xStep { 1.0f / static_cast<f32>(spacingCount - 1) };

    for(uSize i = 0; i < pointCount; ++i)
    {
        // A value beyond the min/max range is drawn at the track's 
        const f32 value { getDataValueAsF32(lineIndex, std::min(i, dataCount - 1)) };
        points.emplace_back(static_cast<f32>(i) * xStep - 0.5f, std::clamp(getRangeFraction(value), 0.0f, 1.0f) - 0.5f);
    }
}

void RetroFuturaGUI::LineDiagram::drawGraph()
{
    if(!_graph.IsShown())
        return;

    for(uSize lineIndex = 0; lineIndex < _lines.size(); ++lineIndex)
    {
        // hovered line is drawn last, so it lies over the others
        if(_lineHovered && lineIndex == _hoveredLineIndex)
            continue;

        drawLine(lineIndex, _lineThickness);
    }

    if(_lineHovered)
        drawLine(_hoveredLineIndex, _lineThickness + _hoverThicknessIncrease);
}

void RetroFuturaGUI::LineDiagram::drawLine(const uSize lineIndex, const f32 thickness)
{
    if(lineIndex >= _lines.size() || lineIndex >= _points.size())
        return;

    if(!_lines[lineIndex])
        return;

    _lines[lineIndex]->SetThickness(thickness);
    _lines[lineIndex]->SetPoints(_points[lineIndex]);
    _lines[lineIndex]->Draw();
}

void RetroFuturaGUI::LineDiagram::drawLineLabels()
{
    if(!_graph.IsShown() || !_showLineLabels)
        return;

    const Background* track { getTrack() };

    if(!track)
        return;

    // Labels of lines that start together overlap at the same depth. GL_LEQUAL lets a later one draw over an earlier one
    GLint depthFunction { GL_LESS };
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
    glDepthFunc(GL_LEQUAL);

    for(uSize lineIndex = 0; lineIndex < _labels.size(); ++lineIndex)
        drawLineLabel(lineIndex, *track);

    glDepthFunc(static_cast<GLenum>(depthFunction));
}

void RetroFuturaGUI::LineDiagram::drawLineLabel(const uSize lineIndex, const Background& track)
{
    if(lineIndex >= _labels.size() || lineIndex >= _points.size())
        return;

    if(!_labels[lineIndex])
        return;

    // A line too short to draw has no start to label
    if(2 > _points[lineIndex].size())
        return;

    Text& label { *_labels[lineIndex] };
    const glm::vec2 start { _points[lineIndex].front() * _plotSize };
    const glm::vec2 extents { getUprightExtents(glm::vec2(label.GetWidth() * 0.5f, label.GetGlyphSize() * 0.5f), track) };
    const f32
        gap { _lineThickness * 0.5f + _labelGap },
        aboveY { start.y + gap + extents.y },
        centerY { aboveY + extents.y > track.GetSize().y * 0.5f ? start.y - gap - extents.y : aboveY }; // below the line where above would leave the track

    // a label's left edge lines up with the line's first point and sits just above that point.
    // Text is centered and therefore offsetted by half of its width
    label.SetColors(getLineColors(lineIndex));
    label.SetRotation(glm::vec3(0.0f)); //upright whichever way the track runs; the widget's own turn is in its frame
    label.SetPosition(toWorldSpace(glm::vec2(start.x + extents.x, centerY), track, 0.025f));
    label.Draw();
}

void RetroFuturaGUI::LineDiagram::drawHoverPointer()
{
    if(!_lineHovered)
        return;

    const Background* track { getTrack() };

    if(!track)
        return;

    if(_hoveredLineIndex >= _points.size() || _hoveredPointIndex >= _points[_hoveredLineIndex].size())
        return;

    const glm::vec2 pointerPosition { _points[_hoveredLineIndex][_hoveredPointIndex] * _plotSize };
    drawHoverMarker(pointerPosition, *track);
    drawHoverText(pointerPosition, *track);
}

void RetroFuturaGUI::LineDiagram::drawHoverMarker(const glm::vec2& pointerPosition, const Background& track)
{
    if(!_indicator.IsShown())
        return;

    _indicator.Place(toWorldSpace(pointerPosition, track, 0.03f), track.GetRotation());
    drawIndicator();
}

void RetroFuturaGUI::LineDiagram::drawHoverText(const glm::vec2& pointerPosition, const Background& track)
{
    if(!_showHoverLineLabel && !_showHoverValue)
        return;

    Text& hoverText { _hoverText.GetText().GetElement() };
    const std::string text { composeHoverText() };

    if(text != hoverText.GetTextUTF8())
    {
        hoverText.SetTextUTF8(text);
        fitClipBox(hoverText);
    }

    const f32 glyphSize { hoverText.GetGlyphSize() };
    const glm::vec2
        boxSize { hoverText.GetWidth() + _hoverTextPadding * 2.0f, glyphSize * 1.1f + _hoverTextPadding * 2.0f },
        textOffset { 0.0f, glyphSize * 0.15f },
        pointerSize { _indicator.IsShown() ? _indicator.GetDrawnSize() : glm::vec2(0.0f) },
        halfTrack { track.GetSize() * 0.5f },
        extents { getUprightExtents(boxSize * 0.5f, track) }; //the upright box's reach along the track
    const f32
        gap { (glm::max)((glm::max)(pointerSize.x, pointerSize.y), _lineThickness + _hoverThicknessIncrease) * 0.5f + _labelGap },
        rightX { pointerPosition.x + gap + extents.x },
        aboveY { pointerPosition.y + gap + extents.y },
        centerX { rightX + extents.x > halfTrack.x ? pointerPosition.x - gap - extents.x : rightX }, // left of the pointer where right would leave the track
        centerY { aboveY + extents.y > halfTrack.y ? pointerPosition.y - gap - extents.y : aboveY }; // below it where above would

    const glm::vec2 boxCenter { centerX, centerY };
    Background& box { _hoverText.GetBackground() };
    box.SetSize(boxSize);
    const glm::vec3 boxPosition { toWorldSpace(boxCenter, track, 0.04f) };
    box.SetRotation(glm::vec3(0.0f)); //upright, as the labels
    box.SetPosition(boxPosition);
    box.Draw();

    if(_hoverText.IsLineColorEnabled())
        hoverText.SetColors(getLineColors(_hoveredLineIndex));

    hoverText.SetRotation(glm::vec3(0.0f));
    hoverText.SetPosition(boxPosition + glm::vec3(textOffset, 0.005f)); //the offset is upright too
    hoverText.Draw();
}

std::string RetroFuturaGUI::LineDiagram::composeHoverText() const
{
    if(_hoveredLineIndex >= _dataCount.size())
        return {};

    if(0 == _dataCount[_hoveredLineIndex])
        return {};

    // A point past the end of a shorter set holds its last value
    const uSize dataIndex { std::min(_hoveredPointIndex, _dataCount[_hoveredLineIndex] - 1) };
    const std::string
        label { _showHoverLineLabel ? getLineLabelText(_hoveredLineIndex) : std::string {} },
        value { _showHoverValue ? getDataValueAsString(_hoveredLineIndex, dataIndex) : std::string {} };

    if(label.empty())
        return value;

    if(value.empty())
        return label;

    return std::format("{}: {}", label, value);
}

std::string RetroFuturaGUI::LineDiagram::getLineLabelText(const uSize lineIndex) const
{
    if(lineIndex >= _labels.size())
        return {};

    if(!_labels[lineIndex])
        return {};

    return _labels[lineIndex]->GetTextUTF8();
}

std::span<const glm::vec4> RetroFuturaGUI::LineDiagram::getLineColors(const uSize lineIndex) const
{
    static const glm::vec4 noLine { 1.0f };

    if(lineIndex >= _lines.size())
        return std::span<const glm::vec4>(&noLine, 1);

    if(!_lines[lineIndex])
        return std::span<const glm::vec4>(&noLine, 1);

    return std::span<const glm::vec4>(&_lines[lineIndex]->GetColor(), 1);
}

void RetroFuturaGUI::LineDiagram::applyLabelFont(Text* text) const
{
    if(!text)
        return;

    if(_labelFontFamily.empty())
        return;

    text->SetFontFamily(_labelFontFamily, _labelFontSize, FontSizeUnit::Pixels, _labelFontSlant, _labelFontWeight);
    const std::string content { text->GetTextUTF8() };
    text->SetTextUTF8(content);
    fitClipBox(*text);
}

void RetroFuturaGUI::LineDiagram::fitClipBox(Text& text)
{
    // Centered text sits around its position with the baseline half a glyph below it - twice the glyph height leaves room for ascenders and descenders
    text.SetParentSize(glm::vec2(text.GetWidth() + 2.0f, text.GetGlyphSize() * 2.0f));
}

void RetroFuturaGUI::LineDiagram::interact()
{
    if(!_parentWindow)
        return;

    updatePoints(); //so the cursor is tested against the lines of the current data
    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer

    const glm::vec2 mousePos { pointer._Position };
    const bool canHover { _isEnabledFlag && _graph.IsShown() && pointer._HasPosition && isPointInside(mousePos) };

    if(!canHover)
    {
        _lineHovered = false;
        return;
    }

    if(!_lineHovered || !isPointOnLine(mousePos, _hoveredLineIndex, _lineThickness + _hoverThicknessIncrease))
        _lineHovered = findHoveredLine(mousePos, _hoveredLineIndex);

    if(_lineHovered)
        _hoveredPointIndex = nearestPointIndex(mousePos, _hoveredLineIndex);
}

bool RetroFuturaGUI::LineDiagram::findHoveredLine(const glm::vec2& point, uSize& lineIndex) const
{
    for(uSize i = _points.size(); 0 < i; --i)
    {
        if(!isPointOnLine(point, i - 1, _lineThickness))
            continue;

        lineIndex = i - 1;
        return true;
    }

    return false;
}

bool RetroFuturaGUI::LineDiagram::isPointOnLine(const glm::vec2& point, const uSize lineIndex, const f32 thickness) const
{
    const Background* track { getTrack() };

    if(!track)
        return false;

    if(lineIndex >= _points.size())
        return false;

    const std::vector<glm::vec2>& points { _points[lineIndex] };

    if(2 > points.size())
        return false;

    // In the track's frame the points only need scaling
    const glm::vec2 trackPoint { toTrackSpace(point, *track) };
    const f32 radius { thickness * 0.5f };

    for(uSize i = 1; i < points.size(); ++i)
    {
        const glm::vec2
            start { points[i - 1] * _plotSize },
            segment { points[i] * _plotSize - start };
        const f32
            segmentLengthSquared { glm::dot(segment, segment) },
            along { 0.0f == segmentLengthSquared ? 0.0f : std::clamp(glm::dot(trackPoint - start, segment) / segmentLengthSquared, 0.0f, 1.0f) };

        if(radius >= glm::distance(trackPoint, start + along * segment))
            return true;
    }

    return false;
}

uSize RetroFuturaGUI::LineDiagram::nearestPointIndex(const glm::vec2& point, const uSize lineIndex) const
{
    const Background* track { getTrack() };

    if(!track)
        return 0;

    if(lineIndex >= _points.size() || 2 > _points[lineIndex].size())
        return 0;

    if(0.0f == _plotSize.x)
        return 0;

    // The points sit 1 / (spacing count - 1) of the plot apart, from its left edge at -0.5 to its right at 0.5: the longest set's
    // count, or the line's own when stretched
    const uSize spacingCount { LineSpacing::Stretched == _lineSpacing ? _points[lineIndex].size() : _maxDataCount };
    const f32
        plotFraction { toTrackSpace(point, *track).x / _plotSize.x + 0.5f },
        nearest { std::round(plotFraction * static_cast<f32>(spacingCount - 1)) };

    return std::min(static_cast<uSize>(std::max(nearest, 0.0f)), _points[lineIndex].size() - 1);
}

glm::vec2 RetroFuturaGUI::LineDiagram::toTrackSpace(const glm::vec2& point, const Background& track)
{
    const glm::vec2 offset { point - glm::vec2(track.GetPosition()) };
    const f32 radians { glm::radians(-track.GetRotation().z) };

    return glm::vec2
    (
        offset.x * cos(radians) - offset.y * sin(radians),
        offset.x * sin(radians) + offset.y * cos(radians)
    );
}

glm::vec3 RetroFuturaGUI::LineDiagram::toWorldSpace(const glm::vec2& trackPoint, const Background& track, const f32 zOffset)
{
    const f32 radians { glm::radians(track.GetRotation().z) };
    const glm::vec2 rotated
    (
        trackPoint.x * cos(radians) - trackPoint.y * sin(radians),
        trackPoint.x * sin(radians) + trackPoint.y * cos(radians)
    );

    return track.GetPosition() + glm::vec3(rotated, zOffset);
}
void RetroFuturaGUI::LineDiagram::SetLineSpacing(const LineSpacing spacing)
{
    _lineSpacing = spacing;
}

RetroFuturaGUI::LineSpacing RetroFuturaGUI::LineDiagram::GetLineSpacing() const
{
    return _lineSpacing;
}

glm::vec2 RetroFuturaGUI::LineDiagram::getUprightExtents(const glm::vec2& halfSize, const Background& track)
{
    const f32
        radians { glm::radians(track.GetRotation().z) },
        cosine { std::abs(std::cos(radians)) },
        sine { std::abs(std::sin(radians)) };

    return glm::vec2(halfSize.x * cosine + halfSize.y * sine, halfSize.x * sine + halfSize.y * cosine);
}