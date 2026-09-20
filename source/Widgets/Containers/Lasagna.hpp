#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include <string>
#include <vector>
#include "Rectangle.hpp"
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
        IWidget* _Widget = nullptr;
        SizingMode _SizingMode { SizingMode::FILL };
        bool
            _SpanOccupied { false },
            _Show { true };
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
                    if(track <= 0.0f)
                        return false;

                return true;
            };

            return axisIsValid(_RowDefinition) && axisIsValid(_ColumnDefinition) && axisIsValid(_LayerDefinition);
        }
    };

    /// @brief One full-size cell on every axis - what a Prefab starts with when no grid is given.
    inline const AxisDefinition SingleCellAxis { ._RowDefinition = { 1.0f }, ._ColumnDefinition = { 1.0f }, ._LayerDefinition = { 1.0f } };

    class Lasagna final: public IWidget
    {
    public:
        using TrackIndex = RetroFuturaGUI::TrackIndex;
        using AxisIndex = RetroFuturaGUI::AxisIndex;

        /// @brief Constructs a Lasagna grid layout container with the given row/column/layer definitions.
        ///        The definition is copied, so callers need not keep theirs alive; this grid is its owner from here on.
        Lasagna(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, const AxisDefinition& axisDefinition);
        Lasagna() = delete;
        Lasagna(const Lasagna&) = delete;
        Lasagna(Lasagna&&) = delete;
        ~Lasagna() = default;
        auto operator =(const Lasagna&) = delete;
        auto operator =(Lasagna&&) = delete;

        /// @brief Places a widget into the grid at the given row, column and layer, optionally spanning multiple cells along any axis.
        /// @return false when the placement is out of range or overlaps an occupied cell, in which case nothing is changed.
        bool AttachWidget(const u32 row, const u32 col, const u32 layer, IWidget* widget, const SizingMode sizingMode = SizingMode::FILL, const u32 rowSpan = 1, const u32 colSpan = 1, const u32 layerSpan = 1);

        /// @brief Returns the cell at the given index, or nullptr when the index is outside the current grid.
        /// @warning Only valid until the grid is rebuilt
        LasagnaCell* GetCell(const TrackIndex& index);
        const LasagnaCell* GetCell(const TrackIndex& index) const;

        /// @brief The number of tracks currently defined on each axis.
        TrackIndex GetTrackCounts() const;
        /// @brief Lays out and draws all attached widgets
        void Draw() override;

        /// @brief Sets whether the cell debug borders
        void ShowDebugLines(const bool show);

        /// @brief Sets the size of the grid, resizing and repositioning all attached widgets.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the grid, repositioning all attached widgets.
        void SetPosition(const glm::vec3& position) override;

    private:
    // Data
        std::vector<std::vector<std::vector<LasagnaCell>>> _lasagna {};
        static constinit const u32 _maxCountPerAxis { MaxTracksPerAxis };
        AxisDefinition _axisdefinition {};
        std::unique_ptr<Rectangle> _debugBorder { nullptr };
        std::vector<glm::vec4> _debugBorderColor { glm::vec4(1.0f) };
        bool _drawDebugLines { false };

        /// @brief Returns the grid's top-left corner in world space. Cell offsets are grid-local, so every placement measures from here rather than from the window.
        glm::vec3 gridOrigin() const;

        void drawDebugLines(const LasagnaCell& cell);
        void updateSpanSize(LasagnaCell& originCell, const uSize row, const uSize column, const uSize layer);
        void resizeCells();
        void resizeWidget(const LasagnaCell& cell);
        void resizeAllWidgets();
        void moveWidgets();
    };
}