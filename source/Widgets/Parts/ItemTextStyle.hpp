#pragma once
#include "IncludeHelper.hpp"
#include "StyledPart.hpp"
#include "ShaderLayers.hpp"
#include "FontManager.hpp"
#include "TextLayout.hpp"
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The font, alignment and padding a list's item texts show, each unset while nothing sets it, so the widget keeps
    ///        its own default for it
    struct ItemTextLayout
    {
        std::optional<std::string> _FontFamily {};
        std::optional<f32> _FontSize {}; //in pixels.
        std::optional<PlatformBridge::Fonts::Slant> _Slant {};
        std::optional<PlatformBridge::Fonts::Weight> _Weight {};
        std::optional<TextAlignment> _Alignment {};
        std::optional<f32> _Padding {};
    };

    /// @brief The look of a list's item texts: their colors per state (Enabled, Disabled for disabled items, Hover for the
    ///        hovered row and Clicked for the picked one), their font, alignment and padding. The widget owns it and hands it out
    ///        through GetItemText(), so it is styled as comboBox.GetItemText().SetColors(...)
    class ItemTextStyle final : public IStyledPart
    {
    public:
        /// @param onLayoutChange called with what shows whenever the font, alignment or padding shown changed, so the widget lays
        ///        its texts out again.
        explicit ItemTextStyle(std::function<void(const ItemTextLayout&)> onLayoutChange = {});

        /// @brief Sets the colors for the given state, over every flavor: one for a solid fill, the stops for a gradient. None leave
        ///        the state to the widget's flavors and the built-in colors; a state none of them sets shows the Enabled colors
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors set on the item texts themselves for the given state, empty when unset
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Sets the item texts' font over every flavor: family, size, slant and weight
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Sets the item texts' alignment over every flavor.
        void SetAlignment(const TextAlignment alignment);

        /// @brief Sets the space between an item text and the edges it is aligned to, over every flavor
        void SetPadding(const f32 padding);

        /// @brief Returns what the font, alignment and padding show
        const ItemTextLayout& GetShownLayout() const;

    //Called by the owning widget
        /// @brief The colors an item shows in the given state: the own ones, else the flavors', else the built-in ones
        const std::vector<glm::vec4>& GetShownColors(const ColorState state) const;

        /// @brief Set the built-in font, alignment and padding, which the item texts' own settings and the flavors override
        void SetBuiltInFontFamily(const StyleKey& key, std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);
        void SetBuiltInAlignment(const StyleKey& key, const TextAlignment alignment);
        void SetBuiltInPadding(const StyleKey& key, const f32 padding);

    private:
        /// @brief The colors are read when drawn; the font, alignment and padding are taken through the layers again
        void restyle() override;

        /// @brief Takes the font, alignment and padding through the layers and tells the widget when what shows changed
        void resolveLayout();

        LayeredColors _colors {};
        LayeredSetting<std::string> _fontFamily {};
        LayeredSetting<f32>
            _fontSize {}, //in pixels
            _padding {};
        LayeredSetting<PlatformBridge::Fonts::Slant> _slant {};
        LayeredSetting<PlatformBridge::Fonts::Weight> _weight {};
        LayeredSetting<TextAlignment> _alignment {};
        ItemTextLayout _shownLayout {};
        std::function<void(const ItemTextLayout&)> _onLayoutChange {};
    };
}