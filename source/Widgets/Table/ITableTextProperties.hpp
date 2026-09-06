#pragma once
#include "IncludeHelper.hpp"
#include <string_view>
#include <string>
#include <span>
#include "PlatformBridge.hpp"
#include "Text.hpp"

namespace RetroFuturaGUI
{
    class ITableTextProperties
    {
    public:
        virtual void SetText(std::string_view text) = 0;
        const std::string& GetText() const;

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Sets the horizontal alignment of the text.
        void SetTextAlignment(const TextAlignment alignment);

        /// @brief Sets the padding applied around the text.
        void SetTextPadding(const f32 padding);

        /// @brief Returns the rendered width of the text content, in pixels.
        f32 GetTextWidth() const;

    protected:
        void setTextColors();

    // Elements
        std::unique_ptr<Text> _text { nullptr };

    // Style    
        std::span<glm::vec4>
            _textColorEnabled {},
            _textColorDisabled {},
            _textColorClicked {},
            _textColorHover {};
        ColorState _textColorState { ColorState::Enabled };
        TextAlignment _textAlignment { TextAlignment::Left };

    // Logic
          
    private:
        inline static const std::string _dummyText { "" };

    };
}