#include "SeparatorLine.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Text.hpp"
#include <algorithm>
#include <memory>

RetroFuturaGUI::SeparatorLine::SeparatorLine(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::SeparatorLine;
    useFlavors(&_background, nullptr, &_text);
    _text.SetTransitionSettings(&_transitionSettings);
    _background.SetTransitionSettings(&_transitionSettings);
}

void RetroFuturaGUI::SeparatorLine::Draw()
{
    _background.Draw();

    if(!_showText)
        return;

    _text.Draw();
}

void RetroFuturaGUI::SeparatorLine::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    _background.SetSize(size);
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    _background.SetPosition(position);
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::SetTextEdgePadding(const f32 padding)
{
    _textEdgePadding = padding;
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::ShowText(const bool enable)
{
    _showText = enable;
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::updateLayout()
{
    if(!_showText || _text.GetString().empty()) //if no label, draw a solid line
    {
        _background.SetGaps(BackgroundGap { ._Offset = 0.0f, ._Length = 0.0f, ._Degree = 0.0f, ._Repeat = 0 });
        return;
    }

    //The text and its padding, but never more than the line, nor the edge padding more than the line leaves beside the gap
    const f32
        lineWidth { std::max(_size.x, 0.0f) },
        gapLength { std::min(_text.GetWidth() + _text.GetPadding() * 2.0f, lineWidth) },
        edgePadding { std::clamp(_textEdgePadding, 0.0f, lineWidth - gapLength) };
    f32 gapStart { edgePadding }; //from the line's left end

    switch(_text.GetAlignment())
    {
        case TextAlignment::Right:
            gapStart = lineWidth - gapLength - edgePadding;
        break;
        case TextAlignment::Center:
            gapStart = (lineWidth - gapLength) * 0.5f;
        break;
        case TextAlignment::Left:
        default:
        break;
    }

    _background.SetGaps(BackgroundGap { ._Offset = gapStart, ._Length = gapLength, ._Degree = 0.0f, ._Repeat = 1 });

    //The gap is the text's box, so its own alignment places it there, and a caption longer than the line is cut at the line's ends
    _text.SetSize(glm::vec2(gapLength, std::max<f32>(_size.y, _text.GetElement().GetSpanHeight())));
    _text.SetPosition(glm::vec3(_position.x - lineWidth * 0.5f + gapStart + gapLength * 0.5f, _position.y, _position.z + 0.01f));
}

RetroFuturaGUI::Background& RetroFuturaGUI::SeparatorLine::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::SeparatorLine::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::SeparatorLine::GetText()
{
    return _text;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::SeparatorLine::GetText() const
{
    return _text;
}