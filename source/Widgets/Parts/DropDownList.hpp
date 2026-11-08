#pragma once
#include "IncludeHelper.hpp"
#include "Background.hpp"
#include "Border.hpp"

namespace RetroFuturaGUI
{
    /// @brief The list a drop-down opens: its background, its border and the highlight behind the hovered row. The drop-down owns
    ///        it and hands it out through GetList(), so it is styled as comboBox.GetList().GetBackground().SetColors(...)
    class DropDownList final
    {
    public:
        explicit DropDownList(Projection* projection);
        DropDownList() = delete;
        DropDownList(const DropDownList&) = delete;
        DropDownList(DropDownList&&) = delete;
        ~DropDownList() = default;
        auto operator =(const DropDownList&) = delete;
        auto operator =(DropDownList&&) = delete;

        /// @brief Returns the list's background
        Background& GetBackground();

        /// @brief Returns the list's background
        const Background& GetBackground() const;

        /// @brief Returns the list's border
        Border& GetBorder();

        /// @brief Returns the list's border
        const Border& GetBorder() const;

        /// @brief Returns the highlight drawn behind the hovered row. It shows its Enabled colors
        Background& GetHighlight();

        /// @brief Returns the highlight drawn behind the hovered row. It shows its Enabled colors
        const Background& GetHighlight() const;

    //Called by the owning widget
        /// @brief Shows the background's and the border's colors of the given state
        void SetColorState(const ColorState state);

        /// @brief Has the colors and corner radii move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

    private:
        Background
            _background,
            _highlight;
        Border _border;
    };
}