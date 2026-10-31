#include "ColorPicker.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <memory>

RetroFuturaGUI::ColorPicker::ColorPicker(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ColorPicker;
    _colorPickerBackground = std::make_unique<ColorField>(projection);
    _colorPickerBorder = std::make_unique<Frame>(projection);
    _indicatorBackground = std::make_unique<Plane>(projection);
    _indicatorBorder = std::make_unique<Frame>(projection);

    if(!_colorPickerBackground)
        return;

    if(!_colorPickerBorder)
        return;

    if(!_indicatorBackground)
        return;

    if(!_indicatorBorder)
        return;

    _colorPickerBackground->SetShape(ColorFieldShape::Circle);
    _colorPickerBorder->SetBorderWidth(2.0f);
    _indicatorBackground->SetSize(glm::vec2(_indicatorDiameter));
    _indicatorBackground->SetCornerRadii(glm::vec4(0.5f * _indicatorDiameter));
    _indicatorBackground->SetColors(_indicatorBackgroundColors);
    _indicatorBorder->SetSize(glm::vec2(_indicatorDiameter));
    _indicatorBorder->SetCornerRadii(glm::vec4(0.5f * _indicatorDiameter));
    _indicatorBorder->SetBorderWidth(2.0f);
    updateColor(false);
}

void RetroFuturaGUI::ColorPicker::Draw()
{
    if(!_colorPickerBackground)
        return;

    if(!_colorPickerBorder)
        return;

    if(!_indicatorBackground)
        return;

    if(!_indicatorBorder)
        return;

    _colorPickerBackground->SetBrightness(_brightness);
    _colorPickerBackground->Draw();
    _colorPickerBorder->Draw();
    _indicatorBackground->Draw();
    _indicatorBorder->Draw();
}

void RetroFuturaGUI::ColorPicker::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(!_colorPickerBackground)
        return;

    if(!_colorPickerBorder)
        return;

    const bool isCircle { _colorPickerType == ColorPickerType::Circle };
    const glm::vec2 fieldSize { isCircle ? glm::vec2(glm::min(_size.x, _size.y)) : glm::vec2(_size) };
    _colorPickerBorder->SetSize(fieldSize);
    _colorPickerBorder->SetCornerRadii(glm::vec4(isCircle ? 0.5f * fieldSize.x : 0.0f)); //0 turns rounding off for the square
    _colorPickerBackground->SetSize(fieldSize - 2.0f); //1 px in from the border's outer edge, so the field's edge stays under the 2 px border
    alignIndicator();
}

void RetroFuturaGUI::ColorPicker::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(!_colorPickerBackground)
        return;

    if(!_colorPickerBorder)
        return;

    if(!_indicatorBackground)
        return;

    if(!_indicatorBorder)
        return;

    _colorPickerBorder->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.01f));
    _colorPickerBackground->SetPosition(_position);
    alignIndicator();
}

void RetroFuturaGUI::ColorPicker::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(!_colorPickerBackground)
        return;

    if(!_colorPickerBorder)
        return;

    if(!_indicatorBackground)
        return;

    if(!_indicatorBorder)
        return;

    _colorPickerBorder->SetRotation(_rotation);
    _colorPickerBackground->SetRotation(_rotation);
    _indicatorBackground->SetRotation(_rotation);
    _indicatorBorder->SetRotation(_rotation);
    alignIndicator();
}

const glm::vec3 RetroFuturaGUI::ColorPicker::GetColorRGBF() const
{
    return glm::vec3(_rgbaf);
}

const glm::vec4 RetroFuturaGUI::ColorPicker::GetColorRGBAF() const
{
    return _rgbaf;
}

u32 RetroFuturaGUI::ColorPicker::GetColorRGB() const
{
    return _rgba >> 8;
}

u32 RetroFuturaGUI::ColorPicker::GetColorRGBA() const
{
    return _rgba;
}

