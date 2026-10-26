#pragma once
#include "IRectangle.hpp"

namespace RetroFuturaGUI
{
    /// @brief The ColorPicker's field of colors: every hue, from white to fully saturated, as a circle or a square. It
    ///        computes its colors, so it takes none, and no features that would change the colors it shows.
    class ColorField final : public IRectangle
    {
    public:
        /// @brief Constructs a ColorField that draws using the given projection. It starts as a circle.
        ColorField(Projection* projection);

        /// @brief Draws the field.
        void Draw() override;

        /// @brief Sets whether the field is a circle or a square.
        void SetShape(const ColorFieldShape shape);

        /// @brief Sets how bright the field is drawn, 0..1.
        void SetBrightness(const f32 brightness);

    private:
        ColorFieldShape _shape { ColorFieldShape::Circle };
        f32 _brightness { 1.0f };
    };
}
