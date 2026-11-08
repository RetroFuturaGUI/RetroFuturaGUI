#pragma once
#include "IRectangle.hpp"
#include "InnerGlow.hpp"
#include "ShadowRectangle.hpp"

namespace RetroFuturaGUI
{
    /// @brief The inner glow of a Plane or a Frame: light inside their shape, or inside a frame's outline, fading in from the
    ///        edges over the InnerGlow's width, measured from edges moved by its offset. Background and Border draw it over
    ///        their element
    class InnerGlowRectangle final : public IRectangle
    {
    public:
        /// @param innerGlow The settings and colors it draws with, owned by the caster's part
        InnerGlowRectangle(Projection* projection, InnerGlow* innerGlow);

        /// @brief Draws the inner glow of the caster set last, if the InnerGlow is shown
        void Draw() override;

        /// @brief Takes the caster's shape as it draws now. The caster's position carries the depth it is drawn at, just in
        ///        front of the caster
        void SetCaster(const ShadowCaster& caster);

    private:
        InnerGlow* _innerGlow { nullptr };
        glm::vec2
            _shapeSize { 0.0f },
            _offset { 0.0f };
        f32
            _width { 0.0f },
            _ringWidth { 0.0f };
    };
}
