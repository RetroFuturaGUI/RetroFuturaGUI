#pragma once
#include "IncludeHelper.hpp"
#include "DialogKit.hpp"
#include "Popup.hpp"
#include <string_view>

namespace RetroFuturaGUI
{
    /// @brief A message, warning or error: the kind in the bar, an icon and a bold one-line summary, and a description that
    ///        wraps, the popup as tall as it needs. OK and Enter close it (OnResult). The bar's close button and Escape dismiss
    ///        it (OnDismiss). Opened by Window::OpenPopup.
    ///        The kind's look comes from two flavors, which the app can change or define first. The accent (GetFlavorName:
    ///        the window's and the bar's border, the title's text) and the icon (GetIconFlavorName: its background image).
    ///        The library adds them with its own look only when they don't exist yet
    class MessagePopup final : public Popup<>, protected DialogKit
    {
    public:
        /// @param type Message, Warning or Error; any other shows as a message
        /// @param summary The bold first line, cut where it leaves the window. Empty leaves the icon alone on its row
        /// @param description The text below it, wrapped to the window's width
        MessagePopup(const PopupTypes type, std::string_view summary, std::string_view description);
        MessagePopup() = delete;
        MessagePopup(const MessagePopup&) = delete;
        MessagePopup(MessagePopup&&) = delete;
        auto operator =(const MessagePopup&) = delete;
        auto operator =(MessagePopup&&) = delete;
        ~MessagePopup() override = default;

        /// @brief Returns the kind it shows. Message, Warning or Error
        PopupTypes GetType() const;

        /// @brief Returns the name of the flavor giving a kind its accent. "popupInformation", "popupWarning" or "popupError"
        static std::string_view GetFlavorName(const PopupTypes type);

        /// @brief Returns the name of the flavor giving a kind its icon. The accent flavor's name followed by "Icon"
        static std::string_view GetIconFlavorName(const PopupTypes type);

    private:
        /// @brief Closes the popup on an Enter pressed since it opened, as OK does
        void handleKeys();

        /// @brief Adds the kinds' flavors with the library's look, each only when the app hasn't defined it
        static void addMissingFlavors();

        /// @brief Message, Warning or Error. Any other kind shows as a message
        static PopupTypes shownType(const PopupTypes type);

        /// @brief What the bar says. "Information", "Warning" or "Error"
        static std::string_view titleOf(const PopupTypes type);

        PopupTypes _type { PopupTypes::Message };
        u32 _enterPressCountSeen { 0 };

        inline static uSize _popupCount { 0 }; //names each popup's window and scene apart
        static constexpr uSize _columnCount { 12 };
        static constexpr f32
            _width { 480.0f },
            _barHeight { 40.0f },
            _summaryHeight { 52.0f },
            _buttonRowHeight { 56.0f },
            _iconSide { 32.0f },
            _shortestDescription { 24.0f }; //an empty description still leaves a little room above the button
    };
}