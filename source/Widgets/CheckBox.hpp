#pragma once
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IncludeHelper.hpp"
#include "Plane.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "SvgTexture.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    /// @brief A widget that visually represents a boolean
    class CheckBox final : public IWidget, public IClickable, public IBackground, public IBorder
    {
    public:
        /// @brief Constructs a CheckBox widget under the given parent widget/window.
        CheckBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        CheckBox(const CheckBox&) = delete;
        CheckBox(CheckBox&&) = delete;
        ~CheckBox() = default;
        auto operator =(const CheckBox&) = delete;
        auto operator =(CheckBox&&) = delete;

        /// @brief Draws the button, including its Inherit, border and label text.
        void Draw() override;

        /// @brief Sets the size of the button.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the button.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the button.
        void SetRotation(const glm::vec3& rotation) override;
        void SetValue(const bool value, const bool emitSignal);
        void SetInheritValueReference(bool* ref);
        bool GetValue() const;
        void SetCheckmarkColors(std::span<glm::vec4> colors, const ColorState state);
        
        /// @brief Sets the corner rounding radii of the CheckBox' background, inherit field and border.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets multiple inherit state fill colors (e.g. gradient stops) for the given color state.
        void SetInheritColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the offset applied to the inherit state fill gradient's start position.
        void SetInheritGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the inherit state fill gradient animates over time.
        void SetInheritGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the inherit state fill's linear gradient, in degrees.
        void SetInheritGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the inherit state fill gradient rotates over time.
        void SetInheritGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets whether the CheckBox mirrors the bool given to SetInheritValueReference instead of keeping a value of its own.
        void EnableInheritedValue(const bool enable);

        /// @brief Sets the distance between the border and checkmark in pixels
        /// @param a float extressing the distance in pixels
        void SetInnerPadding(const f32 pixels);

    private:
        void setColors(const ColorState state) override;
        void setCheckmarkColors();
        void setInheritFillColors();
        void interact();

        std::unique_ptr<Plane> _inheritFill { nullptr };
        std::shared_ptr<SvgTexture> _checkmark { nullptr };
        bool* _inheritValue { nullptr };
        bool
            _useInheritValue { false },
            _isChecked { false };
        f32 _innerPadding { 2.0f };
        glm::vec2 _checkmarkSize { 0.0f };
        ColorState
            _checkmarkColorState { ColorState::Enabled },
            _inheritFillColorState { ColorState::Enabled };
        std::vector<glm::vec4>
            _checkmarkColorsEnabled { { 0.7f, 0.7f, 0.7f, 1.0f } },
            _checkmarkColorsDisabled { { 0.3f, 0.3f, 0.3f, 1.0f } },
            _checkmarkColorsHover { { 0.85f, 0.85f, 0.85f, 1.0f } },
            _checkmarkColorsClicked { { 1.0f, 1.0f, 1.0f, 1.0f } },
            _inheritFillColorsEnabled { { 0.25f, 0.25f, 0.8f, 1.0f } },
            _inheritFillColorsDisabled { { 0.1f, 0.1f, 0.45f, 1.0f } },
            _inheritFillColorsHover { { 0.3f, 0.3f, 0.9f, 1.0f } },
            _inheritFillColorsClicked { { 0.4f, 0.4f, 1.0f, 1.0f } };
        SvgPathFill _checkmarkPathFill
        {
            SvgPathFill
            {
                ._FillType = FillType::Solid,
                ._Colors = { glm::vec4(1.0f) },
                ._GradientDegree = 35.0f,
                ._GradientOffset = 0.0f,
                ._GradientAnimationSpeed = 0.05f,
                ._GradientRotationSpeed = 5.0f
            }
        };
        Signal<>
            _onSetValue,
            _onSetValueAsync,
            _onValueChanged,
            _onValueChangedAsync;
    };
}