void RetroFuturaGUI::ColorPicker::SetBrightness(const f32 brightness, const bool emitSignal)
{
    _brightness = glm::clamp(brightness, 0.0f, 1.0f);
    updateColor(emitSignal);
}

f32 RetroFuturaGUI::ColorPicker::GetBrightness() const
{
    return _brightness;
}

void RetroFuturaGUI::ColorPicker::SetAlpha(const f32 alpha, const bool emitSignal)
{
    _alpha = glm::clamp(alpha, 0.0f, 1.0f);
    updateColor(emitSignal);
}

f32 RetroFuturaGUI::ColorPicker::GetAlpha() const
{
    return _alpha;
}

void RetroFuturaGUI::ColorPicker::EnableAlpha(const bool enable, const bool emitSignal)
{
    _useAlpha = enable;
    updateColor(emitSignal);
}

bool RetroFuturaGUI::ColorPicker::IsAlphaEnabled() const
{
    return _useAlpha;
}

void RetroFuturaGUI::ColorPicker::SetColor(const glm::vec4& color, const bool emitSignal)
{
    /*learn more: 
        https://en.wikipedia.org/wiki/HSL_and_HSV
        https://www.alvyray.com/Papers/CG/color78.pdf
    */
    const glm::vec4 clamped { glm::clamp(color, 0.0f, 1.0f) };
    const f32 
        maxChannel { glm::max(clamped.r, glm::max(clamped.g, clamped.b)) },
        minChannel { glm::min(clamped.r, glm::min(clamped.g, clamped.b)) },
        chroma { maxChannel - minChannel }; //grayscale

    _brightness = maxChannel;
    _alpha = clamped.a;

    //black has no saturation and gray has no hue: keep the old ones, so brightening again restores the color
    if(0.0f < maxChannel)
        _saturation = chroma / maxChannel;

    if(0.0f < chroma)
    {
        f32 sector {}; //position on the hue circle in sixths, 0..6

        if(maxChannel == clamped.r)
            sector = glm::mod((clamped.g - clamped.b) / chroma, 6.0f);
        else if(maxChannel == clamped.g)
            sector = (clamped.b - clamped.r) / chroma + 2.0f;
        else
            sector = (clamped.r - clamped.g) / chroma + 4.0f;

        _hue = sector / 6.0f;
    }

    updateColor(emitSignal);
}

void RetroFuturaGUI::ColorPicker::SetColor(const glm::vec3& color, const bool emitSignal)
{
    SetColor(glm::vec4(color, _alpha), emitSignal); //keeps the current alpha
}

void RetroFuturaGUI::ColorPicker::Connect_OnColorChanged(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onColorChangedAsync.Connect(slot);
    else
        _onColorChanged.Connect(slot);
}

