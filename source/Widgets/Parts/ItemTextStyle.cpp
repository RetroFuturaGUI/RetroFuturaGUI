#include "ItemTextStyle.hpp"
#include <array>
#include <utility>

RetroFuturaGUI::ItemTextStyle::ItemTextStyle(std::function<void(const ItemTextLayout&)> onLayoutChange)
    : _onLayoutChange(std::move(onLayoutChange))
{
    _colors.SetBuiltIn(std::array { glm::vec4(1.0f) }, ColorState::Enabled);
    _colors.SetBuiltIn(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Disabled);
    _colors.SetBuiltIn(std::array { glm::vec4(1.0f) }, ColorState::Hover);
    _colors.SetBuiltIn(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Clicked);
}

void RetroFuturaGUI::ItemTextStyle::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _colors.SetOwn(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::ItemTextStyle::GetColors(const ColorState state) const
{
    return _colors.GetOwn(state);
}

void RetroFuturaGUI::ItemTextStyle::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _fontFamily.Set(std::string(fontFamily));
    _fontSize.Set(FontManager::ToPixels(fontSize, sizeUnit));
    _slant.Set(slant);
    _weight.Set(fontWeight);
    resolveLayout();
}

void RetroFuturaGUI::ItemTextStyle::SetAlignment(const TextAlignment alignment)
{
    _alignment.Set(alignment);
    resolveLayout();
}

void RetroFuturaGUI::ItemTextStyle::SetPadding(const f32 padding)
{
    _padding.Set(padding);
    resolveLayout();
}

const RetroFuturaGUI::ItemTextLayout& RetroFuturaGUI::ItemTextStyle::GetShownLayout() const
{
    return _shownLayout;
}

const std::vector<glm::vec4>& RetroFuturaGUI::ItemTextStyle::GetShownColors(const ColorState state) const
{
    return _colors.GetShown(state, getFlavors(), isStyled(), getStylePart());
}

void RetroFuturaGUI::ItemTextStyle::SetBuiltInFontFamily(const StyleKey&, std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _fontFamily.SetBuiltIn(std::string(fontFamily));
    _fontSize.SetBuiltIn(FontManager::ToPixels(fontSize, sizeUnit));
    _slant.SetBuiltIn(slant);
    _weight.SetBuiltIn(fontWeight);
    resolveLayout();
}

void RetroFuturaGUI::ItemTextStyle::SetBuiltInAlignment(const StyleKey&, const TextAlignment alignment)
{
    _alignment.SetBuiltIn(alignment);
    resolveLayout();
}

void RetroFuturaGUI::ItemTextStyle::SetBuiltInPadding(const StyleKey&, const f32 padding)
{
    _padding.SetBuiltIn(padding);
    resolveLayout();
}

void RetroFuturaGUI::ItemTextStyle::restyle()
{
    resolveLayout();
}

void RetroFuturaGUI::ItemTextStyle::resolveLayout()
{
    const std::span<const u32> flavors { getFlavors() };
    const bool styled { isStyled() };
    const StylePart part { getStylePart() };

    //Each goes through the layers, so all of them keep up; a value set before and since unset goes back to the widget's default.
    const bool
        isFamilyChanged { nullptr != _fontFamily.Resolve(flavors, styled, part, std::string {}, [](const FlavorPart& style) -> const std::optional<std::string>& { return style.GetFontFamily(); }) },
        isSizeChanged { nullptr != _fontSize.Resolve(flavors, styled, part, FontManager::ToPixels(11.0f, FontSizeUnit::Points), [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetFontSize(); }) },
        isSlantChanged { nullptr != _slant.Resolve(flavors, styled, part, PlatformBridge::Fonts::Slant::Roman, [](const FlavorPart& style) -> const std::optional<PlatformBridge::Fonts::Slant>& { return style.GetFontSlant(); }) },
        isWeightChanged { nullptr != _weight.Resolve(flavors, styled, part, PlatformBridge::Fonts::Weight::Normal, [](const FlavorPart& style) -> const std::optional<PlatformBridge::Fonts::Weight>& { return style.GetFontWeight(); }) },
        isAlignmentChanged { nullptr != _alignment.Resolve(flavors, styled, part, TextAlignment::Left, [](const FlavorPart& style) -> const std::optional<TextAlignment>& { return style.GetAlignment(); }) },
        isPaddingChanged { nullptr != _padding.Resolve(flavors, styled, part, 4.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetPadding(); }) };

    if(!isFamilyChanged && !isSizeChanged && !isSlantChanged && !isWeightChanged && !isAlignmentChanged && !isPaddingChanged)
        return;

    _shownLayout = ItemTextLayout
    {
        ._FontFamily = _fontFamily.GetShown(),
        ._FontSize = _fontSize.GetShown(),
        ._Slant = _slant.GetShown(),
        ._Weight = _weight.GetShown(),
        ._Alignment = _alignment.GetShown(),
        ._Padding = _padding.GetShown()
    };

    if(!_onLayoutChange)
        return;

    _onLayoutChange(_shownLayout);
}