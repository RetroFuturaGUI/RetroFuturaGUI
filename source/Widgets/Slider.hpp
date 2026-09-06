#pragma once
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Texture.hpp"
#include "config.hpp"
#include <glm/ext/vector_int2_sized.hpp>
#include <memory>
#include "Button.hpp"
#include <span>
#include "IRangedValue.hpp"

namespace RetroFuturaGUI
{
    enum class ElementSizing : u32
    {
        Pixels,
        Percent
    };

    // A slider which value can be set by dragging its indicator
    class Slider : public IWidget, public IClickable, public IBackground, public IBorder, public IRangedValue
    {
    public:
    /// @brief Constructs the Slider
        Slider(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~Slider() = default;
        Slider(const Slider&) = delete;
        Slider(Slider&&) = delete;
        auto operator =(const Slider&) = delete;
        auto operator =(Slider&&) = delete;

        /// @brief Draws the Slider
        void Draw() override;

        /// @brief Sets the Slider's position
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the Slider's size
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the Slider's Rotation
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Sets whether the Slider runs horizontally or vertically at 0° rotation.
        void SetOrientation(const Orientation orientation) override;

        /// @brief Sets the Slider's indicator's size along the slider's length
        /// @param size (f32): slider's length
        /// @param sizingMode (ElementSizing): Whether the size is set in pixelsize (abolute) or in percent (relative to the Slider's total length)
        void SetIndicatorSize(const f32 size, const ElementSizing sizingMode);

        /// @brief Sets the corner rounding radii of the Slider's track background and border.
        void SetCornerRadii(const glm::vec4& radii);

        void EnableSliderButtons(const bool value);

        /// @brief Sets the amount the value changes by each time a slider button is clicked
        template <typename T> void SetButtonIncrement(T value)
        {
            if constexpr (std::is_same_v<T, i8>)
                _buttonIncrement.Int8 = value;
            else if constexpr (std::is_same_v<T, i16>)
                _buttonIncrement.Int16 = value;
            else if constexpr (std::is_same_v<T, i32>)
                _buttonIncrement.Int32 = value;
            else if constexpr (std::is_same_v<T, i64>)
                _buttonIncrement.Int64 = value;
            else if constexpr (std::is_same_v<T, u8>)
                _buttonIncrement.UInt8 = value;
            else if constexpr (std::is_same_v<T, u16>)
                _buttonIncrement.UInt16 = value;
            else if constexpr (std::is_same_v<T, u32>)
                _buttonIncrement.UInt32 = value;
            else if constexpr (std::is_same_v<T, u64>)
                _buttonIncrement.UInt64 = value;
            else if constexpr (std::is_same_v<T, f32>)
                _buttonIncrement.Float32 = value;
            else if constexpr (std::is_same_v<T, f64>)
                _buttonIncrement.Float64 = value;
            else
                _buttonIncrement.Bool = value;
        }

    private:
        void interact();
        bool isInsideIndicator(const glm::vec2& mousePos);
        void setIndicatorSize();
        void setButtonSizes();
        void setButtonPositions();
        void stepValue(const bool increase);
        void setColors(const ColorState state);

        union
        {
            bool Bool;
            i8 Int8;
            i16 Int16;
            i32 Int32;
            i64 Int64;
            u8 UInt8;
            u16 UInt16;
            u32 UInt32;
            u64 UInt64;
            f32 Float32;
            f64 Float64;
        } _buttonIncrement { .UInt64 = 1 };

        // Elements
        std::unique_ptr<Button>
            _buttonLower { nullptr },
            _buttonHigher { nullptr };

        // Settings
        bool _useSliderButtons { false };
        ElementSizing _indicatorSizingMode { ElementSizing::Percent };
        bool _isDraggingIndicator { false };
        glm::i32vec2 _previousIndicatorPosition { glm::i32vec2(0) };
        f32 _indicatorSizingFactor { 25.0f };
    };
}
