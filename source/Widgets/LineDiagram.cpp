#include "LineDiagram.hpp"
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

    _hoverText = std::make_unique<Text>(_elementProjection);
    _hoverText->SetTextAlignment(TextAlignment::Center);
    _hoverTextBackground = std::make_unique<Rectangle>(_elementProjection);
    _hoverTextBackground->SetRectangleMode(RectangleMode::Plane);
    _hoverTextBackground->SetColors(_hoverTextBackgroundColor);
    _hoverTextBackground->SetCornerRadii(glm::vec4(3.0f));
    SetIndicatorSize(glm::vec2(10.0f), ElementSizing::Pixels);
    SetIndicatorType(IndicatorType::Circle);
    SetIndicatorBorderWidth(2.0f);
}

void RetroFuturaGUI::LineDiagram::Draw()
{
    updatePoints(); // Before interact, so the cursor is tested against the lines
    interact();

    if (_background)
        _background->Draw();

    if (_border)
        _border->Draw();

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

void RetroFuturaGUI::LineDiagram::SetLabelFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _labelFontFamily = fontFamily;
    _labelFontSize = fontSize;
    _labelFontSlant = slant;
    _labelFontWeight = fontWeight;

    for(const std::unique_ptr<Text>& label : _labels)
        applyLabelFont(label.get());

    applyLabelFont(_hoverText.get());
}

void RetroFuturaGUI::LineDiagram::ShowLineLabels(const bool value)
{
    _showLineLabels = value;
}

void RetroFuturaGUI::LineDiagram::ShowHoverLineLabel(const bool value)
{
    _showHoverLineLabel = value;
}

void RetroFuturaGUI::LineDiagram::ShowHoverValue(const bool value)
{
    _showHoverValue = value;
}

void RetroFuturaGUI::LineDiagram::SetHoverTextBackgroundColor(const glm::vec4 color)
{
    if(!_hoverTextBackground)
        return;

    _hoverTextBackgroundColor = { color };
    _hoverTextBackground->SetColors(_hoverTextBackgroundColor);
}

void RetroFuturaGUI::LineDiagram::alignElementsToTrack()
{
    growLinesTo(_dataCount.size());

    const Rectangle* track { getTrack() };

    if(!track)
        return;

    const f32 inset { (_border ? _border->GetBorderWidth() * 2.0f : 0.0f) + _lineThickness + _hoverThicknessIncrease };
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

void RetroFuturaGUI::LineDiagram::alignLine(const uSize lineIndex, const Rectangle& track)
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

    // A shorter set either ends where its data does, or holds its last value up to the end of the track
    const uSize
        dataCount { _dataCount[lineIndex] },
        pointCount { _clampToLastValue ? _maxDataCount : dataCount };

    if(2 > pointCount)
        return;

    std::vector<glm::vec2>& points { _points[lineIndex] };
    const f32 xStep { 1.0f / static_cast<f32>(_maxDataCount - 1) };

    for(uSize i = 0; i < pointCount; ++i)
    {
        // A value beyond the min/max range is drawn at the track's 
        const f32 value { getDataValueAsF32(lineIndex, (std::min)(i, dataCount - 1)) };
        points.emplace_back(static_cast<f32>(i) * xStep - 0.5f, std::clamp(getRangeFraction(value), 0.0f, 1.0f) - 0.5f);
    }
}

void RetroFuturaGUI::LineDiagram::drawGraph()
{
    if(!_useGraph)
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
    if(!_useGraph || !_showLineLabels)
        return;

    const Rectangle* track { getTrack() };

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

void RetroFuturaGUI::LineDiagram::drawLineLabel(const uSize lineIndex, const Rectangle& track)
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
    const f32
        halfWidth { label.GetWidth() * 0.5f },
        halfHeight { label.GetGlyphSize() * 0.5f },
        gap { _lineThickness * 0.5f + _labelGap },
        aboveY { start.y + gap + halfHeight },
        centerY { aboveY + halfHeight > track.GetSize().y * 0.5f ? start.y - gap - halfHeight : aboveY }; // below the line where above would leave the track

    // a label's left edge lines up with the line's first point and sits just above that point.
    // Text is centered and therefore offsetted by half of its width
    label.SetColor(getLineColor(lineIndex));
    label.SetRotation(track.GetRotation());
    label.SetPosition(toWorldSpace(glm::vec2(start.x + halfWidth, centerY), track, 0.025f));
    label.Draw();
}

void RetroFuturaGUI::LineDiagram::drawHoverPointer()
{
    if(!_lineHovered)
        return;

    const Rectangle* track { getTrack() };

    if(!track)
        return;

    if(_hoveredLineIndex >= _points.size() || _hoveredPointIndex >= _points[_hoveredLineIndex].size())
        return;

    const glm::vec2 pointerPosition { _points[_hoveredLineIndex][_hoveredPointIndex] * _plotSize };
    drawHoverMarker(pointerPosition, *track);
    drawHoverText(pointerPosition, *track);
}

void RetroFuturaGUI::LineDiagram::drawHoverMarker(const glm::vec2& pointerPosition, const Rectangle& track)
{
    if(!_useIndicator)
        return;

    if(!_indicatorBackground)
        return;

    if(!_indicatorBorder)
        return;

    const glm::vec3 position { toWorldSpace(pointerPosition, track, 0.03f) };
    _indicatorBackground->SetPosition(position);
    _indicatorBackground->SetRotation(track.GetRotation());
    _indicatorBorder->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.005f));
    _indicatorBorder->SetRotation(track.GetRotation());
    drawIndicator();
}

