#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include "Projection.hpp"
#include <span>
#include <string>
#include <string_view>
#include "Signal.hpp"
#include "IWindowAccessor.hpp"

namespace RetroFuturaGUI
{
    //An interface with properties that all Widgets must implement.
    class IWidget : virtual public IWindowAccessor 
    {
    public:
        IWidget(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        IWidget() = delete;
        virtual ~IWidget() = default;
        virtual void Draw() = 0;
        void Connect_OnEnable(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnDisable(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnEnable(const typename Signal<>::Slot& slot);
        void Disconnect_OnDisable(const typename Signal<>::Slot& slot);
        virtual void SetSize(const glm::vec3& size);
        glm::vec3 GetSize() const;
        virtual void SetPosition(const glm::vec3& position);
        glm::vec3 GetPosition() const;
        virtual void SetRotation(const f32 rotation);
        f32 GetRotation() const;
        std::string_view GetName() const;
        void SetName(std::string_view name);
        WidgetTypeID GetWidgetTypeID() const;
        void SetEnabled(const bool enable, const bool emitSignal = true);
        bool IsEnabled() const;

        template<typename T> T* GetParentWidget() const
        {
            return static_cast<T*>(_parentWidget);
        }

    protected:
    //Identity
        std::string _name;
        IWidget* _parentWidget { nullptr };
        WidgetTypeID 
            _parentWidgetTypeID { WidgetTypeID::None },
            _widgetTypeID { WidgetTypeID::None };

    //Geometry
        Projection& _projection;
        glm::vec3
            _position { 0.0f },
            _size { 0.0f };
        f32 _rotation { 0.0f };

    //Logic
        bool _isEnabledFlag { true };
        ColorState _colorState { ColorState::Enabled };
        Signal<>
            _onEnable,
            _onEnableAsync,
            _onDisable,
            _onDisableAsync;

        bool isPointInside(const glm::vec2& point) const;

    private:

    };
}