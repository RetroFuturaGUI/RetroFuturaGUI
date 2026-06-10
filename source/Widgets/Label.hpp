#pragma once
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    class Label : public IWidget, public ITextProperties
    {
    public:
        Label(const IdentityParams& identity, const GeometryParams3D& geometry, const TextParams& textParams);
        ~Label() = default;
        void Draw() override;
        void SetEnabled(const bool enable);
        
    //Geometry
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const float rotation) override;

    private:
        void setColors(const ColorState state);
    };
}