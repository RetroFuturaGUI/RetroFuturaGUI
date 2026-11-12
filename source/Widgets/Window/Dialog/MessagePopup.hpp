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
    ///        The kind's look comes from its scope (StyleScope::Message, Warning or Error), whose entries the app can change or
    ///        define first. The accent on the Window's and the MenuBar's border and the title's text (Label:title), and the icon
    ///        (Button:icon, its background image). The library adds them with its own look only when they don't exist yet
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

    private:
        /// @brief Closes the popup on an Enter pressed since it opened, as OK does
        void handleKeys();

        /// @brief Measures the description's lines and, when their height changed since, makes its row, the window and the
        ///        description that tall. Runs each update, so a font a scope or flavor changed lays the popup out again
        void layoutToDescription();

        /// @brief The window's height with a description row this tall.
        static f32 heightFor(const f32 descriptionHeight);

        /// @brief The rows' shares with a description row this tall: bar, summary, description, buttons
        static AxisDefinition axisFor(const f32 descriptionHeight);

        /// @brief The scope a kind styles its widgets in: Message, Warning or Error. Any other kind is a message
        static StyleScope scopeOf(const PopupTypes type);

        /// @brief Message, Warning or Error. Any other kind shows as a message
        static PopupTypes shownType(const PopupTypes type);

        /// @brief What the bar says. "Information", "Warning" or "Error"
        static std::string_view titleOf(const PopupTypes type);

        PopupTypes _type { PopupTypes::Message };
        u32 _enterPressCountSeen { 0 };
        Label* _description { nullptr };
        Lasagna* _lasagna { nullptr };
        f32 _descriptionHeight { 0.0f }; //what its row was last made, from its lines

        inline static uSize _popupCount { 0 }; //names each popup's window and scene apart
        static constexpr uSize _columnCount { 12 };
        static constexpr f32
            _width { 480.0f },
            _barHeight { 40.0f },
            _summaryHeight { 52.0f },
            _buttonRowHeight { 56.0f },
            _iconSide { 32.0f },
            _shortestDescription { 24.0f }; //an empty description still leaves a little room above the button
        static constexpr ViewCell _descriptionCell { ._Row = 2, ._Column = 0, ._ColumnSpan = _columnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
    };
}