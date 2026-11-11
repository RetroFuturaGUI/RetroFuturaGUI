#pragma once
#include "IncludeHelper.hpp"
#include "Background.hpp"
#include "Icon.hpp"
#include <memory>
#include <string_view>

namespace RetroFuturaGUI
{
    /// @brief A small button drawn as a background, transparent until hovered by default, with an icon over it. A tab field's
    ///        close button, a tab strip's scroll arrows. Its owner hands it out through a getter, so it is styled as
    ///        tab.GetCloseButton().GetBackground().SetColors(...). One button draws every place it shows, each in its own state.
    class IconButton final
    {
    public:
        /// @param icon The icon's own SVG, from ResourceManager
        /// @param pathName The SVG path the icon's colors fill
        IconButton(Projection* projection, std::unique_ptr<SvgTexture> icon, std::string_view pathName);
        IconButton() = delete;
        IconButton(const IconButton&) = delete;
        IconButton(IconButton&&) = delete;
        ~IconButton() = default;
        auto operator =(const IconButton&) = delete;
        auto operator =(IconButton&&) = delete;

        /// @brief Returns the button's background
        Background& GetBackground();

        /// @brief Returns the button's background
        const Background& GetBackground() const;

        /// @brief Returns the icon
        Icon& GetIcon();

        /// @brief Returns the icon
        const Icon& GetIcon() const;

    private:
        Background _background;
        Icon _icon;
    };
}