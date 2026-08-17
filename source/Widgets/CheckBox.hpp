#pragma once
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "SvgTexture.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    // A widget that visually represents a boolean
    class CheckBox : public IWidget, public IClickable, public IBackground, public IBorder
    {
    public:
        /// @brief Constructs a CheckBox widget under the given parent widget/window.
        CheckBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        CheckBox(const CheckBox&) = delete;
        CheckBox(CheckBox&&) = delete;
        ~CheckBox() = default;
        auto operator =(const CheckBox&) = delete;
        auto operator =(CheckBox&&) = delete;

        /// @brief Draws the button, including its Inheriet, border and label text.
        void Draw() override;

        /// @brief Enables or disables the button, optionally emitting the associated signal.
        void SetEnabled(const bool enable, const bool emitSignal = true);

        /// @brief Sets the size of the button.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the button.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the button.
        void SetRotation(const glm::vec3& rotation) override;
        void SetValue(const bool value, const bool emitSignal = true);
        void SetInheritValueReference(bool* ref);
        bool GetValue() const;
        void SetCheckmarkColors(std::span<glm::vec4> colors, const ColorState state);
        
        /// @brief Sets the corner rounding radii of the CheckBox' background, inheriet field and border.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets multiple inheriet state fill colors (e.g. gradient stops) for the given color state.
        void SetInherietColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the offset applied to the inheriet state fill gradient's start position.
        void SetInherietGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the inheriet state fill gradient animates over time.
        void SetInherietGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the inheriet state fill's linear gradient, in degrees.
        void SetInherietGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the inheriet state fill gradient rotates over time.
        void SetInherietGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets whether an inherieted value should be used.
        /// @param a boolean value whether to use an inherieted value
        void UseInherietedValue(const bool useInheriet);

        /// @brief Sets the distance between the border and checkmark in pixels
        /// @param a float extressing the distance in pixels
        void SetInnerPadding(const f32 pixels);

    private:
        void setColors(const ColorState state);
        void setCheckmarkColors();
        void setInherietFillColors();
        void interact();

        std::unique_ptr<Rectangle> _inherietFill { nullptr };
        std::shared_ptr<SvgTexture> _checkmark { nullptr };
        bool* _inherietValue { nullptr };
        bool
            _useInherietValue { false },
            _isChecked { false };
        f32 _innerPadding { 2.0f };
        ColorState
            _checkmarkColorState { ColorState::Enabled },
            _inherietFillColorState { ColorState::Enabled };
        std::vector<glm::vec4>
            _checkmarkColorsEnabled { { 0.7f, 0.7f, 0.7f, 1.0f } },
            _checkmarkColorsDisabled { { 0.3f, 0.3f, 0.3f, 1.0f } },
            _checkmarkColorsHover { { 0.85f, 0.85f, 0.85f, 1.0f } },
            _checkmarkColorsClicked { { 1.0f, 1.0f, 1.0f, 1.0f } },
            _inherietFillColorsEnabled { { 0.25f, 0.25f, 0.8f, 1.0f } },
            _inherietFillColorsDisabled { { 0.1f, 0.1f, 0.45f, 1.0f } },
            _inherietFillColorsHover { { 0.3f, 0.3f, 0.9f, 1.0f } },
            _inherietFillColorsClicked { { 0.4f, 0.4f, 1.0f, 1.0f } };
        SvgPathFill _checkmarkPathFill
        {
            SvgPathFill
            {
                .fillType = FillType::SOLID,
                .colors = { glm::vec4(1.0f) },
                .gradientDegree = 35.0f,
                .gradientOffset = 0.0f,
                .gradientAnimationSpeed = 0.05f,
                .gradientRotationSpeed = 5.0f
            }
        };
        Signal<>
            _onSetValue,
            _onSetValueAsync,
            _onValueChanged,
            _onValueChangedAsync;
    };
}