#include "RadioButtonGroup.hpp"
#include "ITextProperties.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "RadioButton.hpp"
#include "Rectangle.hpp"
#include <algorithm>
#include <memory>

RetroFuturaGUI::RadioButtonGroup::RadioButtonGroup(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::RadioButtonGroup;
    _border = std::make_unique<Rectangle>(projection);
    _text = std::make_unique<Text>(projection);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::RadioButtonGroup::Draw()
{
    drawBorder();

    if(_text)
        _text->Draw();
}

void RetroFuturaGUI::RadioButtonGroup::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    updateLabelLayout();
}

void RetroFuturaGUI::RadioButtonGroup::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);
    updateLabelLayout();
}

void RetroFuturaGUI::RadioButtonGroup::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_text)
        _text->SetRotation(rotation);
}

void RetroFuturaGUI::RadioButtonGroup::DeselectOthers(RadioButton* activeRadioButton)
{
    for(RadioButton* ref : _radioButtonRefs)
        if(ref != activeRadioButton)
            ref->SetValue(false, false);
}

void RetroFuturaGUI::RadioButtonGroup::RegisterRadioButton(RadioButton* newRadioButton)
{
    _radioButtonRefs.push_back(newRadioButton);
}

void RetroFuturaGUI::RadioButtonGroup::UnregisterRadioButton(RadioButton* obsoleteRadioButton)
{
    _radioButtonRefs.erase(
        std::remove_if(_radioButtonRefs.begin(), _radioButtonRefs.end(),
        [&](RadioButton* ptr) { return obsoleteRadioButton == ptr; })
    );
}

void RetroFuturaGUI::RadioButtonGroup::SetText(std::string_view text)
{
    ITextProperties::SetText(text);
    updateLabelLayout();
}

void RetroFuturaGUI::RadioButtonGroup::updateLabelLayout()
{
    if(!_text || !_border)
        return;

    
    if(GetText().empty()) //if no label, draw solid border
    {
        _border->SetSize(glm::vec2(_size.x, _size.y));
        _border->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.01f));
        _text->SetParentSize(glm::vec2(_size.x, _size.y));
        IBorder::SetBorderGaps(std::span<BorderGap>());
        return;
    }

    f32 
        glyphHeight { _text->GetHeight() },
        textPadding { _text->GetPadding() },
        textWidth { _text->GetWidth() };

    glm::vec2 newSize { _size.x, _size.y - glyphHeight * 0.5f };
    glm::vec3 newPos { _position.x, _position.y + glyphHeight * 0.5f, _position.z + 0.01f };
    _border->SetSize(newSize);
    _border->SetPosition(newPos);
    _text->SetParentSize(newSize);

    // single top border's edge gap, sized to fit the label (+ padding on both sides)
    BorderGap textGap {
        .edge = BorderEdge::Top,
        .offset = _textLeftPadding,
        .length = textWidth + textPadding * 2.0f,
        .anchorFarCorner = false,
        .repeat = 1
    };
    IBorder::SetBorderGaps(std::span<BorderGap>(&textGap, 1));
    _text->SetTextAlignment(TextAlignment::Center);
    _text->SetPosition(glm::vec3(
        newPos.x - newSize.x * 0.5f + _textLeftPadding + textGap.length * 0.5f,
        newPos.y + newSize.y * 0.5f,
        newPos.z + 0.01f));
}

void RetroFuturaGUI::RadioButtonGroup::SetBorderGaps(std::span<BorderGap> gaps)
{
    IBorder::SetBorderGaps(gaps);

    for(const BorderGap& gap : gaps)
        if(gap.edge == BorderEdge::Left)
        {
            _textLeftPadding = gap.offset;
            return;
        }
}