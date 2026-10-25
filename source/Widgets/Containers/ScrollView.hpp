#pragma once
#include "ColorPicker.hpp"
#include "ColorPreview.hpp"
#include "Environment.hpp"
#include "Histogram.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "IPointerFilter.hpp"
#include "IWidget.hpp"
#include "LineDiagram.hpp"
#include "MultilineTextBox.hpp"
#include "SeparatorLine.hpp"
#include "Signal.hpp"
#include "Video.hpp"
#include <glm/ext/vector_float3.hpp>
#include <list>
#include <memory>

namespace RetroFuturaGUI
{
    template<typename T>
    concept ScrollViewWidgetTypes =
           std::same_as<T, Label>  || std::same_as<T, Button> || std::same_as<T, Lasagna> || std::same_as<T, TextBox>
        || std::same_as<T, Model> || std::same_as<T, Image> || std::same_as<T, SvgImage> || std::same_as<T, CheckBox> 
        || std::same_as<T, RadioButtonGroup> || std::same_as<T, Slider> || std::same_as<T, ProgressBar> || std::same_as<T, Table>
        || std::same_as<T, Prefab> || std::same_as<T, Environment> || std::same_as<T, ScrollView> || std::same_as<T, Prefab>
        || std::same_as<T, SeparatorLine> || std::same_as<T, ColorPreview> || std::same_as<T, Video> || std::same_as<T, Histogram>
        || std::same_as<T, MultilineTextBox> || std::same_as<T, LineDiagram> || std::same_as<T, ColorPicker>;

    class ScrollView final : public IWidget, public IBackground, public IBorder, public IClickable, public IPointerFilter
    {
    public:
        enum class ScrollEnable : u32
        {
            AllDisabled = 0,
            X = 1,
            Y = 1 << 1,
            Z = 1 << 2
        };

        /// @brief Combines scroll axes, so EnableScrolling(ScrollEnable::X | ScrollEnable::Y) enables both.
        friend constexpr ScrollEnable operator |(const ScrollEnable left, const ScrollEnable right)
        {
            return static_cast<ScrollEnable>(static_cast<u32>(left) | static_cast<u32>(right));
        }

        enum class ScrollAxis : u32
        {
            X,
            Y,
            Z,
            Horizontal = X,
            Vertical = Y,
            Applicatal = Z
        };

        enum class HorizontalSpanAlignment : u32
        { 
            Center, Left, Right 
        };

        enum class VerticalSpanAlignment : u32
        { 
            Center, Top, Bottom 
        };

        enum class ApplicatalSpanAlignment : u32
        { 
            Center, Front, Back
        };

        struct CellSpanAlignment
        {
            HorizontalSpanAlignment _Horizontal { HorizontalSpanAlignment::Center };
            VerticalSpanAlignment _Vertical { VerticalSpanAlignment::Center };
            ApplicatalSpanAlignment _Applicatal { ApplicatalSpanAlignment::Center };
        };

        struct ScrollViewCell
        {
            glm::vec3
                _Position { 0.0f },
                _Size { 0.0f };
            std::unique_ptr<IWidget> _Widget { nullptr };
            bool _PendingRemoval { false };
        };

