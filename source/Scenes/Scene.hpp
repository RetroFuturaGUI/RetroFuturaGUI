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
#include "Signal.hpp"

namespace RetroFuturaGUI
{
    class Window;

    template<typename T>
    concept RootContainer =
        std::same_as<T, Lasagna> || std::same_as<T, Environment> || std::same_as<T, ScrollView> || std::same_as<T, WindowTab>;

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

        void Draw();

        void Connect_OnUpdate(const typename Signal<>::Slot& slot, const bool async);

        void Disconnect_OnUpdate(const typename Signal<>::Slot& slot);

        /// @brief Enables or disables drawing
        void SetActive(const bool active);
        bool IsActive() const;

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

            GLFWwindow* glfwWindow { parentGlfwWindow() };

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

        /// @brief Builds the scene's root Lasagna. Replaces any previous one, discarding its cells.
        void SetLasagnaAxis(const AxisDefinition axisdefinition, Projection* projection);

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
            _onUpdate,
            _onUpdateAsync;

    //Logic
        bool
            _hasReservedEdge { false },
            _active { true },
            _shownInTab { true };

        /// @brief Asks the parent window to recompute its client area, because what this scene
        ///        reserves from it just changed.
        void requestWindowLayout() const;

        /// @brief The parent window's GLFW window, or nullptr without a parent window. Defined out of
        ///        line, because Window is only forward-declared here and SetRootContainer cannot reach into it.
        GLFWwindow* parentGlfwWindow() const;

        /// @brief Takes ownership of a new root container and hands it the scene's current geometry.
        void adoptRootContainer(std::unique_ptr<IWidget> container);
    };
}