#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Signal.hpp"
#include "Text.hpp"
#include "IWindowAccessor.hpp"

namespace RetroFuturaGUI
{
    class ITextProperties : virtual public IWindowAccessor
    {
    public:
    //Text
        void SetTextColor(const glm::vec4& color, const ColorState state);
        std::vector<glm::vec4> GetTextColor(const ColorState state) const;
        void SetText(std::string_view text);
        void SetReadOnly(const bool readOnly);
        bool IsReadOnly() const;
        const std::string& GetText() const;

    protected:
        void setTextColors();
        void editText();
        void drawText();

    //Elements
        std::unique_ptr<Text> _text;

    //Logic
        Signal<>
            _onTextChange,
            _onTextChangeAsync;
        bool
            _textChangedFlag { false },
            _readOnly { false },
            _editingEnabled { false },
            _keyWasReleased { true };
        u32 _keyHoldFrames { 0 };
        std::string _keyRepeatText {};
        std::vector<char> _prevKeyStates {};

    // Style
        std::vector<glm::vec4>
            _textColorEnabled { glm::vec4(1.0f) },
            _textColorDisabled { glm::vec4(0.5f, 0.5f,0.5f, 1.0f) },
            _textColorClicked { glm::vec4(1.0f) },
            _textColorHover { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
        ColorState _textColorState { ColorState::Enabled };
        TextAlignment _textAlignment { TextAlignment::LEFT };

    private:
        void setTextColorElement(std::vector<glm::vec4>& color);
    };
}