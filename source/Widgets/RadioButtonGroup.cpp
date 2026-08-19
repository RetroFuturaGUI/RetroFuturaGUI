#include "RadioButtonGroup.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "RadioButton.hpp"
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

    if(_border)
        _border->SetSize(size);

    if(_text)
        _text->SetParentSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::RadioButtonGroup::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    if(_text)
        _text->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.02f));
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
