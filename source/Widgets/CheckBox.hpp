#pragma once
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IncludeHelper.hpp"
#include "Parts/Background.hpp"
#include "Plane.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "SvgTexture.hpp"
#include "Icon.hpp"
#include "ResourceManager.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    /// @brief A widget that visually represents a boolean
    class CheckBox final : public IWidget, public IClickable
    {
    public:
        /// @brief Constructs a CheckBox widget under the given parent widget/window.
        CheckBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        CheckBox(const CheckBox&) = delete;
        CheckBox(CheckBox&&) = delete;
        ~CheckBox() = default;
        auto operator =(const CheckBox&) = delete;
        auto operator =(CheckBox&&) = delete;

        /// @brief Draws the CheckBox, including its inherited-value fill, border and checkmark
        void Draw() override;

        void SetValue(const bool value, const bool emitSignal);
        void SetInheritValueReference(bool* ref);
        bool GetValue() const;

        /// @brief Sets multiple inherit state fill colors (e.g. gradient stops) for the given color state.
        void SetInheritColors(std::span<const glm::vec4> colors, const ColorState state);

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

        /// @brief Returns the CheckBox's background
        /// @brief Returns the checkmark drawn while the box is checked
        Icon& GetCheckmark();

        /// @brief Returns the checkmark drawn while the box is checked
        const Icon& GetCheckmark() const;

        Background& GetBackground();

        /// @brief Returns the CheckBox's background
        const Background& GetBackground() const;

        /// @brief Returns the CheckBox's border
        Border& GetBorder();

        /// @brief Returns the CheckBox's border
        const Border& GetBorder() const;

    protected:
        /// @brief Sets the size of the CheckBox.
        void applySize(const glm::vec3& size) override;

        /// @brief Sets the world position of the CheckBox.
        void applyPosition(const glm::vec3& position) override;

    private:
        void setColors(const ColorState state) override;
        void setInheritFillColors();
        void interact() override;
        void updateInheritFillShape();

        std::unique_ptr<Plane> _inheritFill { nullptr };
        Icon _checkmark //its own, so its colors and place are this box's alone
        {
            ResourceManager::CreateCheckmarkIcon(&_projection),
            "CheckmarkPath",
            SvgPathFill { ._FillType = FillType::Solid, ._Colors = {}, ._GradientDegree = 35.0f, ._GradientOffset = 0.0f, ._GradientAnimationSpeed = 0.05f, ._GradientRotationSpeed = 5.0f },
            IconColors
            {
                ._Enabled = { glm::vec4(0.7f, 0.7f, 0.7f, 1.0f) },
                ._Disabled = { glm::vec4(0.3f, 0.3f, 0.3f, 1.0f) },
                ._Clicked = { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
                ._Hover = { glm::vec4(0.85f, 0.85f, 0.85f, 1.0f) },
                ._Selected = {}
            }
        };
        Background _background { &_projection, { ._OnCornerRadiiChange = [this]() { updateInheritFillShape(); } } }; //the inherit fill has the same corners
        Border _border { &_projection };
        bool* _inheritValue { nullptr };
        bool
            _useInheritValue { false },
            _isChecked { false };
        f32 _innerPadding { 2.0f };
        ColorState
            _inheritFillColorState { ColorState::Enabled };
        std::vector<glm::vec4>
            _inheritFillColorsEnabled { { 0.25f, 0.25f, 0.8f, 1.0f } },
            _inheritFillColorsDisabled { { 0.1f, 0.1f, 0.45f, 1.0f } },
            _inheritFillColorsHover { { 0.3f, 0.3f, 0.9f, 1.0f } },
            _inheritFillColorsClicked { { 0.4f, 0.4f, 1.0f, 1.0f } };
        Signal<>
            _onSetValue,
            _onSetValueAsync,
            _onValueChanged,
            _onValueChangedAsync;
    };
}