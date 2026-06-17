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
        void SetTextColor(const glm::vec4& color, const ColorState state);
        std::vector<glm::vec4> GetTextColor(const ColorState state) const;
        void SetText(std::string_view text);
        const std::string& GetText() const;
        virtual void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);
        void SetTextAlignment(const TextAlignment alignment);
        void SetTextPadding(const f32 padding);

    protected:
        void setTextColors();
        void drawText();

    //Elements
        std::unique_ptr<Text> _text;

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