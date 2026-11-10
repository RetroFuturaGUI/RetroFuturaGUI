#include "TableStyles.hpp"
#include <algorithm>
#include <array>
#include <utility>

RetroFuturaGUI::TableTextLayers::TableTextLayers(FlavorPick pick)
    : _pick(pick)
{
}

RetroFuturaGUI::TableTextLayer& RetroFuturaGUI::TableTextLayers::GetOwn()
{
    return _own;
}

const RetroFuturaGUI::TableTextSettings& RetroFuturaGUI::TableTextLayers::GetShown() const
{
    return _shown;
}

bool RetroFuturaGUI::TableTextLayers::Resolve(std::span<const u32> flavors, const bool isStyled)
{
    const TableTextSettings defaults {};
    const std::optional<std::string> family { find(&TableTextLayer::_FontFamily, flavors, isStyled) };
    const TableTextSettings shown
    {
        ._HasFont = family.has_value() && !family->empty(),
        ._FontFamily = family.value_or(std::string {}),
        ._FontSize = find(&TableTextLayer::_FontSize, flavors, isStyled).value_or(defaults._FontSize),
        ._Slant = find(&TableTextLayer::_Slant, flavors, isStyled).value_or(defaults._Slant),
        ._Weight = find(&TableTextLayer::_Weight, flavors, isStyled).value_or(defaults._Weight),
        ._Alignment = find(&TableTextLayer::_Alignment, flavors, isStyled).value_or(defaults._Alignment),
        ._Padding = find(&TableTextLayer::_Padding, flavors, isStyled).value_or(defaults._Padding)
    };

    if(shown == _shown)
        return false;

    _shown = shown;
    return true;
}

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

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::ShownBackgroundColors() const
{
    return shown(_background, _builtInBackground, &FlavorTrackStyle::GetBackgroundColors);
}

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::ShownInnerBorderColors() const
{
    return shown(_innerBorder, _builtInInnerBorder, &FlavorTrackStyle::GetInnerBorderColors);
}

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::ShownTextColors() const
{
    return shown(_text, _builtInText, &FlavorTrackStyle::GetTextColors);
}

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::ShownIconColors() const
{
    return shown(_icon, _builtInIcon, &FlavorTrackStyle::GetIconColors);
}

void RetroFuturaGUI::TrackStyle::setStyleSource(const std::vector<u32>* flavors, const uSize index)
{
    _flavors = flavors;
    _index = index;
}

const std::vector<glm::vec4>& RetroFuturaGUI::TrackStyle::shown(const std::vector<glm::vec4>& own, const std::vector<glm::vec4>& builtIn, const std::vector<glm::vec4>& (FlavorTrackStyle::*get)() const) const
{
    if(!own.empty())
        return own;

    if(!_flavors)
        return builtIn;

    const std::vector<glm::vec4>* styled { StylingManager::FindFirst<const std::vector<glm::vec4>*>(*_flavors, [this, get](const Flavor& flavor) -> const std::vector<glm::vec4>*
    {
        const FlavorTrackStyle* trackStyle { flavor.FindTrackStyle(_index) };

        if(!trackStyle)
            return nullptr;

        const std::vector<glm::vec4>& colors { (trackStyle->*get)() };
        return colors.empty() ? nullptr : &colors;
    }) };

    if(!styled)
        return builtIn;

    return *styled;
}

RetroFuturaGUI::CellStyle::CellStyle(std::function<void()> onTextChange)
    : _textSettings([](const Flavor& flavor) -> const TableTextLayer*
      {
          const FlavorCellStyle* cellStyle { flavor.FindCellStyle() };
          return cellStyle ? &cellStyle->GetTextSettings() : nullptr;
      }),
      _onTextChange(std::move(onTextChange))
{
    _background.SetBuiltIn(std::array { glm::vec4(0.2f, 0.3f, 0.55f, 1.0f) }, ColorState::Selected);
    _innerBorder.SetBuiltIn(std::array { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) }, ColorState::Disabled);
    _innerBorder.SetBuiltIn(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Hover);
    _innerBorder.SetBuiltIn(std::array { glm::vec4(0.6f, 0.6f, 0.6f, 1.0f) }, ColorState::Clicked);
    _text.SetBuiltIn(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Disabled);
    _text.SetBuiltIn(std::array { glm::vec4(1.0f) }, ColorState::Selected);
    _icon.SetBuiltIn(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Disabled);
    _dropTarget.SetBuiltIn(std::array { glm::vec4(0.25f, 0.5f, 0.42f, 1.0f) }, ColorState::Enabled);
}

void RetroFuturaGUI::CellStyle::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    TableTextLayer& own { _textSettings.GetOwn() };
    own._FontFamily = std::string(fontFamily);
    own._FontSize = FontManager::ToPixels(fontSize, sizeUnit);
    own._Slant = slant;
    own._Weight = fontWeight;
    resolveText();
}

f32 RetroFuturaGUI::CellStyle::GetFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_textSettings.GetShown()._FontSize, sizeUnit);
}

