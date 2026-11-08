#pragma once
#include "IncludeHelper.hpp"
#include "Background.hpp"
#include "Border.hpp"

namespace RetroFuturaGUI
{
    /// @brief A background with a border, the look of something a widget draws once per row, item or field, such as a MenuBar's
    ///        items. The widget owns it and hands it out through a getter, so it is styled as menuBar.GetItemStyle().GetBorder().SetWidth(...)
    class Box final
    {
    public:
        explicit Box(Projection* projection);
        Box() = delete;
        Box(const Box&) = delete;
        Box(Box&&) = delete;
        ~Box() = default;
        auto operator =(const Box&) = delete;
        auto operator =(Box&&) = delete;

        /// @brief Returns the box's background
        Background& GetBackground();

        /// @brief Returns the box's background
        const Background& GetBackground() const;

        /// @brief Returns the box's border
        Border& GetBorder();

        /// @brief Returns the box's border
        const Border& GetBorder() const;

    //Called by the owning widget
        /// @brief Draws the background, then the border over it
        void Draw();

        /// @brief Sizes and places the box, its border just in front of its background
        void Place(const glm::vec2& size, const glm::vec3& position);

        /// @brief Shows the colors of the given state
        void SetColorState(const ColorState state);

    private:
        Background _background;
        Border _border;
    };
}