#pragma once
#include "FileExplorer.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace RetroFuturaGUI
{
    class CheckBox;

    /// @brief Asks for a name
    class FileExplorer::NamePrompt final : public Popup<std::string>
    {
    public:
        NamePrompt(std::string_view title, std::string_view message, std::string_view name, const ViewFont& font);
        NamePrompt() = delete;
        NamePrompt(const NamePrompt&) = delete;
        NamePrompt(NamePrompt&&) = delete;
        auto operator =(const NamePrompt&) = delete;
        auto operator =(NamePrompt&&) = delete;
        ~NamePrompt() override = default;

        /// @brief Gives the keys to the name box, whose text is selected, so typing replaces the suggestion.
        void TakeKeys();

    private:
        void update();
        void confirm();

        TextBox* _name { nullptr };
        KeyPress _escapeKey { ._Key = PB_KEY_ESCAPE, ._PressCountSeen = 0 };

        static constexpr i32
            _width { 460 },
            _height { 200 };
        inline static const AxisDefinition _axis
        {
            ._RowDefinition = { 0.2f, 0.26f, 0.26f, 0.28f }, //bar, message, name, buttons
            ._ColumnDefinition = std::vector<f32>(12, 1.0f / 12.0f),
            ._LayerDefinition = { 1.0f }
        };
    };

    /// @brief Asks to confirm
    class FileExplorer::ConfirmPrompt final : public Popup<>
    {
    public:
        ConfirmPrompt(std::string_view title, std::string_view message, std::string_view detail, std::string_view confirmText, const ViewFont& font);
        ConfirmPrompt() = delete;
        ConfirmPrompt(const ConfirmPrompt&) = delete;
        ConfirmPrompt(ConfirmPrompt&&) = delete;
        auto operator =(const ConfirmPrompt&) = delete;
        auto operator =(ConfirmPrompt&&) = delete;
        ~ConfirmPrompt() override = default;

        /// @brief Makes Enter and Escape arrive at the prompt's window.
        void TakeKeys();

    private:
        void update();

        KeyPress
            _enterKey { ._Key = PB_KEY_RETURN, ._PressCountSeen = 0 },
            _escapeKey { ._Key = PB_KEY_ESCAPE, ._PressCountSeen = 0 };

        static constexpr i32
            _width { 520 },
            _height { 200 };
        inline static const AxisDefinition _axis
        {
            ._RowDefinition = { 0.2f, 0.26f, 0.26f, 0.28f }, //bar, message, detail, buttons
            ._ColumnDefinition = std::vector<f32>(12, 1.0f / 12.0f),
            ._LayerDefinition = { 1.0f }
        };
    };

    /// @brief Asks what to do about a name clash: Replace, Skip or Keep both, for this file or with the box checked for every
    ///        clash left. Enter keeps both; Escape and the bar's close button dismiss it, which stops the transfer.
    class FileExplorer::ClashPrompt final : public Popup<ClashAnswer, ClashScope>
    {
    public:
        ClashPrompt(std::string_view name, std::string_view folder, const ViewFont& font);
        ClashPrompt() = delete;
        ClashPrompt(const ClashPrompt&) = delete;
        ClashPrompt(ClashPrompt&&) = delete;
        auto operator =(const ClashPrompt&) = delete;
        auto operator =(ClashPrompt&&) = delete;
        ~ClashPrompt() override = default;

        /// @brief Makes Enter and Escape arrive at the prompt's window.
        void TakeKeys();

    private:
        void update();
        void answer(const ClashAnswer clashAnswer);

        CheckBox* _forRemaining { nullptr };
        KeyPress
            _enterKey { ._Key = PB_KEY_RETURN, ._PressCountSeen = 0 },
            _escapeKey { ._Key = PB_KEY_ESCAPE, ._PressCountSeen = 0 };

        static constexpr i32
            _width { 560 },
            _height { 220 };
        inline static const AxisDefinition _axis
        {
            ._RowDefinition = { 0.18f, 0.26f, 0.26f, 0.3f }, //bar, message, "for the remaining clashes", buttons
            ._ColumnDefinition = std::vector<f32>(12, 1.0f / 12.0f),
            ._LayerDefinition = { 1.0f }
        };
    };
}