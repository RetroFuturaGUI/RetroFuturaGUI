#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <optional>
#include "Projection.hpp"
#include <span>
#include <string>
#include <string_view>
#include "Signal.hpp"
#include "IWindowAccessor.hpp"
#include "IHierarchyNode.hpp"
#include "IPointerFilter.hpp"
#include "CursorIcon.hpp"
#include "Transition.hpp"
#include <initializer_list>

namespace RetroFuturaGUI
{
    class Tooltip; //not included: its text pulls in windows.h, whose min/max macros would reach every widget
    class ContextMenu;
    class FillStyle;
    template<typename> class WidgetReference;

    //An interface with properties that all Widgets must implement.
    class IWidget : virtual public IWindowAccessor, public IHierarchyNode
    {
        friend class Tooltip;
        friend class ContextMenu;
        friend class ScrollView; //finds the frontmost cell under the pointer with its widgets' own hit tests
        template<typename> friend class WidgetReference;

    public:
        /// @brief Constructs a widget with the given name, under the given parent widget/window.
        IWidget(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        IWidget() = delete;
        virtual ~IWidget();

        /// @brief Reacts to all user input (Mouse, keyboard, gamepad, ...). Must be called before Update(). Inside 
        //         the same frame it draws in, so the pointer meets it where it is drawn!
        void Interact();

        /// @brief Does the per-frame work apart from input and drawing, such as a video advancing. Must be called after
        ///        Interact() and before Draw(). Runs for hidden widgets and windows too.
        void Update();

        /// @brief Draws the widget. Must be called after Update()!
        virtual void Draw() = 0;

        /// @brief Draws the widget at its opacity, which multiplies the opacity of whatever draws it. It also skips it while it is fully transparent. Containers draw their widgets with it. It 
        // draws tilted by its rotation around its center, inside the frame of whatever draws it (Shader::FrameScope)
        void DrawAtOpacity();

        /// @brief Sets how opaque the widget and everything inside it draw, 0 to 1, fading there over the widget's Opacity
        ///        transition (SetTransition). A fully transparent widget still takes the input; hide it to stop that.
        void SetOpacity(const f32 opacity);

        /// @brief Returns how opaque the widget draws now, midway while a fade runs.
        f32 GetOpacity() const;

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

        /// @brief Sets the size of the widget, moving to it over the widget's size transition when one is set (SetTransition).
        ///        A container laying the widget out sets it at once.
        void SetSize(const glm::vec3& size);

        /// @brief Returns the current size of the widget, midway while a transition runs.
        glm::vec3 GetSize() const;

        /// @brief Sets the world position of the widget, moving to it over the widget's position transition when one is set.
        ///        A container laying the widget out sets it at once.
        void SetPosition(const glm::vec3& position);

        /// @brief Returns the current world position of the widget, midway while a transition runs.
        glm::vec3 GetPosition() const;

        /// @brief Sets the rotation of the widget in degrees. Turning to it over the widget's rotation transition when one is set. The 
        //         widget turns around its center, with everything in it. A container turns its widgets along
        void SetRotation(const glm::vec3& rotation);

        /// @brief Returns the current rotation of the widget, midway while a transition runs.
        glm::vec3 GetRotation() const;

        /// @brief Returns the widget's name.
        const std::string& GetName() const final override;

        /// @brief Returns the widget this one is parented to, or nullptr when it hangs directly off a window.
        const IHierarchyNode* GetParent() const final override;

        /// @brief Sets the widget's name.
        void SetName(std::string_view name);

        /// @brief Returns the widget's type ID.
        WidgetTypeID GetWidgetTypeID() const;

        /// @brief Returns the GLFW window that owns this widget.
        GLFWwindow* GetParentWindow() const;

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
        void ShowTooltip(const bool show);

        /// @brief Returns the widget's tooltip for styling (font, colors, corner radii), or nullptr until SetTooltipText created it.
        Tooltip* GetTooltip() const;

        void AddContextMenuItem(std::string_view itemName, std::string_view node, std::string_view imagePath, std::string_view text, std::string_view note);
        void AddContextMenuNode(std::string_view newNodeName, std::string_view node);

        /// @brief Returns this widget's context menu, or nullptr if it has none.
        ContextMenu* GetContextMenu() const;

        /// @brief Sets how this widget's colors or corner radii move to new values, its hover and pressed colors included, in
        ///        place of the global default (Transitions::SetDefault): SetTransition(Colors | CornerRadii, { 200ms, EaseInOut }).
        void SetTransition(const TransitionProperty properties, const Transition& transition);

        /// @brief Has these properties follow the global default again.
        void ResetTransition(const TransitionProperty properties);

        /// @brief Returns the transition one property follows: the widget's own, or else the global default.
        Transition GetTransition(const TransitionProperty property) const;

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
        std::unique_ptr<ContextMenu> _contextMenu;

        bool _enableTooltip { false };

        TransitionSettings _transitionSettings {}; //the elements registered by useTransitions read them, the tooltip and context menu too

        bool isPointInside(const glm::vec2& point) const;

        /// @brief Whether a point lies inside one of the widget's rectangles, such as a slider's indicator. Both are measured as
        ///        if the widget weren't tilted. Pass the point readPointer gives, aas well as the rectangle's own size, position and
        ///        rotation (a vertical slider's indicator is turned by 90 degrees). Never pass the widget's tilt here!
        bool isPointInsideRect(const glm::vec2& point, const glm::vec3& size, const glm::vec3& position, const glm::vec3& rotation) const;

        /// @brief Reads the mouse as this widget should see it. _Position is where the pointer meets the widget's untilted
        ///        layout, so a tilted widget's math stays that of an untilted one. _WindowPosition is the window point. Every
        ///        ancestor that is an IPointerFilter may hold the press back, replay it, or take the pointer away while it scrolls
        PointerState readPointer() const;

        /// @brief Where a point of the window lies in this widget's own untilted layout. Through the frame it last took the
        ///        pointer or drew in the point readPointer gives as _Position
        std::optional<glm::vec2> toOwnLayoutPoint(const glm::vec2& windowPoint) const;

        /// @brief change the cursor icon, such as the hand over a button. Call it every frame the mouse is over a widget to visualize an action can be performed. In a frame where no widget calls it, the cursor is the arrow.
        void requestCursorIcon(const PlatformBridge::CursorIcon icon) const;

        /// @brief Switches the widget to the colors of the given state. Does nothing in a widget without colors per state.
        virtual void setColors([[maybe_unused]] const ColorState state) {}

        /// @brief The widget's own reaction to the frame's input, after its tooltip and context menu. A container calls its widgets' Interact here.
        virtual void interact() {}

        /// @brief The widget's own per-frame work. A container calls its widgets' Update here, hidden ones included.
        virtual void update() {}

        /// @brief Has the given elements follow this widget's transitions. Only for elements drawn once per frame: one drawn
        ///        again with other colors, like a Table's shared cell checkbox, would restart its transition at every draw.
        void useTransitions(std::initializer_list<FillStyle*> elements);

        /// @brief Takes the size: stores it and fits whatever the widget draws to it. so what the widget resizes along with itself follows at once.
        virtual void applySize(const glm::vec3& size);

        /// @brief Takes the position as applySize does the size.
        virtual void applyPosition(const glm::vec3& position);

        /// @brief Takes the rotation on, as applySize does the size. The widget's parts never take it. They are laid out untilted
        ///        and the frame turns them with getTilt()
        virtual void applyRotation(const glm::vec3& rotation);

        /// @brief The matrix that tilts the widget's untilted layout by its rotation around its center. The identity remains unrotated
        glm::mat4 getTilt() const;

        /// @brief Where a point of the window (bottom-up pixels) lies in an untilted layout drawn through the given frame. The
        ///        view through the point meets the layout's plane at the widget's depth
        std::optional<glm::vec2> toLayoutPoint(const glm::vec2& windowPoint, const glm::mat4& frame) const;

    private:
        void updateTooltip();
        void updateContextMenu();

        /// @brief Applies the shown size, position, rotation and opacity while their transitions run, the targets once they arrive.
        void updateGeometry();

        /// @brief Applies the pointer filters from the outermost container down to the closest one. Each container then adjusts
        ///        what the containers above it already let through. An inner ScrollView only ever sees a press its outer one passed on.
        ///        An outer one that scrolls keeps the pointer captured whatever the inner one does. Each learns which widget reads it
        static void filterThroughAncestors(const IHierarchyNode* node, const IHierarchyNode* reader, PointerState& pointer);

        //Freed with the widget, which empties every WidgetReference to it
        std::shared_ptr<const bool> _lifetimeToken { std::make_shared<const bool>(true) };

    //Geometry transitions: _size, _position and _rotation hold what is shown, these where it is going
        TransitionValue<glm::vec3>
            _sizeTransition,
            _positionTransition,
            _rotationTransition;
        bool
            _hasUpdated { false }, //geometry set before the first Update applies at once
            _isResizing { false },
            _isMoving { false },
            _isTurning { false };

        glm::mat4 _frame { 1.0f }; //the frame the widget last took the pointer or drew in (its own tilt included) for readPointer

    //Opacity: _opacity is drawn, _opacityTransition where it is going
        f32 _opacity { 1.0f };
        TransitionValue<f32> _opacityTransition;
        bool _isFading { false };
    };
}