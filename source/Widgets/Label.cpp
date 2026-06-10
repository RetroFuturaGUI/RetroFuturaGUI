#include "Label.hpp"

RetroFuturaGUI::Label::Label(const IdentityParams& identity, const GeometryParams3D& geometry, const TextParams& textParams)
: IWidget(identity, geometry)
{
    _widgetTypeID = WidgetTypeID::Label;

    if(textParams._Text.size() > 0)
        _text = std::make_unique<Text>(geometry, textParams);
}

void RetroFuturaGUI::Label::Draw()
{
    drawText();
}

void RetroFuturaGUI::Label::SetEnabled(const bool enable)
{
    _isEnabledFlag = enable;

    if(_isEnabledFlag)
    {
        _onEnableAsync.EmitAsync();
        _onEnable.Emit();
        setColors(ColorState::Enabled);
        return;
    }

    _onDisableAsync.EmitAsync();
    _onDisable.Emit();
    setColors(ColorState::Disabled);
}

void RetroFuturaGUI::Label::setColors(const ColorState state)
{
    _textColorState = state;
    setTextColors();
}

void RetroFuturaGUI::Label::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    //if(_text)    
        //_text->Resize(size); //add extra text resizing logic
}

void RetroFuturaGUI::Label::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_text)
        _text->SetPosition(position);
}

void RetroFuturaGUI::Label::SetRotation(const float rotation)
{
    _rotation = rotation;

    if(_text)
        _text->SetRotation(rotation);
}