void RetroFuturaGUI::LineDiagram::drawHoverText(const glm::vec2& pointerPosition, const Rectangle& track)
{
    if(!_showHoverLineLabel && !_showHoverValue)
        return;

    if(!_hoverText)
        return;

    if(!_hoverTextBackground)
        return;

    if(!_indicatorBackground)
        return;

    const std::string text { composeHoverText() };

    if(text != _hoverText->GetTextUTF8())
    {
        _hoverText->SetTextUTF8(text);
        fitClipBox(*_hoverText);
    }

    const f32 glyphSize { _hoverText->GetGlyphSize() };
    const glm::vec2
        boxSize { _hoverText->GetWidth() + _hoverTextPadding * 2.0f, glyphSize * 1.1f + _hoverTextPadding * 2.0f },
        textOffset { 0.0f, glyphSize * 0.15f },
        pointerSize { _useIndicator ? _indicatorBackground->GetSize() : glm::vec2(0.0f) },
        halfTrack { track.GetSize() * 0.5f };
    const f32
        gap { (glm::max)((glm::max)(pointerSize.x, pointerSize.y), _lineThickness + _hoverThicknessIncrease) * 0.5f + _labelGap },
        rightX { pointerPosition.x + gap + boxSize.x * 0.5f },
        aboveY { pointerPosition.y + gap + boxSize.y * 0.5f },
        centerX { rightX + boxSize.x * 0.5f > halfTrack.x ? pointerPosition.x - gap - boxSize.x * 0.5f : rightX }, // left of the pointer where right would leave the track
        centerY { aboveY + boxSize.y * 0.5f > halfTrack.y ? pointerPosition.y - gap - boxSize.y * 0.5f : aboveY }; // below it where above would

    const glm::vec2 boxCenter { centerX, centerY };
    _hoverTextBackground->SetSize(boxSize);
    _hoverTextBackground->SetRotation(track.GetRotation());
    _hoverTextBackground->SetPosition(toWorldSpace(boxCenter, track, 0.04f));
    _hoverTextBackground->Draw();

    _hoverText->SetColor(getLineColor(_hoveredLineIndex));
    _hoverText->SetRotation(track.GetRotation());
    _hoverText->SetPosition(toWorldSpace(boxCenter + textOffset, track, 0.045f));
    _hoverText->Draw();
}

std::string RetroFuturaGUI::LineDiagram::composeHoverText() const
{
    if(_hoveredLineIndex >= _dataCount.size())
        return {};

    if(0 == _dataCount[_hoveredLineIndex])
        return {};

    // A point past the end of a shorter set holds its last value
    const uSize dataIndex { (std::min)(_hoveredPointIndex, _dataCount[_hoveredLineIndex] - 1) };
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

glm::vec4 RetroFuturaGUI::LineDiagram::getLineColor(const uSize lineIndex) const
{
    if(lineIndex >= _lines.size())
        return glm::vec4(1.0f);

    if(!_lines[lineIndex])
        return glm::vec4(1.0f);

    return _lines[lineIndex]->GetColor();
}

void RetroFuturaGUI::LineDiagram::applyLabelFont(Text* text) const
{
    if(!text)
        return;

    if(_labelFontFamily.empty())
        return;

    text->SetFontFamily(_labelFontFamily, _labelFontSize, _labelFontSlant, _labelFontWeight);
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

    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer

    const glm::vec2 mousePos { pointer._Position };
    const bool canHover { _isEnabledFlag && _useGraph && pointer._HasPosition && isPointInside(mousePos) };

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
    const Rectangle* track { getTrack() };

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
    const Rectangle* track { getTrack() };

    if(!track)
        return 0;

    if(lineIndex >= _points.size() || 2 > _points[lineIndex].size())
        return 0;

    if(0.0f == _plotSize.x)
        return 0;

    // The points sit 1 / (_maxDataCount - 1) of the plot apart, from its left edge at -0.5 to its right at 0.5
    const f32
        plotFraction { toTrackSpace(point, *track).x / _plotSize.x + 0.5f },
        nearest { std::round(plotFraction * static_cast<f32>(_maxDataCount - 1)) };

    return (std::min)(static_cast<uSize>((std::max)(nearest, 0.0f)), _points[lineIndex].size() - 1);
}

glm::vec2 RetroFuturaGUI::LineDiagram::toTrackSpace(const glm::vec2& point, const Rectangle& track)
{
    const glm::vec2 offset { point - glm::vec2(track.GetPosition()) };
    const f32 radians { glm::radians(-track.GetRotation().z) };

    return glm::vec2
    (
        offset.x * cos(radians) - offset.y * sin(radians),
        offset.x * sin(radians) + offset.y * cos(radians)
    );
}

glm::vec3 RetroFuturaGUI::LineDiagram::toWorldSpace(const glm::vec2& trackPoint, const Rectangle& track, const f32 zOffset)
{
    const f32 radians { glm::radians(track.GetRotation().z) };
    const glm::vec2 rotated
    (
        trackPoint.x * cos(radians) - trackPoint.y * sin(radians),
        trackPoint.x * sin(radians) + trackPoint.y * cos(radians)
    );

    return track.GetPosition() + glm::vec3(rotated, zOffset);
}