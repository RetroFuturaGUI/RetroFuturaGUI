#pragma once
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include "Text.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include "IDesignable.hpp"
#include "ITextProperties.hpp"

namespace RetroFuturaGUI
{
    class TextBox : public IWidget, public IClickable, public IDesignable, public ITextProperties 
    {
    public:
        TextBox(const IdentityParams& identity, const GeometryParams3D& geometry, const TextParams& textParams, const float borderWidth = 5.0f);
        void Draw();
        void Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnTextChange(const typename Signal<>::Slot& slot);
        void SetEnabled(const bool enable, const bool emitSignal = true);

    //Geometry
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const float rotation) override;

    private:
        void interact();
        void setColors(const ColorState state);
    };
}