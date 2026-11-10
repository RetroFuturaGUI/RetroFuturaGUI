#pragma once
#include <memory>
#include <string>
#include "IHierarchyNode.hpp"
#include <string_view>
#include "Lasagna.hpp"
#include "Environment.hpp"
#include "ScrollView.hpp"
#include "WindowTab.hpp"
#include "DockEdge.hpp"
#include "IContainer.hpp"
#include "Signal.hpp"

namespace RetroFuturaGUI
{
    class Window;

    class Scene final : public IHierarchyNode
    {
    public:
        Scene(std::string_view name, Window* parentWindow);
        Scene() = delete;
        Scene(const Scene&) = delete;
        Scene(Scene&&) = delete;
        auto operator =(const Scene&) = delete;
        auto operator =(Scene&&) = delete;
        ~Scene() = default;

        /// @brief Emits OnInteract, then has the widgets react to the frame's input. The window calls it once per frame while the
        ///        scene shows, before Update.
        void Interact();

        /// @brief Emits OnUpdate, then has the widgets do their per-frame work. The window calls it once per frame between
        ///        Interact and Draw, also while the scene doesn't show because its window sits in a background tab.
        void Update();

        void Draw();

        /// @brief Connects a slot called once per frame before the scene's widgets take the input, while the scene shows. For
        ///        keys a widget would take too, such as an Enter that must end an edit rather than confirm a dialog.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnInteract(const typename Signal<>::Slot& slot, const bool async);

        void Disconnect_OnInteract(const typename Signal<>::Slot& slot);

        /// @brief Connects a slot called once per frame after the input and before drawing, for every active scene, a window's
        ///        in a background tab too.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnUpdate(const typename Signal<>::Slot& slot, const bool async);

        void Disconnect_OnUpdate(const typename Signal<>::Slot& slot);

        /// @brief Enables or disables drawing. Emits OnActivate or OnDeactivate when it changes
        void SetActive(const bool active);
        bool IsActive() const;

        /// @brief Enables or disables every widget in the scene. A disabled scene still draws, its widgets in their disabled
        ///        colors, and none of them takes input: the scene under a modal overlay, which itself stays click-through.
        ///        Enabling it again gives each widget back the state it had of its own. The widgets take the change at the
        ///        start of the next frame, and each emits OnEnable or OnDisable only if emitSignal is set
        void SetEnabled(const bool enable, const bool emitSignal);

        bool IsEnabled() const;

        /// @brief Whether the widgets emit OnEnable or OnDisable when they take the last SetEnabled
        bool IsEnabledSignalEmitted() const;

