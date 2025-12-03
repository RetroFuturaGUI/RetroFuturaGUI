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
        bool _isEnabledFlag = true;
        bool _wasClicked = false;
        bool _colorSetFlag = false;
        bool _mouseEnteredFlag = false;
        bool _mouseLeftFlag = false;
        ColorSetState _state = Enabled;
        Signal<> _onClick;
        Signal<> _onClickAsync;
        Signal<> _onRelease;
        Signal<> _onReleaseAsync;
        Signal<> _whileHover;
        Signal<> _whileHoverAsync;
        Signal<> _onMouseEnter;
        Signal<> _onMouseEnterAsync;
        Signal<> _onMouseLeave;
        Signal<> _onMouseLeaveAsync;
        Signal<> _onEnable;
        Signal<> _onEnableAsync;
        Signal<> _onDisable;
        Signal<> _onDisableAsync;

        // Style
        glm::vec4 _backgroundColorEnabled = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        glm::vec4 _backgroundColorDisabled = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
        glm::vec4 _backgroundColorClicked = glm::vec4(0.6f, 0.6f,0.6f, 1.0f);
        glm::vec4 _backgroundColorHover = glm::vec4(0.55f, 0.55f, 0.55f, 1.0f);
        glm::vec4 _borderColorEnabled = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
        glm::vec4 _borderColorDisabled = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        glm::vec4 _borderColorClicked = glm::vec4(0.3f, 0.3f, 1.3f, 1.0f);
        glm::vec4 _borderColorHover = glm::vec4(0.275f, 0.275f, 0.275f, 1.0f);
        glm::vec4 _textColorEnabled = glm::vec4(1.0f);
        glm::vec4 _textColorDisabled = glm::vec4(0.5f, 0.5f,0.5f, 1.0f);
        glm::vec4 _textColorClicked = glm::vec4(1.0f);
        glm::vec4 _textColorHover = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

        void interact();
        void setColors();
        void setBackgroundColorElement(const glm::vec4& color);
        void setTextColorElement(const glm::vec4& color);
        void setLineBorderColorElement(const glm::vec4& color);
    };
}