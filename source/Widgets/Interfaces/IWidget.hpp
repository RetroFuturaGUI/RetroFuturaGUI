#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include "Projection.hpp"
#include <span>
#include <string>
#include <string_view>
#include "Signal.hpp"
#include "IWindowAccessor.hpp"
#include "IHierarchyNode.hpp"
#include "IPointerFilter.hpp"
#include "CursorIcon.hpp"

namespace RetroFuturaGUI
{
    class Tooltip; //not included: its text pulls in windows.h, whose min/max macros would reach every widget

    //An interface with properties that all Widgets must implement.
    class IWidget : virtual public IWindowAccessor, public IHierarchyNode
    {
        friend class Tooltip;

    public:
        /// @brief Constructs a widget with the given name, under the given parent widget/window.
        IWidget(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        IWidget() = delete;
        virtual ~IWidget();

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
        const std::string& GetName() const override;

        /// @brief Returns the widget this one is parented to, or nullptr when it hangs directly off a window.
        const IHierarchyNode* GetParent() const override;

        /// @brief Sets the widget's name.
        void SetName(std::string_view name);

        /// @brief Returns the widget's type ID.
        WidgetTypeID GetWidgetTypeID() const;

        /// @brief Enables or disables the widget, switching it to that state's colors, and emits OnEnable or OnDisable if emitSignal is set.
        void SetEnabled(const bool enable, const bool emitSignal);

        /// @brief Returns whether the widget is currently enabled.
        bool IsEnabled() const;

        /// @brief Returns the parent widget, cast to the given type.
        template<typename T> T* GetParentWidget() const
        {
            return static_cast<T*>(_parentWidget);
        }

        void SetTooltipText(std::string_view text);
        void EnableTooltip(const bool enable);

        /// @brief Returns the widget's tooltip for styling (font, colors, corner radii), or nullptr until SetTooltipText created it.
        Tooltip* GetTooltip() const;

        /// @brief Tracks the mouse for the widget's tooltip. Containers call it every frame, right before the widget's Draw,
        ///        so the tooltips of widgets inside this one are updated after it and win.
        void UpdateTooltip();

    protected:
    //Identity
        std::string _name;
        IHierarchyNode* _parentWidget { nullptr };
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
        ColorState _colorState { ColorState::Enabled };
        Signal<>
            _onEnable,
            _onEnableAsync,
            _onDisable,
            _onDisableAsync;
        bool _isEnabledFlag { true };

    // stuff
        std::unique_ptr<Tooltip> _tooltip; //no { nullptr }: with Tooltip only declared here, MSVC can't compile the initializer's cleanup
        bool _enableTooltip { false };

        bool isPointInside(const glm::vec2& point) const;
        bool isPointInsideRect(const glm::vec2& point, const glm::vec3& size, const glm::vec3& position, const glm::vec3& rotation) const;

        /// @brief Reads the mouse in window coordinates, bottom-up, as this widget should see it: every ancestor that is an
        ///        IPointerFilter may hold the press back, replay it, or take the pointer away entirely while it scrolls.
        PointerState readPointer() const;

        /// @brief change the cursor icon, such as the hand over a button. Call it every frame the mouse is over a widget to visualize an action can be performed. In a frame where no widget calls it, the cursor is the arrow.
        void requestCursorIcon(const PlatformBridge::CursorIcon icon) const;

        /// @brief Switches the widget to the colors of the given state. Does nothing in a widget without colors per state.
        virtual void setColors([[maybe_unused]] const ColorState state) {}

    private:
        /// @brief Applies the pointer filters from the outermost container down to the closest one. Each container then adjusts
        ///        what the containers above it already let through: an inner ScrollView only ever sees a press its outer one passed on,
        ///        and an outer one that scrolls keeps the pointer captured whatever the inner one does.
        static void filterThroughAncestors(const IHierarchyNode* node, PointerState& pointer);
    };
}