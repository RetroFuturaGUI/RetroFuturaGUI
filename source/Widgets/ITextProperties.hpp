#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Signal.hpp"
#include "Text.hpp"
#include "IWindowAccessor.hpp"
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    //An interface to specialize a widget with text capabilities. It also inherits from IWindowAccessor to let its children 💎✨ Shine bright like a diamond ✨💎
    class ITextProperties : virtual public IWindowAccessor
    {
    public:
        /// @brief Sets the text color for the given color state.
        void SetTextColor(const glm::vec4& color, const ColorState state);

        /// @brief Returns the text color configured for the given color state.
        std::vector<glm::vec4> GetTextColor(const ColorState state) const;

        /// @brief Sets the text content, in UTF-8.
        void SetText(std::string_view text);

        /// @brief Returns the text content, in UTF-8.
        const std::string& GetText() const;

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        virtual void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Sets the horizontal alignment of the text.
        virtual void SetTextAlignment(const TextAlignment alignment);

        /// @brief Sets the padding applied around the text.
        virtual void SetTextPadding(const f32 padding);

        /// @brief Connects a slot to be called when the text content changes.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnTextChange slot.
        void Disconnect_OnTextChange(const typename Signal<>::Slot& slot);

    protected:
        void setTextColors();
        void drawText();

    //Elements
        std::unique_ptr<Text> _text;
        inline static const std::string _dummy {};

    //Logic
        Signal<>
            _onTextChange,
            _onTextChangeAsync;

    // Style
        std::vector<glm::vec4>
            _textColorEnabled { glm::vec4(1.0f) },
            _textColorDisabled { glm::vec4(0.5f, 0.5f,0.5f, 1.0f) },
            _textColorClicked { glm::vec4(1.0f) },
            _textColorHover { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
        ColorState _textColorState { ColorState::Enabled };
        TextAlignment _textAlignment { TextAlignment::Left };

    private:
        void setTextColorElement(std::vector<glm::vec4>& color);
    };
}