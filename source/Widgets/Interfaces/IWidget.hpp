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
        /// @brief Constructs a widget with the given name, under the given parent widget/window.
        IWidget(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        IWidget() = delete;
        virtual ~IWidget() = default;

        /// @brief Draws the widget.
        virtual void Draw() = 0;

        /// @brief Connects a slot to be called when the widget is enabled.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnEnable(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the widget is disabled.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnDisable(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnEnable slot.
        void Disconnect_OnEnable(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnDisable slot.
        void Disconnect_OnDisable(const typename Signal<>::Slot& slot);

        /// @brief Sets the size of the widget.
        virtual void SetSize(const glm::vec3& size);

        /// @brief Returns the current size of the widget.
        glm::vec3 GetSize() const;

        /// @brief Sets the world position of the widget.
        virtual void SetPosition(const glm::vec3& position);

        /// @brief Returns the current world position of the widget.
        glm::vec3 GetPosition() const;

        /// @brief Sets the rotation of the widget.
        virtual void SetRotation(const glm::vec3& rotation);

        /// @brief Returns the current rotation of the widget.
        glm::vec3 GetRotation() const;

        /// @brief Returns the widget's name.
        std::string_view GetName() const;

        /// @brief Sets the widget's name.
        void SetName(std::string_view name);

        /// @brief Returns the widget's type ID.
        WidgetTypeID GetWidgetTypeID() const;

        /// @brief Enables or disables the widget, optionally emitting the associated signal.
        void SetEnabled(const bool enable, const bool emitSignal = true);

        /// @brief Returns whether the widget is currently enabled.
        bool IsEnabled() const;

        /// @brief Returns the parent widget, cast to the given type.
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
        glm::vec3 _rotation { 0.0f };

    //Logic
        bool _isEnabledFlag { true };
        ColorState _colorState { ColorState::Enabled };
        Signal<>
            _onEnable,
            _onEnableAsync,
            _onDisable,
            _onDisableAsync;

        bool isPointInside(const glm::vec2& point) const;
        bool isPointInsideRect(const glm::vec2& point, const glm::vec3& size, const glm::vec3& position, const glm::vec3& rotation) const;

    private:

    };
}