#include "Label.hpp"

RetroFuturaGUI::Label::Label(const IdentityParams& identity, const GeometryParams2D& geometry, const TextParams& textParams)
: IWidget(identity, geometry)
{
    _widgetTypeID = WidgetTypeID::Label;
    _textObject = std::make_unique<Text>(geometry, textParams);



}

void RetroFuturaGUI::Label::Draw()
{
    if(_textObject)
        _textObject->Draw();
}

void RetroFuturaGUI::Label::SetEnabled(const bool enable)
{
    _isEnabledFlag = enable;

    if(_isEnabledFlag)
    {
        _onEnableAsync.EmitAsync();
        _onEnable.Emit();
        _colorState = ColorState::Enabled;
        setColors();
        return;
    }

    _onDisableAsync.EmitAsync();
    _onDisable.Emit();
    ColorState::Disabled;
    setColors();
}

void RetroFuturaGUI::Label::setColors()
{
    switch(_colorState)
    {
        case ColorState::Enabled:
        {
            _textObject->SetColor(_textColorEnabled.front());
        } break;
        /*case ColorState::Clicked:
        {
            _textObject->SetColor(_textColorClicked.front());
        } break;*/
        case ColorState::Hover:
        {
            _textObject->SetColor(_textColorHover.front());
        } break;
        default: //Disabled
        {
            _textObject->SetColor(_textColorDisabled.front());
        }
    }
}