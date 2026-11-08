#pragma once
#include "IncludeHelper.hpp"
#include "Border.hpp"
#include "Plane.hpp"
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The round marker on a ColorPicker's field: a dot in the picked color with a border around it. The ColorPicker owns
    ///        it and hands it out through GetIndicator(), so it is styled as colorPicker.GetIndicator().GetBorder().SetColors(...).
    ///        The dot always shows the picked color, so only the border is styled
    class ColorPickerIndicator final
    {
    public:
        explicit ColorPickerIndicator(Projection* projection);
        ColorPickerIndicator() = delete;
        ColorPickerIndicator(const ColorPickerIndicator&) = delete;
        ColorPickerIndicator(ColorPickerIndicator&&) = delete;
        ~ColorPickerIndicator() = default;
        auto operator =(const ColorPickerIndicator&) = delete;
        auto operator =(ColorPickerIndicator&&) = delete;

        /// @brief Returns the border around the dot
        Border& GetBorder();

        /// @brief Returns the border around the dot
        const Border& GetBorder() const;

    //Called by the ColorPicker
        /// @brief Shows the picked color in the dot, opaque so the field doesn't show through it
        void SetColor(const glm::vec4& color);

        /// @brief Sizes the dot and its border, fully round
        void SetDiameter(const f32 diameter);

        /// @brief Places the dot, its border just in front of it
        void SetPosition(const glm::vec3& position);

        /// @brief Returns the dot's size
        glm::vec2 GetSize() const;

        /// @brief Returns where the dot is drawn
        glm::vec3 GetPosition() const;

        /// @brief Draws the dot, then its border
        void Draw();

        /// @brief Shows the border's colors of the given state
        void SetColorState(const ColorState state);

        /// @brief Has the colors move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

    private:
        Plane _dot;
        Border _border;
        std::vector<glm::vec4> _dotColors { glm::vec4(1.0f) }; //always one: the dot keeps a span over it
    };
}