#pragma once
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IDesignable.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    class Button : public IWidget, public IClickable, public IDesignable, public ITextProperties
    {
    public:
        Button(const IdentityParams& identity, const GeometryParams3D& geometry, const TextParams& textParams, const float borderWidth = 5.0f);
        void Draw();
        void SetEnabled(const bool enable, const bool emitSignal = true);
        
    //Geometry
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const float rotation) override;
        void SetCornerRadii(const glm::vec4& radii);

    private:
        void interact();
        void setColors(const ColorState state);
    };
}