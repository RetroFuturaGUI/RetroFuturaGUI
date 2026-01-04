#pragma once
#include "IncludeHelper.hpp"
#include "InputManager.hpp"
#include "Rectangle.hpp"
#include "Text.hpp"
#include "IWidget.hpp"

namespace RetroFuturaGUI
{
    class Button : public IWidget
    {
    public:
        Button(const IdentityParams& identity, const GeometryParams2D& geometry, const TextParams& textParams, const float borderWidth = 5.0f);
        void Draw();
        void Connect_OnClick(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnRelease(const typename Signal<>::Slot& slot, const bool async);
        void Connect_WhileHover(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnMouseEnter(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnMouseLeave(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnClick(const typename Signal<>::Slot& slot);
        void Disconnect_OnRelease(const typename Signal<>::Slot& slot);
        void Disconnect_WhileHover(const typename Signal<>::Slot& slot);
        void Disconnect_OnMouseEnter(const typename Signal<>::Slot& slot);
        void Disconnect_OnMouseLeave(const typename Signal<>::Slot& slot);
        void SetEnabled(const bool enable);

        //Background
        void SetBackgroundColor(const glm::vec4& color, const ColorState state);
        void SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state);
        void SetBackgroundGradientOffset(const f32 gradientOffset);
        void SetBackgroundGradientAnimationSpeed(const f32 animationSpeed);
        void SetBackgroundGradientDegree(const f32 degree);
        void SetBackgroundGradientRotationSpeed(const f32 rotationSpeed);
        void SetWindowBackgroundImageTextureID(const u32 textureID);
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;

        //Border
        void SetBorderColor(const glm::vec4& color, const ColorState state);
        void SetBorderColors(std::span<glm::vec4> colors, const ColorState state);
        void SetBorderGradientOffset(const f32 gradientOffset);
        void SetBorderGradientAnimationSpeed(const f32 animationSpeed);
        void SetBorderGradientDegree(const f32 degree);
        void SetBorderGradientRotationSpeed(const f32 rotationSpeed);
        void SetWindowBorderImageTextureID(const u32 textureID);
        const std::vector<glm::vec4>& GetBorderColor(const ColorState state) const;
        
        //Geometry
        void SetSize(const glm::vec2& size) override;
        void SetPosition(const glm::vec2& position) override;
        void SetRotation(const float rotation) override;
        void SetCornerRadii(const glm::vec4& radii);

        //Text
        void SetTextColor(const glm::vec4& color, const ColorState state);
        std::vector<glm::vec4> GetTextColor(const ColorState state) const;

    private:
        //elements
        std::unique_ptr<Rectangle> _rectangle;
        std::unique_ptr<Rectangle> _border;
        std::unique_ptr<Text> _text;
        
        //logic
        bool 
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
            _onMouseLeaveAsync;

        // Style
        std::vector<glm::vec4> 
            _backgroundColorEnabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _backgroundColorDisabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _backgroundColorClicked { glm::vec4(0.6f, 0.6f,0.6f, 1.0f) },
            _backgroundColorHover { glm::vec4(0.55f, 0.55f, 0.55f, 1.0f) },
            _borderColorEnabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _borderColorDisabled { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) },
            _borderColorClicked { glm::vec4(0.3f, 0.3f, 1.3f, 1.0f) },
            _borderColorHover { glm::vec4(0.275f, 0.275f, 0.275f, 1.0f) },
            _textColorEnabled { glm::vec4(1.0f) },
            _textColorDisabled { glm::vec4(0.5f, 0.5f,0.5f, 1.0f) },
            _textColorClicked { glm::vec4(1.0f) },
            _textColorHover { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };

        void interact();
        void setColors();
        void setBackgroundColorElement(std::vector<glm::vec4>& color);
        void setTextColorElement(const glm::vec4& color);
        void setBorderColorElement(std::vector<glm::vec4>& color);
    };
}