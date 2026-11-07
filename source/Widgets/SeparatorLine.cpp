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
    _text.SetTransitionSettings(&_transitionSettings);
    _text.SetChangeHook([this](const WidgetTextChange) { updateLayout(); }); //the text's size is the gap's
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

void RetroFuturaGUI::SeparatorLine::SetTextLeftPadding(const f32 padding)
{
    _textLeftPadding = padding;
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::ShowText(const bool enable)
{
    _showText = enable;
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::updateLayout()
{
    _text.SetSize(glm::vec2(_size.x, std::max<f32>(_size.y, _text.GetElement().GetSpanHeight())));

    if(!_showText || _text.GetString().empty()) //if no label, draw a solid line
    {
        _background.SetGaps(BackgroundGap { ._Repeat = 0 });
        return;
    }

    f32
        textPadding { _text.GetPadding() },
        textWidth { _text.GetWidth() };

    BackgroundGap textGap {
        ._Offset = _textLeftPadding,
        ._Length = textWidth + textPadding * 2.0f,
        ._Degree = 0.0f,
        ._Repeat = 1
    };

    _background.SetGaps(textGap);
    _text.SetAlignment(TextAlignment::Center);
    _text.SetPosition(glm::vec3(
        _position.x - _size.x * 0.5f + _textLeftPadding + textGap._Length * 0.5f,
        _position.y,
        _position.z + 0.01f)
    );
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