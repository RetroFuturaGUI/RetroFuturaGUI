#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include <glm/gtc/quaternion.hpp>
#include <concepts>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "Frame.hpp"

namespace RetroFuturaGUI
{
    /// @brief How an Environment is scaled into the rectangle (Window/Scene) it is given
    enum class EnvironmentFit : u32
    {
        Contain,    // The whole Environment stays visible. In a rect of a different aspect ratio the remaining areas won't be covered by the Environment
        Cover,      // The box fills the whole rectangle. Overhangs are cut off when clipping is enabled
        KeepHeight, // The box's height fills the rectangle. A wider rectangle shows more on the sides, a narrower one less
        KeepWidth   // The box's width fills the rectangle. A taller rectangle shows more above and below, a shorter one less
    };

    /// @brief Where a widget sits inside an Environment in environment units (-1.0f ... 1.0f)
    struct EnvironmentPlacement
    {
        glm::vec3
            _Position { 0.0f },
            _Size { 1.0f },     // For a Model this is the scale applied to its mesh, so a mesh modelled within -1 ... 1 fills a box of the default extent.
            _Rotation { 0.0f }; // Degrees, applied in the same Z * Y * X order as IWidget::SetRotation, before the Environment's own rotation.
    };

    struct EnvironmentEntry
    {
        std::unique_ptr<IWidget> _Widget { nullptr };
        EnvironmentPlacement _Placement {};
        bool _Show { true };
    };

    /// @brief An empty room in which models and widgets are placed freely in its own environment units (-1.0f ... 1.0f)
    ///        Moving, resizing or rotating the Environment moves, resizes and rotates everything inside it alike.
    class Environment final : public IWidget
    {
    public:
        /// @brief Constructs an empty Environment that covers the whole projection until a parent places it
        Environment(const std::string& name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Environment() = delete;
        Environment(const Environment&) = delete;
        Environment(Environment&&) = delete;
        ~Environment() override = default;
        auto operator =(const Environment&) = delete;
        auto operator =(Environment&&) = delete;

        /// @brief Builds a widget of type T in the Environment and owns it. The Environment is its parent, and passes its
        ///        projection and window on to it. Widgets draw in the order they were attached.
        /// @param args Constructor arguments after the parent window, such as an Image's path.
        /// @return The new widget, or nullptr, building nothing, when the name is taken. The widget lives until it is removed
        ///         or the Environment is destroyed.
        /// @warning Call from the GUI thread only: a widget creates OpenGL objects, and those must stay on the context's thread.
        template<std::derived_from<IWidget> T, typename... Args>
        T* AddWidget(std::string_view name, const EnvironmentPlacement& placement, Args&&... args)
        {
            if(findEntry(name))
            {
                std::println("ERROR::ENVIRONMENT::WIDGET_NAME_TAKEN::{}::{}", _name, name);
                return nullptr;
            }

            //Most widget constructors take the name as const std::string&, so it is materialised here
            const std::string widgetName { name };
            std::unique_ptr<T> widget { std::make_unique<T>(widgetName, &_projection, this, _widgetTypeID, _parentWindow, std::forward<Args>(args)...) };
            T* attachedWidget { widget.get() };
            _entries.push_back(EnvironmentEntry { ._Widget = std::move(widget), ._Placement = placement, ._Show = true });
            placeWidget(_entries.back());
            return attachedWidget;
        }

        /// @brief Removes the widget with the given name. It is destroyed at the start of the next Interact, so its own slot may
        ///        remove it.
        /// @return false when no widget has that name.
        bool RemoveWidget(std::string_view name);

        /// @brief Returns the widget with the given name, or nullptr when there is none or it isn't a T.
        template<std::derived_from<IWidget> T> T* GetWidget(std::string_view name) const
        {
            const EnvironmentEntry* entry { findEntry(name) };

            if(!entry)
                return nullptr;

            return dynamic_cast<T*>(entry->_Widget.get());
        }

        /// @brief Moves the widget with the given name within the Environment.
        /// @return false when no widget has that name.
        bool SetPlacement(std::string_view name, const EnvironmentPlacement& placement);

        /// @brief Returns where the widget with the given name sits, or nullptr when there is none.
        const EnvironmentPlacement* GetPlacement(std::string_view name) const;

        /// @brief Sets whether the widget with the given name is drawn. A hidden widget keeps its placement.
        void ShowWidget(std::string_view name, const bool show);

        /// @brief Returns whether the widget with the given name is drawn. false when there is none.
        bool IsWidgetShown(std::string_view name) const;

        /// @brief Sets half the box's size on each axis, in environment units, and refits everything inside.
        /// @return false when any component is not positive, in which case nothing is changed.
        bool SetExtent(const glm::vec3& extent);

        /// @brief Returns half the box's size on each axis, in environment units.
        const glm::vec3& GetExtent() const;

        /// @brief Sets how the box is scaled into the Environment's rectangle, and refits everything inside.
        void SetFit(const EnvironmentFit fit);

        /// @brief Returns how the box is scaled into the Environment's rectangle.
        EnvironmentFit GetFit() const;

        /// @brief Returns how many window pixels one environment unit currently spans.
        f32 GetScale() const;

        /// @brief Converts a point from environment units to window coordinates, through the Environment's position, rotation and scale.
        glm::vec3 ToWindow(const glm::vec3& point) const;

        /// @brief Sets whether drawing is cut off at the Environment's rectangle. Off by default; Cover, KeepHeight and KeepWidth
        ///        need it to keep overhanging content off neighbouring widgets.
        /// @note  The clip is the unrotated rectangle, because scissoring only cuts along the window's axes. It no longer matches
        ///        once the Environment itself is rotated.
        void EnableClipping(const bool enable);

        /// @brief Sets whether the box's outline is drawn over the contents, which shows where the fit put it.
        void ShowDebugBox(const bool show);

        /// @brief Draws every shown widget in attachment order.
        void Draw() override;

        /// @brief Sets the size of the Environment's rectangle, in window pixels, and refits everything inside.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the window position of the box's center, moving everything inside along.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the box, in degrees, turning everything inside around the box's center.
        void SetRotation(const glm::vec3& rotation) override;

    private:
        /// @brief Sweeps the removed widgets, then passes the frame's input on to the shown ones in attachment order.
        void interact() override;

        EnvironmentEntry* findEntry(std::string_view name);
        const EnvironmentEntry* findEntry(std::string_view name) const;

        /// @brief Recomputes the scale and orientation from the Environment's size, rotation, extent and fit, then places every widget anew.
        void applyTransform();
        void placeWidget(const EnvironmentEntry& entry) const;
        void drawDebugBox();

        /// @brief Splits a rotation into the X, Y and Z angles, in degrees, that every widget composes as Rz * Ry * Rx (see Model::SetRotation).
        static glm::vec3 toEulerDegrees(const glm::quat& rotation);

        std::vector<EnvironmentEntry> _entries {};
        std::vector<std::unique_ptr<IWidget>> _removedWidgets {}; //destroyed at the start of the next Interact
        glm::vec3 _extent { 1.0f };
        EnvironmentFit _fit { EnvironmentFit::Contain };
        f32 _scale { 1.0f };
        glm::quat _orientation { glm::vec3(0.0f) };
        std::unique_ptr<Frame> _debugBox { nullptr };
        std::vector<glm::vec4> _debugBoxColor { glm::vec4(0.35f, 0.9f, 1.0f, 1.0f) };
        bool
            _clippingOn { false },
            _drawDebugBox { false };
    };
}