#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include <concepts>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "Frame.hpp"
#include "TrackIndex.hpp"

namespace RetroFuturaGUI
{
    struct LasagnaCell
    {
        glm::vec3
            _PositionPixels { 0.0f },
            _PositionNormalized { 0.0f },
            _SizePixels { 0.0f },
            _SizeNormalized { 0.0f },
            _PaddingPixels { 0.0f },
            _PaddingNormalized { 0.0f };
        u32 
            _RowSpan { 1 },
            _ColSpan { 1 },
            _LayerSpan { 1 };
        std::unique_ptr<IWidget> _Widget { nullptr };
        SizingMode _SizingMode { SizingMode::FILL };
        bool
            _SpanOccupied { false },
            _Show { true };
    };

    /// @brief Where a widget sits in a Lasagna. Aggregate so call-sites can use named initializers (example: AddWidget<Button>("Inc", { ._Row = 0, ._Column = 2 }))
    struct CellPlacement
    {
        u32
            _Row { 0 },
            _Column { 0 },
            _Layer { 0 };
        SizingMode _SizingMode { SizingMode::FILL };
        u32
            _RowSpan { 1 },
            _ColSpan { 1 },
            _LayerSpan { 1 };
    };

    inline constexpr uSize MaxTracksPerAxis { 64 };

    struct AxisDefinition
    {
        std::vector<f32>
            _RowDefinition {},
            _ColumnDefinition {},
            _LayerDefinition {};

        /// @brief True when every axis defines between one and MaxTracksPerAxis tracks, all of positive size
        bool IsValid() const
        {
            const auto axisIsValid = [](const std::vector<f32>& axis)
            {
                if(axis.empty() || axis.size() > MaxTracksPerAxis)
                    return false;

                for(const f32 track : axis)
                    if(0.0f >= track)
                        return false;

                return true;
            };

            return axisIsValid(_RowDefinition) && axisIsValid(_ColumnDefinition) && axisIsValid(_LayerDefinition);
        }
    };

    /// @brief One full-size cell on every axis - what a Prefab starts with when it's given no axis definition.
    inline const AxisDefinition SingleCellAxis { ._RowDefinition = { 1.0f }, ._ColumnDefinition = { 1.0f }, ._LayerDefinition = { 1.0f } };

    class Lasagna final: public IWidget
    {
    public:
        using TrackIndex = RetroFuturaGUI::TrackIndex;
        using AxisIndex = RetroFuturaGUI::AxisIndex;

        /// @brief Constructs a Lasagna layout container with the given row/column/layer definitions.
        Lasagna(const std::string& name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, const AxisDefinition& axisDefinition);
        Lasagna() = delete;
        Lasagna(const Lasagna&) = delete;
        Lasagna(Lasagna&&) = delete;
        ~Lasagna() = default;
        auto operator =(const Lasagna&) = delete;
        auto operator =(Lasagna&&) = delete;

        /// @brief Builds a widget of type T in the given cell, optionally spanning several cells along any axis, and owns it. The Lasagna is its parent, and passes its projection and window on to it.
        /// @param args Constructor arguments after the parent window, such as an Image's path or a Lasagna's AxisDefinition.
        /// @return The new widget, or nullptr, building nothing, when the name is taken or the placement is out of range or
        ///         overlaps a taken cell. The widget lives until it is removed or the Lasagna is destroyed.
        /// @warning Call from the GUI thread only: a widget creates OpenGL objects, and those must stay on the context's thread.
        template<std::derived_from<IWidget> T, typename... Args>
        T* AddWidget(std::string_view name, const CellPlacement& placement, Args&&... args)
        {
            if(findCell(name))
            {
                std::println("ERROR::LASAGNA::WIDGET_NAME_TAKEN::{}::{}", _name, name);
                return nullptr;
            }

            if(!isPlacementFree(placement))
            {
                std::println("ERROR::LASAGNA::CELL_UNAVAILABLE::{}::{}::[{},{},{}]", _name, name, placement._Row, placement._Column, placement._Layer);
                return nullptr;
            }

            //Most widget constructors take the name as const std::string&, so it is materialised here
            const std::string widgetName { name };
            std::unique_ptr<T> widget { std::make_unique<T>(widgetName, &_projection, this, _widgetTypeID, _parentWindow, std::forward<Args>(args)...) };
            T* attachedWidget { widget.get() };
            placeWidget(std::move(widget), placement);
            return attachedWidget;
        }

