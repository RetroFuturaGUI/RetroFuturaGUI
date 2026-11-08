#pragma once
#include "IncludeHelper.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "Icon.hpp"
#include "ResourceManager.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief One track variant's look: the colors its rows (or columns, in Column orientation) show while enabled. The tracks
    ///        take the variants in turn, so neighbors alternate. Every other state looks the same on every track (see CellStyle).
    ///        Reached through table.GetTrackStyle(n)
    class TrackStyle final
    {
    public:
        /// @brief Sets the cells' background colors: one for a solid fill, the stops for a gradient
        void SetBackgroundColors(std::span<const glm::vec4> colors);

        /// @brief Returns the cells' background colors
        const std::vector<glm::vec4>& GetBackgroundColors() const;

        /// @brief Sets the colors of the border around each cell
        void SetInnerBorderColors(std::span<const glm::vec4> colors);

        /// @brief Returns the colors of the border around each cell
        const std::vector<glm::vec4>& GetInnerBorderColors() const;

        /// @brief Sets the text colors
        void SetTextColors(std::span<const glm::vec4> colors);

        /// @brief Returns the text colors
        const std::vector<glm::vec4>& GetTextColors() const;

        /// @brief Sets the icon colors. Kept for later. No cell draws them yet
        void SetIconColors(std::span<const glm::vec4> colors);

        /// @brief Returns the icon colors
        const std::vector<glm::vec4>& GetIconColors() const;

    private:
        std::vector<glm::vec4>
            _background { ResourceManager::_Eigengrau },
            _innerBorder { glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) },
            _text { glm::vec4(1.0f) },
            _icon { glm::vec4(1.0f) };
    };

    /// @brief The look every track shares in the states besides Enabled: Disabled while the table is, Selected for selected
    ///        tracks, and Hover and Clicked, which no cell draws yet. A state without colors shows the track's own Enabled colors,
    ///        so the alternation stays. Reached through table.GetCellStyle()
    class CellStyle final
    {
    public:
        /// @brief Sets the cells' background colors for the given state. Enabled is each track's own, so it is ignored here
        void SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the cells' background colors for the given state, empty for Enabled
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;

        /// @brief Sets the colors of the border around each cell for the given state. Enabled is ignored
        void SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors of the border around each cell for the given state, empty for Enabled
        const std::vector<glm::vec4>& GetInnerBorderColors(const ColorState state) const;

        /// @brief Sets the text colors for the given state. Enabled is ignored
        void SetTextColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the text colors for the given state, empty for Enabled
        const std::vector<glm::vec4>& GetTextColors(const ColorState state) const;

        /// @brief Sets the icon colors for the given state. Kept for later: no cell draws them yet
        void SetIconColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the icon colors for the given state, empty for Enabled
        const std::vector<glm::vec4>& GetIconColors(const ColorState state) const;

        /// @brief Sets the background colors of the track a drag would drop on, see Table::SetDropTargetRow
        void SetDropTargetColors(std::span<const glm::vec4> colors);

        /// @brief Returns the background colors of the drop target track
        const std::vector<glm::vec4>& GetDropTargetColors() const;

        /// @brief Sets the width of the border around each cell
        void SetInnerBorderWidth(const f32 width);

        /// @brief Returns the width of the border around each cell
        f32 GetInnerBorderWidth() const;

    //Called by the Table
        /// @brief The background colors a cell of the track shows in the state. The shared ones, or the track's own
        const std::vector<glm::vec4>& ShownBackgroundColors(const TrackStyle& track, const ColorState state) const;

        /// @brief The inner border colors a cell of the track shows in the state. The shared ones, or the track's own
        const std::vector<glm::vec4>& ShownInnerBorderColors(const TrackStyle& track, const ColorState state) const;

        /// @brief The text colors a cell of the track shows in the state. The shared ones, or the track's own
        const std::vector<glm::vec4>& ShownTextColors(const TrackStyle& track, const ColorState state) const;

    private:
        /// @brief One element's colors in the shared states
        struct StateColors
        {
            std::vector<glm::vec4>
                _Disabled {},
                _Hover {},
                _Clicked {},
                _Selected {};
        };

        /// @brief The colors kept for the state, or nothing for Enabled
        static std::vector<glm::vec4>* colorsOf(StateColors& colors, const ColorState state);
        static const std::vector<glm::vec4>& colorsOf(const StateColors& colors, const ColorState state);

        /// @brief The shared colors of the state, or the track's own for Enabled or a state without any
        static const std::vector<glm::vec4>& shown(const StateColors& colors, const std::vector<glm::vec4>& own, const ColorState state);

        StateColors
            _background { ._Disabled = {}, ._Hover = {}, ._Clicked = {}, ._Selected = { glm::vec4(0.2f, 0.3f, 0.55f, 1.0f) } },
            _innerBorder { ._Disabled = { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) }, ._Hover = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ._Clicked = { glm::vec4(0.6f, 0.6f, 0.6f, 1.0f) }, ._Selected = {} },
            _text { ._Disabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ._Hover = {}, ._Clicked = {}, ._Selected = { glm::vec4(1.0f) } },
            _icon { ._Disabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ._Hover = {}, ._Clicked = {}, ._Selected = {} };
            //Caret and graph colors per state aren't needed for now; kept here in case they turn out useful later
            //_caret { ._Disabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ._Hover = {}, ._Clicked = {}, ._Selected = {} },
            //_graph { ._Disabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ._Hover = {}, ._Clicked = {}, ._Selected = {} };
        std::vector<glm::vec4> _dropTarget { glm::vec4(0.25f, 0.5f, 0.42f, 1.0f) };
        f32 _innerBorderWidth { 1.0f };
    };

    /// @brief The header cells' look: Enabled, and Disabled while the table is. Reached through table.GetHeaderStyle()
    class HeaderStyle final
    {
    public:
        /// @brief Sets the header cells' background colors for Enabled or Disabled. Any other state sets the Enabled ones
        void SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the header cells' background colors for Enabled or Disabled
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;

        /// @brief Sets the colors of the border around each header cell for Enabled or Disabled
        void SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors of the border around each header cell for Enabled or Disabled
        const std::vector<glm::vec4>& GetInnerBorderColors(const ColorState state) const;

        /// @brief Sets the colors of the border around the header band for Enabled or Disabled. Kept for later: not drawn yet
        void SetOuterBorderColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors of the border around the header band for Enabled or Disabled
        const std::vector<glm::vec4>& GetOuterBorderColors(const ColorState state) const;

        /// @brief Sets the header text colors for Enabled or Disabled. A header can't be selected
        void SetTextColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the header text colors for Enabled or Disabled
        const std::vector<glm::vec4>& GetTextColors(const ColorState state) const;

        /// @brief Sets the width of the border around each header cell
        void SetInnerBorderWidth(const f32 width);

        /// @brief Returns the width of the border around each header cell
        f32 GetInnerBorderWidth() const;

        /// @brief Sets the width of the border around the header band. Kept for later: not drawn yet
        void SetOuterBorderWidth(const f32 width);

        /// @brief Returns the width of the border around the header band
        f32 GetOuterBorderWidth() const;

    private:
        /// @brief One element's colors, Enabled and Disabled
        struct StateColors
        {
            std::vector<glm::vec4>
                _Enabled {},
                _Disabled {};
        };

        static std::vector<glm::vec4>& colorsOf(StateColors& colors, const ColorState state);
        static const std::vector<glm::vec4>& colorsOf(const StateColors& colors, const ColorState state);

        StateColors
            _background { ._Enabled = { ResourceManager::_Eigengrau }, ._Disabled = { ResourceManager::_Eigengrau } },
            _innerBorder { ._Enabled = { glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) }, ._Disabled = { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) } },
            _outerBorder { ._Enabled = { glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) }, ._Disabled = { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) } },
            _text { ._Enabled = { glm::vec4(1.0f) }, ._Disabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) } };
        f32
            _innerBorderWidth { 1.0f },
            _outerBorderWidth { 1.0f };
    };

    /// @brief The checkbox drawn in every checkbox cell: one background, border and checkmark, colored per cell. Enabled, Hover
    ///        and Clicked, and Disabled for a read-only track or a disabled table. Reached through table.GetCellCheckBox()
    class CellCheckBox final
    {
    public:
        explicit CellCheckBox(Projection* projection);
        CellCheckBox() = delete;
        CellCheckBox(const CellCheckBox&) = delete;
        CellCheckBox(CellCheckBox&&) = delete;
        ~CellCheckBox() = default;
        auto operator =(const CellCheckBox&) = delete;
        auto operator =(CellCheckBox&&) = delete;

        /// @brief Returns the box's background
        Background& GetBackground();

        /// @brief Returns the box's background
        const Background& GetBackground() const;

        /// @brief Returns the box's border
        Border& GetBorder();

        /// @brief Returns the box's border
        const Border& GetBorder() const;

        /// @brief Returns the checkmark drawn in checked boxes
        Icon& GetCheckmark();

        /// @brief Returns the checkmark drawn in checked boxes
        const Icon& GetCheckmark() const;

    //Called by the Table
        /// @brief Shows the colors of the given state, for the cell drawn next
        void SetColorState(const ColorState state);

    private:
        Background _background;
        Border _border;
        Icon _checkmark;
    };
}