void RetroFuturaGUI::CellStyle::SetTextAlignment(const TextAlignment alignment)
{
    _textSettings.GetOwn()._Alignment = alignment;
    resolveText();
}

RetroFuturaGUI::TextAlignment RetroFuturaGUI::CellStyle::GetTextAlignment() const
{
    return _textSettings.GetShown()._Alignment;
}

void RetroFuturaGUI::CellStyle::SetTextPadding(const f32 padding)
{
    _textSettings.GetOwn()._Padding = padding;
    resolveText();
}

f32 RetroFuturaGUI::CellStyle::GetTextPadding() const
{
    return _textSettings.GetShown()._Padding;
}

const RetroFuturaGUI::TableTextSettings& RetroFuturaGUI::CellStyle::GetTextSettings() const
{
    return _textSettings.GetShown();
}

void RetroFuturaGUI::CellStyle::restyle()
{
    resolveText();
}

void RetroFuturaGUI::CellStyle::resolveText()
{
    if(!_textSettings.Resolve(getFlavors(), isStyled()))
        return;

    notifyTextChange();
}

void RetroFuturaGUI::CellStyle::notifyTextChange()
{
    if(!_onTextChange)
        return;

    _onTextChange();
}

void RetroFuturaGUI::CellStyle::SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Enabled == state)
        return; //each track's own.

    _background.SetOwn(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetBackgroundColors(const ColorState state) const
{
    return _background.GetOwn(state);
}

void RetroFuturaGUI::CellStyle::SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Enabled == state)
        return;

    _innerBorder.SetOwn(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetInnerBorderColors(const ColorState state) const
{
    return _innerBorder.GetOwn(state);
}

void RetroFuturaGUI::CellStyle::SetTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Enabled == state)
        return;

    _text.SetOwn(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetTextColors(const ColorState state) const
{
    return _text.GetOwn(state);
}

void RetroFuturaGUI::CellStyle::SetIconColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Enabled == state)
        return;

    _icon.SetOwn(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetIconColors(const ColorState state) const
{
    return _icon.GetOwn(state);
}

void RetroFuturaGUI::CellStyle::SetDropTargetColors(std::span<const glm::vec4> colors)
{
    _dropTarget.SetOwn(colors, ColorState::Enabled);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::GetDropTargetColors() const
{
    return _dropTarget.GetOwn(ColorState::Enabled);
}

void RetroFuturaGUI::CellStyle::SetInnerBorderWidth(const f32 width)
{
    _innerBorderWidth = width;
}

f32 RetroFuturaGUI::CellStyle::GetInnerBorderWidth() const
{
    if(_innerBorderWidth)
        return *_innerBorderWidth;

    return StylingManager::FindWidth(getFlavors(), StylePart::CellStyleInnerBorder).value_or(1.0f);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownBackgroundColors(const TrackStyle& track, const ColorState state) const
{
    return shown(_background, StylePart::CellStyleBackground, track.ShownBackgroundColors(), state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownInnerBorderColors(const TrackStyle& track, const ColorState state) const
{
    return shown(_innerBorder, StylePart::CellStyleInnerBorder, track.ShownInnerBorderColors(), state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownTextColors(const TrackStyle& track, const ColorState state) const
{
    return shown(_text, StylePart::CellStyleText, track.ShownTextColors(), state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownIconColors(const TrackStyle& track, const ColorState state) const
{
    return shown(_icon, StylePart::CellStyleIcon, track.ShownIconColors(), state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::ShownDropTargetColors() const
{
    return _dropTarget.GetShown(ColorState::Enabled, getFlavors(), isStyled(), StylePart::CellStyleDropTarget);
}

const std::vector<glm::vec4>& RetroFuturaGUI::CellStyle::shown(const LayeredColors& colors, const StylePart part, const std::vector<glm::vec4>& own, const ColorState state) const
{
    if(ColorState::Enabled == state)
        return own;

    const std::vector<glm::vec4>* shared { colors.Find(state, getFlavors(), isStyled(), part) };

    if(!shared)
        return own;

    return *shared;
}

RetroFuturaGUI::HeaderStyle::HeaderStyle(std::function<void()> onTextChange)
    : _textSettings([](const Flavor& flavor) -> const TableTextLayer*
      {
          const FlavorHeaderStyle* headerStyle { flavor.FindHeaderStyle() };
          return headerStyle ? &headerStyle->GetTextSettings() : nullptr;
      }),
      _onTextChange(std::move(onTextChange))
{
    _background.SetBuiltIn(std::array { ResourceManager::_Eigengrau }, ColorState::Enabled);
    _background.SetBuiltIn(std::array { ResourceManager::_Eigengrau }, ColorState::Disabled);
    _innerBorder.SetBuiltIn(std::array { glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) }, ColorState::Enabled);
    _innerBorder.SetBuiltIn(std::array { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) }, ColorState::Disabled);
    _outerBorder.SetBuiltIn(std::array { glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) }, ColorState::Enabled);
    _outerBorder.SetBuiltIn(std::array { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) }, ColorState::Disabled);
    _text.SetBuiltIn(std::array { glm::vec4(1.0f) }, ColorState::Enabled);
    _text.SetBuiltIn(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Disabled);
}

void RetroFuturaGUI::HeaderStyle::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    TableTextLayer& own { _textSettings.GetOwn() };
    own._FontFamily = std::string(fontFamily);
    own._FontSize = FontManager::ToPixels(fontSize, sizeUnit);
    own._Slant = slant;
    own._Weight = fontWeight;
    resolveText();
}

f32 RetroFuturaGUI::HeaderStyle::GetFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_textSettings.GetShown()._FontSize, sizeUnit);
}

void RetroFuturaGUI::HeaderStyle::SetTextAlignment(const TextAlignment alignment)
{
    _textSettings.GetOwn()._Alignment = alignment;
    resolveText();
}

RetroFuturaGUI::TextAlignment RetroFuturaGUI::HeaderStyle::GetTextAlignment() const
{
    return _textSettings.GetShown()._Alignment;
}

void RetroFuturaGUI::HeaderStyle::SetTextPadding(const f32 padding)
{
    _textSettings.GetOwn()._Padding = padding;
    resolveText();
}

f32 RetroFuturaGUI::HeaderStyle::GetTextPadding() const
{
    return _textSettings.GetShown()._Padding;
}

const RetroFuturaGUI::TableTextSettings& RetroFuturaGUI::HeaderStyle::GetTextSettings() const
{
    return _textSettings.GetShown();
}

void RetroFuturaGUI::HeaderStyle::restyle()
{
    resolveText();
}

void RetroFuturaGUI::HeaderStyle::resolveText()
{
    if(!_textSettings.Resolve(getFlavors(), isStyled()))
        return;

    notifyTextChange();
}

void RetroFuturaGUI::HeaderStyle::notifyTextChange()
{
    if(!_onTextChange)
        return;

    _onTextChange();
}

void RetroFuturaGUI::HeaderStyle::SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _background.SetOwn(colors, headerState(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::GetBackgroundColors(const ColorState state) const
{
    return _background.GetOwn(headerState(state));
}

void RetroFuturaGUI::HeaderStyle::SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _innerBorder.SetOwn(colors, headerState(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::GetInnerBorderColors(const ColorState state) const
{
    return _innerBorder.GetOwn(headerState(state));
}

void RetroFuturaGUI::HeaderStyle::SetOuterBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _outerBorder.SetOwn(colors, headerState(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::GetOuterBorderColors(const ColorState state) const
{
    return _outerBorder.GetOwn(headerState(state));
}

void RetroFuturaGUI::HeaderStyle::SetTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _text.SetOwn(colors, headerState(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::GetTextColors(const ColorState state) const
{
    return _text.GetOwn(headerState(state));
}

void RetroFuturaGUI::HeaderStyle::SetInnerBorderWidth(const f32 width)
{
    _innerBorderWidth = width;
}

f32 RetroFuturaGUI::HeaderStyle::GetInnerBorderWidth() const
{
    return shownWidth(_innerBorderWidth, StylePart::HeaderStyleInnerBorder);
}

void RetroFuturaGUI::HeaderStyle::SetOuterBorderWidth(const f32 width)
{
    _outerBorderWidth = width;
}

f32 RetroFuturaGUI::HeaderStyle::GetOuterBorderWidth() const
{
    return shownWidth(_outerBorderWidth, StylePart::HeaderStyleOuterBorder);
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::ShownBackgroundColors(const ColorState state) const
{
    return _background.GetShown(headerState(state), getFlavors(), isStyled(), StylePart::HeaderStyleBackground);
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::ShownInnerBorderColors(const ColorState state) const
{
    return _innerBorder.GetShown(headerState(state), getFlavors(), isStyled(), StylePart::HeaderStyleInnerBorder);
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::ShownOuterBorderColors(const ColorState state) const
{
    return _outerBorder.GetShown(headerState(state), getFlavors(), isStyled(), StylePart::HeaderStyleOuterBorder);
}

const std::vector<glm::vec4>& RetroFuturaGUI::HeaderStyle::ShownTextColors(const ColorState state) const
{
    return _text.GetShown(headerState(state), getFlavors(), isStyled(), StylePart::HeaderStyleText);
}

RetroFuturaGUI::ColorState RetroFuturaGUI::HeaderStyle::headerState(const ColorState state)
{
    return ColorState::Disabled == state ? ColorState::Disabled : ColorState::Enabled;
}

f32 RetroFuturaGUI::HeaderStyle::shownWidth(const std::optional<f32>& own, const StylePart part) const
{
    if(own)
        return *own;

    return StylingManager::FindWidth(getFlavors(), part).value_or(1.0f);
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
        _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(1.0f) }, state);
        _border.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(1.0f) }, state);
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