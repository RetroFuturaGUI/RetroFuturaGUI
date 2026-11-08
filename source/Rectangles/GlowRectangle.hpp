#pragma once
#include "IRectangle.hpp"
#include "Glow.hpp"
#include "ShadowRectangle.hpp"

namespace RetroFuturaGUI
{
    /// @brief Which edge of the caster a GlowRectangle glows from
    enum class GlowEdge : u32
    {
        Outer, //outward from the shape's outer edge
        Inner //inward from a ring's inner edge, a border's toward its background
    };

    /// @brief The glow of a Plane or a Frame: light fading away from their edge over the Glow's width, rounded or cut like
    ///        the shape. Background and Border draw it around their element
    class GlowRectangle final : public IRectangle
    {
    public:
        /// @param glow The settings and colors it draws with, owned by the caster's part
        GlowRectangle(Projection* projection, Glow* glow);

        /// @brief Draws the glow of the caster set last, if the Glow is shown
        void Draw() override;

        /// @brief Takes the caster's shape as it draws now and the edge to glow from. The caster's position carries the depth
        ///        the glow is drawn at, which decides what it covers
        void SetCaster(const ShadowCaster& caster, const GlowEdge edge);

    private:
        Glow* _glow { nullptr };
        GlowEdge _edge { GlowEdge::Outer };
        glm::vec2 _shapeSize { 0.0f };
        f32
            _width { 0.0f },
            _ringWidth { 0.0f };
    };
}