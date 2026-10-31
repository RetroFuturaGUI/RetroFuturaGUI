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
    _background = std::make_unique<Plane>(&_projection);
    _text = std::make_unique<Text>(&_projection);
    useTransitions({ _background.get(), _text.get() });

    if(!_background)
        return;

}

void RetroFuturaGUI::SeparatorLine::Draw()
{
    if(!_background)
        return;

    if(!_text)
        return;

    drawBackground();

    if(!_showText)
        return;

    _text->Draw();
}

void RetroFuturaGUI::SeparatorLine::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);

    if(!_background)
        return;

    _background->SetSize(size);
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);

    if(!_background)
        return;

    _background->SetPosition(position);
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::applyRotation(const glm::vec3& rotation)
{
    IWidget::applyRotation(rotation);

    if(!_background || !_text)
        return;

    _background->SetRotation(rotation);
    _text->SetRotation(rotation);
}

void RetroFuturaGUI::SeparatorLine::SetText(std::string_view text, const bool emitSignal)
{
    ITextProperties::SetText(text, emitSignal);
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    ITextProperties::SetFontFamily(fontFamily, fontSize, slant, fontWeight);
    updateLayout();
}

void RetroFuturaGUI::SeparatorLine::SetTextPadding(const f32 padding)
{
    ITextProperties::SetTextPadding(padding);
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
    if(!_text || !_background)
        return;

    _text->SetParentSize(glm::vec2(_size.x, std::max<f32>(_size.y, _text->GetSpanHeight())));

    if(!_showText || GetText().empty()) //if no label, draw a solid line
    {
        IBackground::SetBackgroundGaps(BackgroundGap { ._Repeat = 0 });
        return;
    }

    f32
        textPadding { _text->GetPadding() },
        textWidth { _text->GetWidth() };

    BackgroundGap textGap {
        ._Offset = _textLeftPadding,
        ._Length = textWidth + textPadding * 2.0f,
        ._Degree = 0.0f,
        ._Repeat = 1
    };

    IBackground::SetBackgroundGaps(textGap);
    _text->SetTextAlignment(TextAlignment::Center);
    _text->SetPosition(glm::vec3(
        _position.x - _size.x * 0.5f + _textLeftPadding + textGap._Length * 0.5f,
        _position.y,
        _position.z + 0.01f)
    );
}