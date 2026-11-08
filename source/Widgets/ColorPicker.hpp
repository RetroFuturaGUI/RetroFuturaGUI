#pragma once
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Border.hpp"
#include "ColorPickerIndicator.hpp"
#include "ColorField.hpp"
#include "Frame.hpp"
#include "Plane.hpp"
#include "Texture.hpp"
#include "config.hpp"
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_int2_sized.hpp>
#include <memory>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    // A colorpicker which value can be set by dragging its indicator
    class ColorPicker final : public IWidget
    {
    public:
        enum class ColorPickerType : u32
        {
            Square,
            Circle
        };

        /// @brief Constructs the ColorPicker
        ColorPicker(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~ColorPicker() = default;
        ColorPicker(const ColorPicker&) = delete;
        ColorPicker(ColorPicker&&) = delete;
        auto operator =(const ColorPicker&) = delete;
        auto operator =(ColorPicker&&) = delete;

        /// @brief Draws the ColorPicker
        void Draw() override;
        const glm::vec3 GetColorRGBF() const;
        const glm::vec4 GetColorRGBAF() const;
        u32 GetColorRGB() const;
        u32 GetColorRGBA() const;

        /// @brief Sets how bright the circle is drawn, 0..1. Keeps hue and saturation, so passing through black loses nothing.
        void SetBrightness(const f32 brightness, const bool emitSignal);
        f32 GetBrightness() const;

        /// @brief Sets the picked color's opacity, 0..1. Ignored while alpha is disabled.
        void SetAlpha(const f32 alpha, const bool emitSignal);

        f32 GetAlpha() const;

        /// @brief While disabled, the picked color is fully opaque. The alpha set before is kept for when it's enabled again.
        void EnableAlpha(const bool enable, const bool emitSignal);
        bool IsAlphaEnabled() const;

        /// @brief Sets the picked color from code: moves the indicator and updates brightness and alpha to match.
        void SetColor(const glm::vec4& color, const bool emitSignal);

        /// @brief Sets the picked color from code: moves the indicator and updates brightness and alpha to match.
        void SetColor(const glm::vec3& color, const bool emitSignal);

        /// @brief Returns the field's border. Hover while the pointer is over the field, Clicked while it drags the indicator
        Border& GetBorder();

        /// @brief Returns the field's border
        const Border& GetBorder() const;

        /// @brief Returns the marker on the picked color. Only its border is styled. The dot always shows the picked color
        ColorPickerIndicator& GetIndicator();

        /// @brief Returns the marker on the picked color
        const ColorPickerIndicator& GetIndicator() const;

        /// @brief Connects a slot to be called when the picked color has changed, by dragging or by any setter.
        void Connect_OnColorChanged(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnColorChanged(const typename Signal<>::Slot& slot);

    protected:
        void applySize(const glm::vec3& size) override;

        void applyPosition(const glm::vec3& position) override;

    private:
        void interact() override;
        
        /// @brief Rebuilds _rgbaf from hue, saturation, brightness and alpha, then emits OnColorChanged.
        void updateColor(const bool emitSignal);

        /// @brief Sets the field's and the indicator's border state and applies the colors for them.
        void setBorderStates(const ColorState fieldState, const ColorState indicatorState);

        /// @brief Moves the indicator onto the picked hue and saturation, in front of the field and its border.
        void alignIndicator();

        /// @brief The point relative to the field's center, in pixels, with the widget's rotation undone - the space the field's shader draws in.
        glm::vec2 toFieldSpace(const glm::vec2& point) const;

        /// @brief Whether a point in field space lies on the circle or the square.
        bool isOnField(const glm::vec2& fieldPoint, const glm::vec2& fieldSize) const;

        /// @brief Picks the hue and saturation the field's shader draws at a point in field space. A point past the edge picks the edge, so a drag can leave the field.
        void pickColorAt(const glm::vec2& fieldPoint, const glm::vec2& fieldSize);

    // Color Picker Field
        std::unique_ptr<ColorField> _colorPickerBackground { nullptr };
        Border _border { &_projection };

    // Indicator
        ColorPickerIndicator _indicator { &_projection };
        glm::vec3 _indicatorPosition {};
        static constexpr f32 _indicatorDiameter { 10.0f }; //sizes the dot and its border, and rounds both with half of it

    // Colors
        f32
            _hue { 0.0f },
            _saturation { 0.0f },
            _brightness { 1.0f },
            _alpha { 1.0f };
        glm::vec4 _rgbaf {};
        u32 _rgba { 0x000000FF };

    // Settings
        ColorPickerType _colorPickerType { ColorPickerType::Circle };
        bool
            _isIndicatorDragging { false },
            _wasClicked { false }, //the left button was down last frame, which tells a new press from a held one
            _useAlpha { true };

        Signal<>
            _onColorChanged,
            _onColorChangedAsync;
    };
}