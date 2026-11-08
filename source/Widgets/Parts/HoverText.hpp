#pragma once
#include "IncludeHelper.hpp"
#include "Background.hpp"
#include "WidgetText.hpp"

namespace RetroFuturaGUI
{
    /// @brief The text a LineDiagram shows next to its pointer, on a box that keeps it readable where it crosses its own line. The
    ///        LineDiagram owns it and hands it out through GetHoverText(), so it is styled as lineDiagram.GetHoverText().GetBackground().SetColors(...)
    class HoverText final
    {
    public:
        explicit HoverText(Projection* projection);
        HoverText() = delete;
        HoverText(const HoverText&) = delete;
        HoverText(HoverText&&) = delete;
        ~HoverText() = default;
        auto operator =(const HoverText&) = delete;
        auto operator =(HoverText&&) = delete;

        /// @brief Returns the box behind the text: translucent black, its corners rounded by 3 px by default
        Background& GetBackground();

        /// @brief Returns the box behind the text
        const Background& GetBackground() const;

        /// @brief Returns the text: its colors (see EnableLineColor), fills and effects. Its string is the diagram's, and its font
        ///        the labels' (SetLabelFontFamily)
        WidgetText& GetText();

        /// @brief Returns the text.
        const WidgetText& GetText() const;

        /// @brief Sets whether the text takes the hovered line's color (the default) or shows its own colors
        void EnableLineColor(const bool enable);

        /// @brief Returns whether the text takes the hovered line's color
        bool IsLineColorEnabled() const;

    //Called by the owning widget
        /// @brief Has the colors move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

    private:
        Background _background;
        WidgetText _text;
        bool _isLineColorEnabled { true };
    };
}