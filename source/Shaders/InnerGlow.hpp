#pragma once
#include "IncludeHelper.hpp"
#include "StateFillStyle.hpp"

namespace RetroFuturaGUI
{
    class Text;
    class InnerGlowRectangle;

    /// @brief Light inside a background, a border's outline or a text's glyphs: full at their edges and fading inward over
    ///        its width, drawn over them. Set off by a distance at an angle, the edges it is measured from move with it, so
    ///        it gathers on the edges facing away from that direction: in a dark color it is an inner shadow. Its colors take
    ///        every fill and gradient, per color state like the part it belongs to, and follow that part's state; Enabled is
    ///        white at 60 % opacity by default. Reached by GetInnerGlow(): button.GetBackground().GetInnerGlow().Show(true)
    ///        Hidden until Show(true)
    class InnerGlow final : public StateFillStyle
    {
    public:
        InnerGlow();
        ~InnerGlow() = default;

        /// @brief Shows or hides the inner glow. Hidden by default
        void Show(const bool show);

        /// @brief Returns whether the inner glow is shown
        bool IsShown() const;

        /// @brief Sets how far the inner glow reaches in from the edge, in pixels, fading from full at the edge to nothing
        ///        there. 6 by default. A text renders its inner glow's glyphs once per width and offset, so a text's are best
        ///        not changed every frame
        void SetWidth(const f32 width);
        f32 GetWidth() const;

        /// @brief Sets the direction the edges it is measured from are moved in, in degrees: 0 to the right, counterclockwise
        ///        as the gradients turn. 315 (the default) moves them to the bottom right, so an inner shadow lies along the top
        ///        and left edges, as under light from the top left
        void SetDegree(const f32 degree);
        f32 GetDegree() const;

        /// @brief Sets how far the edges it is measured from are moved, in pixels. 0 (the default) glows evenly from every edge
        void SetDistance(const f32 distance);
        f32 GetDistance() const;

    //Called by the part it belongs to
        /// @brief Returns how far the edges are moved, in pixels with y up, from the degree and the distance
        glm::vec2 GetOffset() const;

    private:
        friend class Text; //draws it.
        friend class InnerGlowRectangle; //draws it.

        f32
            _width { 6.0f },
            _degree { 315.0f },
            _distance { 0.0f };
        bool _isShown { false };
    };
}