        /// @brief Connects a slot called when the scene joins its window (Window::AddScene), ready to show
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnLoad(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnLoad(const typename Signal<>::Slot& slot);

        /// @brief Connects a slot called when the scene leaves its window: closed by SceneLoader::CloseScene once the frame
        ///        is drained, unregistered, or removed by Window::RemoveScene. Its widgets still exist while the slot runs
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnUnload(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnUnload(const typename Signal<>::Slot& slot);

        /// @brief Connects a slot called when SetActive turns the scene on
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnActivate(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnActivate(const typename Signal<>::Slot& slot);

        /// @brief Connects a slot called when SetActive turns the scene off
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnDeactivate(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnDeactivate(const typename Signal<>::Slot& slot);

        /// @brief Sets whether the scene shows while its window is docked into a WindowTab, which it does by default. A window's own bar turns it off: the tab's field shows the window's title and close button instead, and the strip the bar reserves goes back to the content.
        void ShowInTab(const bool show);

        /// @brief Whether the scene shows while its window is docked into a WindowTab.
        bool IsShownInTab() const;
        IWidget* GetRootContainer() const;
        const std::string& GetName() const override;
        const IHierarchyNode* GetParent() const override;

        /// @brief Builds the scene's root container
        template<RootContainer T, typename... Args>
        T* SetRootContainer(Projection* projection, Args&&... args)
        {
            if(!projection)
                return nullptr;

            GLFWwindow* glfwWindow { getParentGlfwWindow() };

            if(!glfwWindow)
                return nullptr;

            std::unique_ptr<T> container { std::make_unique<T>("RootContainer", projection, this, WidgetTypeID::None, glfwWindow, std::forward<Args>(args)...) };
            T* root { container.get() };
            adoptRootContainer(std::move(container));
            return root;
        }

        /// @brief Returns the root container as T, or nullptr when there is none or it is of another kind.
        template<RootContainer T>
        T* GetRootContainer() const
        {
            return dynamic_cast<T*>(_rootContainer.get());
        }

        /// @brief Sets the rows, columns and layers of the scene's root Lasagna. A root Lasagna already there keeps its widgets
        ///        in their cells, as by Lasagna::SetAxisDefinition. One reaching past the new last track is removed. Any other root
        ///        container, or none, is replaced by a new Lasagna
        /// @return The names of the widgets removed
        std::vector<std::string> SetLasagnaAxis(const AxisDefinition& axisDefinition, Projection* projection);

    //Geometry
        void SetSize(const glm::vec3& size);
        const glm::vec3& GetSize() const;
        void SetPosition(const glm::vec3& position);
        const glm::vec3& GetPosition() const;
        void SetRotation(const glm::vec3& rotation);
        const glm::vec3& GetRotation() const;

    //Docking
        /// @brief Docks the scene to an edge of the window's client area, reserving `thickness`
        ///        pixels across that edge. Scenes laid out after this one, and the window's root
        ///        Lasagna, are fitted into what is left - so nothing lands underneath it, and
        ///        because widgets hit-test inside their own Draw, nothing underneath it can be
        ///        clicked either.
        void SetReservedEdge(const DockEdge edge, const f32 thickness);

        /// @brief Turns the scene back into an overlay: it covers the whole client area, draws over
        ///        whatever was laid out before it, and reserves nothing. This is the default.
        void ClearReservedEdge();

        /// @brief Whether the scene reserves space. False means it overlays the client area.
        bool HasReservedEdge() const;

        /// @brief The edge the scene is docked to. Only meaningful while HasReservedEdge() is true.
        DockEdge GetReservedEdge() const;

        /// @brief The thickness the scene reserves. Only meaningful while HasReservedEdge() is true.
        f32 GetReservedThickness() const;

    private:

    //Identity
        std::string _name {};
        Window* _parentWindow { nullptr };

    //Geometry
        glm::vec3
            _size { 0.0f },
            _position { 0.0f },
            _rotation { 0.0f };

    //Elements
        std::unique_ptr<IWidget> _rootContainer { nullptr };

    //Docking
        DockEdge _reservedEdge { DockEdge::Top };
        f32 _reservedThickness { 0.0f };

    //Signals
        Signal<>
            _onInteract,
            _onInteractAsync,
            _onUpdate,
            _onUpdateAsync,
            _onLoad,
            _onLoadAsync,
            _onUnload,
            _onUnloadAsync,
            _onActivate,
            _onActivateAsync,
            _onDeactivate,
            _onDeactivateAsync;

    //Logic
        bool
            _hasReservedEdge { false },
            _active { true },
            _shownInTab { true },
            _isEnabled { true },
            _isEnabledSignalEmitted { false };

        friend class Window; //tells the scene when it joins and leaves

        /// @brief Emits OnLoad. Window::AddScene calls it.
        void emitLoad();

        /// @brief Emits OnUnload. Window::RemoveScene calls it.
        void emitUnload();

        /// @brief Asks the parent window to recompute its client area, because what this scene
        ///        reserves from it just changed.
        void requestWindowLayout() const;

        /// @brief The parent window's GLFW window, or nullptr without a parent window. Defined out of
        ///        line, because Window is only forward-declared here and SetRootContainer cannot reach into it.
        GLFWwindow* getParentGlfwWindow() const;

        /// @brief Takes ownership of a new root container and hands it the scene's current geometry.
        void adoptRootContainer(std::unique_ptr<IWidget> container);
    };
}