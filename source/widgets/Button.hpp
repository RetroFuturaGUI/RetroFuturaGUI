#pragma once
#include "IncludeHelper.hpp"
#include "InputManager.hpp"
#include "LineBorder.hpp"
#include "Rectangle.hpp"
#include "Signal.hpp"
#include "Text.hpp"
#include "IWidget.hpp"

namespace RetroFuturaGUI
{
    enum ColorSetState : u32
    {
        Enabled,
        Disabled,
        Clicked,
        Hover
    };

    class Button : public IWidget
    {
    public:
        Button(const IdentityParams& identity, const GeometryParams2D& geometry, const TextParams& textParams, const BorderParams& borderParams);
        void Draw();
        void Connect_OnClick(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnRelease(const typename Signal<>::Slot& slot, const bool async);
        void Connect_WhileHover(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnMouseEnter(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnMouseLeave(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnEnable(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnDisable(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnClick(const typename Signal<>::Slot& slot);
        void Disconnect_OnRelease(const typename Signal<>::Slot& slot);
        void Disconnect_WhileHover(const typename Signal<>::Slot& slot);
        void Disconnect_OnMouseEnter(const typename Signal<>::Slot& slot);
        void Disconnect_OnMouseLeave(const typename Signal<>::Slot& slot);
        void Disconnect_OnEnable(const typename Signal<>::Slot& slot);
        void Disconnect_OnDisable(const typename Signal<>::Slot& slot);
        void SetEnabled(const bool enable);
        bool IsEnabled() const;
        void SetBackgroundColor(const glm::vec4& color, const ColorSetState state);
        glm::vec4 GetBackgroundColor(const ColorSetState state) const;
        void SetTextColor(const glm::vec4& color, const ColorSetState state);
        glm::vec4 GetTextColor(const ColorSetState state) const;
        void SetLineBorderColor(const glm::vec4& color, const ColorSetState state);
        glm::vec4 GetLineBorderColor(const ColorSetState state) const;

    private:
        //elements
        std::unique_ptr<Rectangle> _rectangle;
        std::unique_ptr<LineBorder> _lineBorder;
        std::unique_ptr<Text> _text;
        
        //logic
        bool 
            _isEnabledFlag { true },
            _wasClicked { false },
            _colorSetFlag { false },
            _mouseEnteredFlag { false },
            _mouseLeftFlag { false };
        Signal<> 
            _onClick,
            _onClickAsync,
            _onRelease,
            _onReleaseAsync,
            _whileHover,
            _whileHoverAsync,
            _onMouseEnter,
            _onMouseEnterAsync,
            _onMouseLeave,
            _onMouseLeaveAsync,
            _onEnable,
            _onEnableAsync,
            _onDisable,
            _onDisableAsync;

        // Style
        glm::vec4 
            _backgroundColorEnabled { 0.5f, 0.5f, 0.5f, 1.0 },
            _backgroundColorDisabled { 0.25f, 0.25f, 0.25f, 1.0 },
            _backgroundColorClicked { 0.6f, 0.6f,0.6f, 1.0 },
            _backgroundColorHover { 0.55f, 0.55f, 0.55f, 1.0 },
            _borderColorEnabled { 0.25f, 0.25f, 0.25f, 1.0 },
            _borderColorDisabled { 0.2f, 0.2f, 0.2f, 1.0 },
            _borderColorClicked { 0.3f, 0.3f, 1.3f, 1.0 },
            _borderColorHover { 0.275f, 0.275f, 0.275f, 1.0 },
            _textColorEnabled { 1.0 },
            _textColorDisabled { 0.5f, 0.5f,0.5f, 1.0 },
            _textColorClicked { 1.0 },
            _textColorHover { 1.0f, 0.0f, 1.0f, 1.0 };
        ColorSetState _state { ColorSetState::Enabled };

        void interact();
        void setColors();
        void setBackgroundColorElement(const glm::vec4& color);
        void setTextColorElement(const glm::vec4& color);
        void setLineBorderColorElement(const glm::vec4& color);
    };
}