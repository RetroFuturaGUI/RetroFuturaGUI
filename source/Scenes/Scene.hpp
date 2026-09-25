#pragma once
#include <memory>
#include <string>
#include "IHierarchyNode.hpp"
#include <string_view>
#include "Lasagna.hpp"

namespace RetroFuturaGUI
{
    class Window;

    /// @brief Identifies the edge of the window's client area a Scene docks to.
    enum class DockEdge : u32
    {
        Top = 0,
        Right = 1,
        Bottom = 2,
        Left = 3
    };

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

        /// @brief Enables or disables drawing
        void SetActive(const bool active);
        bool IsActive() const;
        Lasagna* GetRootLasagna() const;
        const std::string& GetName() const override;
        const IHierarchyNode* GetParent() const override;

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
        std::unique_ptr<Lasagna> _rootLasagna { nullptr };

    //Docking
        DockEdge _reservedEdge { DockEdge::Top };
        f32 _reservedThickness { 0.0f };

    //Logic
        bool
            _hasReservedEdge { false },
            _active { true };

        /// @brief Asks the parent window to recompute its client area, because what this scene
        ///        reserves from it just changed.
        void requestWindowLayout() const;
    };
}
