#pragma once
#include "IRectangle.hpp"
#include "DropShadow.hpp"

namespace RetroFuturaGUI
{
    /// @brief The shape a ShadowRectangle casts a shadow of, as the caster draws it now
    struct ShadowCaster
    {
        glm::vec2 _Size { 0.0f };
        glm::vec3 _Position { 0.0f };
        glm::vec4 _CornerRadii { 0.0f };
        CutCorners _CutCorners { CutCorners::None };
        f32 _RingWidth { 0.0f }; //a border's width, casting the shadow of its outline only. 0 casts the whole shape
    };

    /// @brief The drop shadow of a Plane or a Frame: their shape, rounded or cut like it, moved, scaled and blurred as the
    ///        DropShadow says and filled with its colors. Background and Border draw one behind their element
    class ShadowRectangle final : public IRectangle
    {
    public:
        /// @param shadow The settings and colors it draws with, owned by the caster's part
        ShadowRectangle(Projection* projection, DropShadow* shadow);

        /// @brief Draws the shadow of the caster set last, if the DropShadow is shown
        void Draw() override;

        /// @brief Takes the caster's shape as it draws now, so the shadow follows it
        void SetCaster(const ShadowCaster& caster);

    private:
        DropShadow* _shadow { nullptr };
        glm::vec2 _shapeSize { 0.0f };
        f32
            _blur { 0.0f },
            _ringWidth { 0.0f };
        static constexpr f32 _depthOffset { 0.002f }; //behind the caster, which draws next
    };
}