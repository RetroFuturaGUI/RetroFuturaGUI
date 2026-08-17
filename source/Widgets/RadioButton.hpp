#pragma once
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "RadioButtonGroup.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    // A radiobutton
    class RadioButton : public IWidget, public IClickable, public IBackground, public IBorder
    {
    public:
        /// @brief Constructs a RadioButton widget under the given parent widget/window.
        RadioButton(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, RadioButtonGroup* parentGroup);
        RadioButton(const RadioButton&) = delete;
        RadioButton(RadioButton&&) = delete;
        ~RadioButton() = default;
        auto operator =(const RadioButton&) = delete;
        auto operator =(RadioButton&&) = delete;

        /// @brief Draws the button, including its Inheriet, border and label text.
        void Draw() override;

        /// @brief Enables or disables the RadioButton, optionally emitting the associated signal.
        void SetEnabled(const bool enable, const bool emitSignal = true);

        /// @brief Sets the size of the button.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the button.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the button.
        void SetRotation(const glm::vec3& rotation) override;
        void SetValue(const bool value, const bool emitSignal = true);
        void SetParentGroup(std::shared_ptr<RadioButtonGroup> parentGroup);
        bool GetValue() const;
        
        /// @brief Sets the corner rounding radii of the RadioButton' background, Indicator field and border.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets multiple Indicator state fill colors (e.g. gradient stops) for the given color state.
        void SetIndicatorColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the offset applied to the Indicator state fill gradient's start position.
        void SetIndicatorGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the Indicator state fill gradient animates over time.
        void SetIndicatorGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the Indicator state fill's linear gradient, in degrees.
        void SetIndicatorGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the Indicator state fill gradient rotates over time.
        void SetIndicatorGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the distance between border and Indicator in pixels
        /// @param a float extressing the distance in pixels
        void SetIndicatorPadding(const f32 pixels);

    private:
        void setColors(const ColorState state);
        void setIndicatorColors();
        void interact();

        std::unique_ptr<Rectangle> _indicator { nullptr };
        RadioButtonGroup* _parentGroup { nullptr };
        bool _isChecked { false };
        f32 _indicatorPadding { 2.0f };
        ColorState _indicatorColorState { ColorState::Enabled };
        std::vector<glm::vec4>
            _indicatorColorsEnabled { { 0.25f, 0.25f, 0.8f, 1.0f } },
            _indicatorColorsDisabled { { 0.1f, 0.1f, 0.45f, 1.0f } },
            _indicatorColorsHover { { 0.3f, 0.3f, 0.9f, 1.0f } },
            _indicatorColorsClicked { { 0.4f, 0.4f, 1.0f, 1.0f } };
            
        Signal<>
            _onSetValue,
            _onSetValueAsync,
            _onValueChanged,
            _onValueChangedAsync;
    };
}