void RetroFuturaGUI::ColorPicker::Disconnect_OnColorChanged(const typename Signal<>::Slot& slot)
{
    _onColorChanged.Disconnect(slot);
    _onColorChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::ColorPicker::interact()
{
    if(!_colorPickerBackground)
        return;

    if(!_indicatorBackground)
        return;

    const PointerState pointer { readPointer() };
    const bool isPressStart { pointer._LeftButtonPressed && !_wasClicked };
    _wasClicked = pointer._LeftButtonPressed;

    if(!pointer._LeftButtonPressed || pointer._IsCaptured || !_isEnabledFlag) //releasing, a ScrollView taking the pointer or disabling ends the drag
        _isIndicatorDragging = false;

    if(!_isEnabledFlag)
    {
        setBorderStates(ColorState::Disabled, ColorState::Disabled);
        return;
    }

    if(pointer._IsCaptured || !pointer._HasPosition) //ScrollView above has taken the pointer, or it is outside the window
    {
        setBorderStates(ColorState::Enabled, ColorState::Enabled);
        return;
    }

    const glm::vec2
        fieldSize { _colorPickerBackground->GetSize() },
        fieldPoint { toFieldSpace(pointer._Position) };
    const bool isPointerOnField { isOnField(fieldPoint, fieldSize) };

    if(isPressStart && isPointerOnField) //a press anywhere on the field moves the indicator there and starts dragging it
        _isIndicatorDragging = true;

    if(_isIndicatorDragging)
    {
        pickColorAt(fieldPoint, fieldSize); //keeps following past the field's edge, held on the edge
        setBorderStates(ColorState::Clicked, ColorState::Clicked);
        return;
    }

    const bool isPointerOnIndicator { isPointInsideRect(pointer._Position, glm::vec3(_indicatorBackground->GetSize(), 0.0f), _indicatorBackground->GetPosition(), _rotation) };
    setBorderStates(isPointerOnField ? ColorState::Hover : ColorState::Enabled, isPointerOnIndicator ? ColorState::Hover : ColorState::Enabled);
}

void RetroFuturaGUI::ColorPicker::setBorderStates(const ColorState fieldState, const ColorState indicatorState)
{
    _colorPickerFieldState = fieldState;
    _indicatorState = indicatorState;
    setColorPickerFieldBorderColors();
    setIndicatorBorderColors();
}

void RetroFuturaGUI::ColorPicker::alignIndicator()
{
    if(!_colorPickerBackground)
        return;

    if(!_indicatorBackground)
        return;

    if(!_indicatorBorder)
        return;

    const glm::vec2 fieldSize { _colorPickerBackground->GetSize() };
    glm::vec2 fieldPoint {};

    if(_colorPickerType == ColorPickerType::Square)
    {
        fieldPoint = (glm::vec2(_hue, _saturation) - 0.5f) * fieldSize;
    }
    else
    {
        const f32 hueAngle { _hue * glm::two_pi<f32>() };
        fieldPoint = _saturation * 0.5f * glm::min(fieldSize.x, fieldSize.y) * glm::vec2(std::cos(hueAngle), std::sin(hueAngle));
    }

    // Back out of field space: turn with the widget, then stack in front of the field (0.0) and its border (0.01)
    const f32 rotation { glm::radians(_rotation.z) };
    _indicatorPosition = glm::vec3(
        fieldPoint.x * std::cos(rotation) - fieldPoint.y * std::sin(rotation),
        fieldPoint.x * std::sin(rotation) + fieldPoint.y * std::cos(rotation),
        0.02f);
    _indicatorBackground->SetPosition(_position + _indicatorPosition);
    _indicatorBorder->SetPosition(_position + _indicatorPosition + glm::vec3(0.0f, 0.0f, 0.01f));
}

glm::vec2 RetroFuturaGUI::ColorPicker::toFieldSpace(const glm::vec2& point) const
{
    // only the screen-plane rotation can be undone in 2D
    const glm::vec2 offset { point - glm::vec2(_position) };
    const f32 rotation { glm::radians(-_rotation.z) };
    return glm::vec2(
        offset.x * std::cos(rotation) - offset.y * std::sin(rotation),
        offset.x * std::sin(rotation) + offset.y * std::cos(rotation));
}

bool RetroFuturaGUI::ColorPicker::isOnField(const glm::vec2& fieldPoint, const glm::vec2& fieldSize) const
{
    if(_colorPickerType == ColorPickerType::Square)
        return glm::abs(fieldPoint.x) <= fieldSize.x * 0.5f && glm::abs(fieldPoint.y) <= fieldSize.y * 0.5f;

    return glm::length(fieldPoint) <= 0.5f * glm::min(fieldSize.x, fieldSize.y);
}

void RetroFuturaGUI::ColorPicker::pickColorAt(const glm::vec2& fieldPoint, const glm::vec2& fieldSize)
{
    if(0.0f >= fieldSize.x || 0.0f >= fieldSize.y) //nothing to pick from before the first SetSize
        return;

    f32
        hue { _hue },
        saturation { 0.0f };

    if(_colorPickerType == ColorPickerType::Square)
    {
        // Same layout as ColorSquare.fs: hue left to right, saturation bottom to top
        hue = glm::clamp(fieldPoint.x / fieldSize.x + 0.5f, 0.0f, 1.0f);
        saturation = glm::clamp(fieldPoint.y / fieldSize.y + 0.5f, 0.0f, 1.0f);
    }
    else
    {
        // Same layout as ColorCircle.fs: hue counter-clockwise from red at the right, saturation from the center out
        const f32 distance { glm::length(fieldPoint) };

        if(0.0f < distance) //the center has no hue, keep the old one
            hue = glm::fract(std::atan2(fieldPoint.y, fieldPoint.x) / glm::two_pi<f32>());

        saturation = glm::min(distance / (0.5f * glm::min(fieldSize.x, fieldSize.y)), 1.0f);
    }

    if(hue == _hue && saturation == _saturation) //the pointer hasn't moved, so no signal
        return;

    _hue = hue;
    _saturation = saturation;
    updateColor(true);
}

void RetroFuturaGUI::ColorPicker::updateColor(const bool emitSignal)
{
    const glm::vec3 hueColor { glm::clamp(glm::abs(glm::mod(_hue * 6.0f + glm::vec3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f) };
    _rgbaf = glm::vec4(_brightness * glm::mix(glm::vec3(1.0f), hueColor, _saturation), _useAlpha ? _alpha : 1.0f);

    const glm::u32vec4 channels { glm::round(_rgbaf * 255.0f) };
    _rgba = channels.r << 24 | channels.g << 16 | channels.b << 8 | channels.a; //alpha in the low byte, like the 0x000000FF default

    _indicatorBackgroundColors[0] = glm::vec4(glm::vec3(_rgbaf), 1.0f); //opaque, or the field would show through a transparent pick
    alignIndicator();

    if(!emitSignal)
        return;

    _onColorChanged.Emit();
    _onColorChangedAsync.EmitAsync();
}

void RetroFuturaGUI::ColorPicker::SetColorPickerFieldBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _colorPickerBorderColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _colorPickerBorderColorsDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _colorPickerBorderColorsHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _colorPickerBorderColorsEnabled.assign(colors.begin(), colors.end());
    }

    setColorPickerFieldBorderColors();
}

void RetroFuturaGUI::ColorPicker::SetColorPickerIndicatorBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _indicatorBorderColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _indicatorBorderColorsDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _indicatorBorderColorsHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _indicatorBorderColorsEnabled.assign(colors.begin(), colors.end());
    }

    setIndicatorBorderColors();
}

