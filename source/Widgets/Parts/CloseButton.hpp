#pragma once
#include "IncludeHelper.hpp"
#include "Background.hpp"
#include "Icon.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    /// @brief The close button on every field of a WindowTab: a background, transparent until hovered by default, and the cross
    ///        over it. The WindowTab owns it and hands it out through GetCloseButton(), so it is styled as
    ///        windowTab.GetCloseButton().GetBackground().SetColors(...). One button draws every field's, in that field's state
    class CloseButton final
    {
    public:
        /// @param icon The cross's own SVG, from ResourceManager
        CloseButton(Projection* projection, std::unique_ptr<SvgTexture> icon);
        CloseButton() = delete;
        CloseButton(const CloseButton&) = delete;
        CloseButton(CloseButton&&) = delete;
        ~CloseButton() = default;
        auto operator =(const CloseButton&) = delete;
        auto operator =(CloseButton&&) = delete;

        /// @brief Returns the button's background
        Background& GetBackground();

        /// @brief Returns the button's background
        const Background& GetBackground() const;

        /// @brief Returns the cross
        Icon& GetIcon();

        /// @brief Returns the cross
        const Icon& GetIcon() const;

    private:
        Background _background;
        Icon _icon;
    };
}