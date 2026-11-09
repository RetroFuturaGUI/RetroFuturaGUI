#include "TableStyles.hpp"
#include <algorithm>
#include <array>
#include <utility>

void RetroFuturaGUI::TrackStyle::SetBackgroundColors(std::span<const glm::vec4> colors)
{
    _background.assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::GetBackgroundColors() const
{
    return _background;
}

void RetroFuturaGUI::TrackStyle::SetInnerBorderColors(std::span<const glm::vec4> colors)
{
    _innerBorder.assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::GetInnerBorderColors() const
{
    return _innerBorder;
}

void RetroFuturaGUI::TrackStyle::SetTextColors(std::span<const glm::vec4> colors)
{
    _text.assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::GetTextColors() const
{
    return _text;
}

void RetroFuturaGUI::TrackStyle::SetIconColors(std::span<const glm::vec4> colors)
{
    _icon.assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::GetIconColors() const
{
    return _icon;
}

RetroFuturaGUI::CellStyle::CellStyle(std::function<void()> onTextChange)
    : _onTextChange(std::move(onTextChange))
{
}

void RetroFuturaGUI::CellStyle::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _textSettings._HasFont = true;
    _textSettings._FontFamily = fontFamily;
    _textSettings._FontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _textSettings._Slant = slant;
    _textSettings._Weight = fontWeight;
    notifyTextChange();
}

f32 RetroFuturaGUI::CellStyle::GetFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_textSettings._FontSize, sizeUnit);
}

void RetroFuturaGUI::CellStyle::SetTextAlignment(const TextAlignment alignment)
{
    _textSettings._Alignment = alignment;
    notifyTextChange();
}

RetroFuturaGUI::TextAlignment RetroFuturaGUI::CellStyle::GetTextAlignment() const
{
    return _textSettings._Alignment;
}

void RetroFuturaGUI::CellStyle::SetTextPadding(const f32 padding)
{
    _textSettings._Padding = padding;
    notifyTextChange();
}

f32 RetroFuturaGUI::CellStyle::GetTextPadding() const
{
    return _textSettings._Padding;
}

const RetroFuturaGUI::TableTextSettings& RetroFuturaGUI::CellStyle::GetTextSettings() const
{
    return _textSettings;
}

void RetroFuturaGUI::CellStyle::notifyTextChange()
{
    if(!_onTextChange)
        return;

    _onTextChange();
}

void RetroFuturaGUI::CellStyle::SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    std::vector<glm::vec4>* target { colorsOf(_background, state) };

    if(!target)
        return;

    target->assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetBackgroundColors(const ColorState state) const
{
    return colorsOf(_background, state);
}

void RetroFuturaGUI::CellStyle::SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    std::vector<glm::vec4>* target { colorsOf(_innerBorder, state) };

    if(!target)
        return;

    target->assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetInnerBorderColors(const ColorState state) const
{
    return colorsOf(_innerBorder, state);
}

void RetroFuturaGUI::CellStyle::SetTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    std::vector<glm::vec4>* target { colorsOf(_text, state) };

    if(!target)
        return;

    target->assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetTextColors(const ColorState state) const
{
    return colorsOf(_text, state);
}

void RetroFuturaGUI::CellStyle::SetIconColors(std::span<const glm::vec4> colors, const ColorState state)
{
    std::vector<glm::vec4>* target { colorsOf(_icon, state) };

    if(!target)
        return;

    target->assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetIconColors(const ColorState state) const
{
    return colorsOf(_icon, state);
}

void RetroFuturaGUI::CellStyle::SetDropTargetColors(std::span<const glm::vec4> colors)
{
    _dropTarget.assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetDropTargetColors() const
{
    return _dropTarget;
}

void RetroFuturaGUI::CellStyle::SetInnerBorderWidth(const f32 width)
{
    _innerBorderWidth = width;
}

f32 RetroFuturaGUI::CellStyle::GetInnerBorderWidth() const
{
    return _innerBorderWidth;
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownBackgroundColors(const TrackStyle& track, const ColorState state) const
{
    return shown(_background, track.GetBackgroundColors(), state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownInnerBorderColors(const TrackStyle& track, const ColorState state) const
{
    return shown(_innerBorder, track.GetInnerBorderColors(), state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownTextColors(const TrackStyle& track, const ColorState state) const
{
    return shown(_text, track.GetTextColors(), state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownIconColors(const TrackStyle& track, const ColorState state) const
{
    return shown(_icon, track.GetIconColors(), state);
}

std::vector<glm::vec4>* RetroFuturaGUI::CellStyle::colorsOf(StateColors& colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Disabled:
            return &colors._Disabled;
        case ColorState::Hover:
            return &colors._Hover;
        case ColorState::Clicked:
            return &colors._Clicked;
        case ColorState::Selected:
            return &colors._Selected;
        default: //Enabled is each track's own
            return nullptr;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::colorsOf(const StateColors& colors, const ColorState state)
{
    static const std::vector<glm::vec4> none {};

    switch(state)
    {
        case ColorState::Disabled:
            return colors._Disabled;
        case ColorState::Hover:
            return colors._Hover;
        case ColorState::Clicked:
            return colors._Clicked;
        case ColorState::Selected:
            return colors._Selected;
        default: //Enabled is each track's own
            return none;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::shown(const StateColors& colors, const std::vector<glm::vec4>& own, const ColorState state)
{
    const std::vector<glm::vec4>& shared { colorsOf(colors, state) };

    if(shared.empty())
        return own;

    return shared;
}

RetroFuturaGUI::HeaderStyle::HeaderStyle(std::function<void()> onTextChange)
    : _onTextChange(std::move(onTextChange))
{
}

void RetroFuturaGUI::HeaderStyle::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _textSettings._HasFont = true;
    _textSettings._FontFamily = fontFamily;
    _textSettings._FontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _textSettings._Slant = slant;
    _textSettings._Weight = fontWeight;
    notifyTextChange();
}

f32 RetroFuturaGUI::HeaderStyle::GetFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_textSettings._FontSize, sizeUnit);
}

void RetroFuturaGUI::HeaderStyle::SetTextAlignment(const TextAlignment alignment)
{
    _textSettings._Alignment = alignment;
    notifyTextChange();
}

RetroFuturaGUI::TextAlignment RetroFuturaGUI::HeaderStyle::GetTextAlignment() const
{
    return _textSettings._Alignment;
}

void RetroFuturaGUI::HeaderStyle::SetTextPadding(const f32 padding)
{
    _textSettings._Padding = padding;
    notifyTextChange();
}

f32 RetroFuturaGUI::HeaderStyle::GetTextPadding() const
{
    return _textSettings._Padding;
}

const RetroFuturaGUI::TableTextSettings& RetroFuturaGUI::HeaderStyle::GetTextSettings() const
{
    return _textSettings;
}

void RetroFuturaGUI::HeaderStyle::notifyTextChange()
{
    if(!_onTextChange)
        return;

    _onTextChange();
}

void RetroFuturaGUI::HeaderStyle::SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(_background, state).assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::GetBackgroundColors(const ColorState state) const
{
    return colorsOf(_background, state);
}

void RetroFuturaGUI::HeaderStyle::SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(_innerBorder, state).assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::GetInnerBorderColors(const ColorState state) const
{
    return colorsOf(_innerBorder, state);
}

void RetroFuturaGUI::HeaderStyle::SetOuterBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(_outerBorder, state).assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::GetOuterBorderColors(const ColorState state) const
{
    return colorsOf(_outerBorder, state);
}

void RetroFuturaGUI::HeaderStyle::SetTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(_text, state).assign(colors.begin(), colors.end());
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::GetTextColors(const ColorState state) const
{
    return colorsOf(_text, state);
}

void RetroFuturaGUI::HeaderStyle::SetInnerBorderWidth(const f32 width)
{
    _innerBorderWidth = width;
}

f32 RetroFuturaGUI::HeaderStyle::GetInnerBorderWidth() const
{
    return _innerBorderWidth;
}

void RetroFuturaGUI::HeaderStyle::SetOuterBorderWidth(const f32 width)
{
    _outerBorderWidth = width;
}

f32 RetroFuturaGUI::HeaderStyle::GetOuterBorderWidth() const
{
    return _outerBorderWidth;
}

std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::colorsOf(StateColors& colors, const ColorState state)
{
    return ColorState::Disabled == state ? colors._Disabled : colors._Enabled;
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::colorsOf(const StateColors& colors, const ColorState state)
{
    return ColorState::Disabled == state ? colors._Disabled : colors._Enabled;
}

RetroFuturaGUI::CellCheckBox::CellCheckBox(Projection* projection)
    : _background(projection), _border(projection),
      _checkmark
      {
          ResourceManager::CreateCheckmarkIcon(projection),
          "CheckmarkPath",
          SvgPathFill { ._FillType = FillType::Solid, ._Colors = {}, ._GradientDegree = 0.0f, ._GradientOffset = 0.0f, ._GradientAnimationSpeed = 0.0f, ._GradientRotationSpeed = 0.0f },
          IconColors
          {
              ._Enabled = { glm::vec4(1.0f) },
              ._Disabled = { glm::vec4(1.0f) },
              ._Clicked = { glm::vec4(1.0f) },
              ._Hover = { glm::vec4(1.0f) },
              ._Selected = {}
          }
      }
{
    for(const ColorState state : { ColorState::Enabled, ColorState::Hover, ColorState::Clicked, ColorState::Disabled })
    {
        _background.SetColors(std::array { glm::vec4(1.0f) }, state);
        _border.SetColors(std::array { glm::vec4(1.0f) }, state);
    }
}

RetroFuturaGUI::Background& RetroFuturaGUI::CellCheckBox::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::CellCheckBox::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::CellCheckBox::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::CellCheckBox::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::Icon& RetroFuturaGUI::CellCheckBox::GetCheckmark()
{
    return _checkmark;
}

const RetroFuturaGUI::Icon& RetroFuturaGUI::CellCheckBox::GetCheckmark() const
{
    return _checkmark;
}

void RetroFuturaGUI::CellCheckBox::SetColorState(const ColorState state)
{
    _background.SetColorState(state);
    _border.SetColorState(state);
}

void RetroFuturaGUI::CellCheckBox::SetCheckmarkColors(std::span<const glm::vec4> colors)
{
    //Shown as its Enabled colors, so one state carries whatever the track's state gives it.
    _checkmark.SetColorState(ColorState::Enabled);

    if(std::ranges::equal(colors, _checkmark.GetColors(ColorState::Enabled)))
        return; //neighboring cells of one track ask for the same colors again and again.

    _checkmark.SetColors(colors, ColorState::Enabled);
}