void RetroFuturaGUI::ColorPicker::setColorPickerFieldBorderColors()
{
    if(!_colorPickerBorder)
        return;

    switch(_colorPickerFieldState)
    {
        case ColorState::Enabled:
            _colorPickerBorder->SetColors(_colorPickerBorderColorsEnabled);
        break;
        case ColorState::Clicked:
            _colorPickerBorder->SetColors(_colorPickerBorderColorsClicked);
        break;
        case ColorState::Hover:
            _colorPickerBorder->SetColors(_colorPickerBorderColorsHover);
        break;
        default: //Disabled
            _colorPickerBorder->SetColors(_colorPickerBorderColorsDisabled);
    }
}

void RetroFuturaGUI::ColorPicker::setIndicatorBorderColors()
{
    if(!_indicatorBorder)
        return;

    switch(_indicatorState)
    {
        case ColorState::Enabled:
            _indicatorBorder->SetColors(_indicatorBorderColorsEnabled);
        break;
        case ColorState::Clicked:
            _indicatorBorder->SetColors(_indicatorBorderColorsClicked);
        break;
        case ColorState::Hover:
            _indicatorBorder->SetColors(_indicatorBorderColorsHover);
        break;
        default: //Disabled
            _indicatorBorder->SetColors(_indicatorBorderColorsDisabled);
    }
}