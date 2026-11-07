#pragma once
#include "IncludeHelper.hpp"
#include <string_view>
#include <string>
#include <span>
#include "Fonts.hpp"
#include "Text.hpp"

namespace RetroFuturaGUI
{
    class Table;

    class ITableTextProperties
    {
        friend class Table;

    public:
        virtual void SetText(std::string_view text) = 0;
        const std::string& GetText() const;

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Returns the font size in the given unit.
        f32 GetFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets the horizontal alignment of the text.
        void SetTextAlignment(const TextAlignment alignment);

        /// @brief Sets the padding applied around the text.
        void SetTextPadding(const f32 padding);

        /// @brief Returns the rendered width of the text content, in pixels.
        f32 GetTextWidth() const;

        /// @brief Shows the colors of the given state. Selected colors are optional. Without any, the Selected state shows the Enabled colors
        void SetTextColorState(const ColorState state);

    protected:
        void setTextColors();

        /// @brief Applies the kept font, alignment, padding and colors to a Text just built, so it draws as the one it replaces.
        void applyTextSettings();

    // Elements
        std::unique_ptr<Text> _text { nullptr }; //built for drawing only, while the cell is in view: a table can hold thousands of cells.
        std::string _content {}; //the text while there is no Text, and what it is built with

    // Style
        std::span<const glm::vec4>
            _textColorEnabled {},
            _textColorDisabled {},
            _textColorClicked {},
            _textColorHover {},
            _textColorSelected {};
        ColorState _textColorState { ColorState::Enabled };
        TextAlignment _textAlignment { TextAlignment::Left };
        std::string _fontFamily {}; //empty: Text's own default
        f32
            _fontSize { FontManager::ToPixels(16.0f, FontSizeUnit::Points) },
            _textPadding { 0.0f };
        PlatformBridge::Fonts::Slant _slant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };

    private:
        inline static const std::string _dummyText { "" };
    };
}