        /// @brief Removes the widget with the given name and frees its cells at once, so another widget can take them. The widget
        ///        itself is destroyed at the start of the next Draw, so a widget's own slot may remove it.
        /// @return false when no widget has that name.
        bool RemoveWidget(std::string_view name);

        /// @brief Returns the widget with the given name, or nullptr when there is none or it isn't a T.
        template<std::derived_from<IWidget> T> T* GetWidget(std::string_view name) const
        {
            const LasagnaCell* cell { findCell(name) };

            if(!cell)
                return nullptr;

            return dynamic_cast<T*>(cell->_Widget.get());
        }

        /// @brief Shows or hides the widget with the given name. A hidden widget doesn't draw or take input.
        void ShowWidget(std::string_view name, const bool show);

        /// @brief Whether the widget with the given name is shown. false when there is none.
        bool IsWidgetShown(std::string_view name) const;

        /// @brief Returns the cell at the given index, or nullptr when the index is outside the current tracks.
        /// @warning Only valid until the cells are rebuilt
        LasagnaCell* GetCell(const TrackIndex& index);
        const LasagnaCell* GetCell(const TrackIndex& index) const;

        /// @brief The number of tracks currently defined on each axis.
        TrackIndex GetTrackCounts() const;
        /// @brief Lays out and draws all attached widgets
        void Draw() override;

        /// @brief Sets whether the cell debug borders
        void ShowDebugLines(const bool show);

        /// @brief Sets the size of the Lasagna, resizing and repositioning all attached widgets.
        void SetSize(const glm::vec3& size) override;

        /// @brief Replaces the column track widths and relays out the attached widgets.
        void SetColumnDefinition(std::vector<f32> columnDefinition);

        /// @brief Replaces the row track widths and relays out the attached widgets.
        void SetRowDefinition(std::vector<f32> rowDefinition);

        /// @brief Replaces the layer track widths and relays out the attached widgets.
        void SetLayerDefinition(std::vector<f32> layerDefinition);

        /// @brief Sets the world position of the Lasagna, repositioning all attached widgets.
        void SetPosition(const glm::vec3& position) override;

    private:
    // Data
        std::vector<std::vector<std::vector<LasagnaCell>>> _lasagna {};
        static constinit const u32 _maxCountPerAxis { MaxTracksPerAxis };
        AxisDefinition _axisdefinition {};
        std::unique_ptr<Frame> _debugBorder { nullptr };
        std::vector<glm::vec4> _debugBorderColor { glm::vec4(1.0f) };
        bool _drawDebugLines { false };
        std::vector<std::unique_ptr<IWidget>> _removedWidgets {}; //destroyed at the start of the next Draw

        /// @brief The cell holding the widget with the given name, or nullptr.
        LasagnaCell* findCell(std::string_view name);
        const LasagnaCell* findCell(std::string_view name) const;

        /// @brief Where the widget with the given name sits, or nothing.
        std::optional<TrackIndex> findWidgetIndex(std::string_view name) const;

        /// @brief Whether the placement lies within the tracks and covers no taken cell.
        bool isPlacementFree(const CellPlacement& placement) const;

        /// @brief Puts the widget into the placement's cells, which isPlacementFree has checked, and lays it out.
        void placeWidget(std::unique_ptr<IWidget> widget, const CellPlacement& placement);

        /// @brief Returns the Lasagna's top-left corner in world space. Cell offsets count from there, so every placement measures from here rather than from the window.
        glm::vec3 getTopLeftCorner() const;

        void drawDebugLines(const LasagnaCell& cell);
        void updateSpanSize(LasagnaCell& originCell, const uSize row, const uSize column, const uSize layer);
        void resizeCells();
        void resizeWidget(const LasagnaCell& cell);
        void resizeAllWidgets();
        void moveWidgets();
    };
}