        ScrollView(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ScrollView() = delete;
        ScrollView(const ScrollView&) = delete;
        ScrollView(ScrollView&&) = delete;
        ~ScrollView() = default;
        auto operator =(const ScrollView&) = delete;
        auto operator =(ScrollView&&) = delete;

        void Draw() override;
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;
        void EnableBackground(const bool enable);
        void EnableBorder(const bool enable);
        void EnableScrolling(const ScrollEnable enableMask);
        f32 GetScrollPosition(const ScrollAxis axis) const;
        void SetScrollPosition(const ScrollAxis axis, const f32 scrollPosition);
        void SetAlignmentDirection(const ScrollAxis direction);

        /// @brief Decides what the widgets inside see of the mouse: A ScrollView may hide a press while it decides between click and scroll, or take the pointer away while scrolling
        void FilterPointer(PointerState& pointer) const override;

        /// @brief Constructs a widget of type T after the last cell and takes ownership of it.
        /// @param args Constructor arguments after the parent window, such as a Lasagna's AxisDefinition or an Image's path.
        /// @return The new widget, or nullptr when the name is already taken. The pointer stays valid until the widget is removed.
        /// @warning Call from the GUI thread only, never from an async slot: a widget creates OpenGL objects, and those must stay on the context's thread.
        template<ScrollViewWidgetTypes T, typename... Args>
        T* AddWidget(std::string_view name, Args&&... args)
        {
            if(findCell(name))
            {
                std::println("ERROR::SCROLLVIEW::WIDGET_NAME_TAKEN::{}::{}", _name, name);
                return nullptr;
            }

            // Most widget constructors take the name as const std::string&, so it is materialised here.
            const std::string widgetName { name };
            std::unique_ptr<T> widget { std::make_unique<T>(widgetName, &_projection, this, _widgetTypeID, _parentWindow, std::forward<Args>(args)...) };
            T* addedWidget { widget.get() };
            adoptWidget(std::move(widget));
            return addedWidget;
        }

        /// @brief Removes the widget with the given name, and the cells after it move up to close the gap.
        ///        The widget is destroyed at the start of the next Draw, so a widget's own slot may remove it.
        /// @return false when no widget has that name.
        /// @warning Call from the GUI thread only, for the same reason as AddWidget.
        bool RemoveWidget(std::string_view name);

        /// @brief Returns the widget with the given name, or nullptr when there is none or it isn't a T.
        template<ScrollViewWidgetTypes T> T* GetWidget(std::string_view name) const
        {
            const ScrollViewCell* cell { findCell(name) };

            if(!cell)
                return nullptr;

            return dynamic_cast<T*>(cell->_Widget.get());
        }


    private:
        /// @brief What a press inside the view has turned out to be so far.
        enum class PressPhase : u32
        {
            None,
            Deciding,   // Held back from the widgets until it moves past the drag threshold or is released
            Scrolling,  // Moved past the threshold along an axis the view can scroll and drags its content and has taken the pointer away from the widgets
            Replaying,  // Released while deciding, so it was a click and the geometrically underlying widgets see the press for this one frame, the release in the next
            HandedOver  // Moved past the threshold along an axis the view can't scroll, or nothing to scroll: the widgets see the real press until it is released
        };

        // Windows starts a drag after 4 px (SM_CXDRAG), so 5 px takes a deliberate move without catching the jitter of a click.
        static constexpr f32 _dragThreshold { 5.0f };

        void interact();

        /// @brief Advances the current press: decides between a scroll and a click, drags the content, and ends the press on release.
        void updatePress(const PointerState& pointer, const bool isMouseInside);

        /// @brief True when the axis is enabled and holds more content than fits, so a drag along it can scroll.
        bool canScrollAlong(const ScrollEnable axis) const;

        /// @brief Appends an already constructed widget. It isn't a template, so a future markup factory that builds widgets by type name can share it.
        void adoptWidget(std::unique_ptr<IWidget> widget);

        /// @brief Looks a cell up by its widget's name. Cells pending removal are skipped, so their names are free again at once.
        ScrollViewCell* findCell(std::string_view name);
        const ScrollViewCell* findCell(std::string_view name) const;
        void layoutCells();
        glm::vec3 getSpanAlignmentFactors() const;
        glm::vec3 contentToWindow(const glm::vec3& point) const;
        glm::vec3 getInnerSize() const;
        void setColors(const ColorState state) override;

        /// @brief Moves the content by as much as the pointer moved since the last frame, on every enabled axis but Z.
        void dragScroll(const glm::vec2& mousePosition);

        /// @brief Emits OnRelease for the view's own press and resets its colors.
        void releasePress(const bool isMouseInside);

        /// @brief Scrolls to the given position, clamped to the content, and emits the signal of every axis that moved.
        void applyScrollPosition(const glm::vec3& scrollPosition);

        bool isScrollEnabled(const ScrollEnable axis) const;

        void performPendingRemoval();

        ScrollEnable _enableMask { ScrollEnable::Y };
        ScrollAxis _alignmentDirection { ScrollAxis::Vertical };
        CellSpanAlignment _cellSpanAlignment {};
        glm::vec3
            _scrollPosition {},
            _contentExtent {};
        glm::vec2
            _previousMousePosition {},
            _pressPosition {};
        PressPhase _pressPhase { PressPhase::None };
        std::list<ScrollViewCell> _items {};
        f32 _cellPadding { 0.0f };
        Signal<>
            _onXScrolled,
            _onXScrolledAsync,
            _onYScrolled,
            _onYScrolledAsync,
            _onZScrolled,
            _onZScrolledAsync;
        bool
            _showBackground { false },
            _showBorder { false },
            _hasPendingRemovals { false }; // RemoveWidget marked a cell, so the next Draw erases it
    };
}