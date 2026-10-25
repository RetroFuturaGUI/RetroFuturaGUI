#pragma once
#include "IncludeHelper.hpp"
#include "config.hpp"
#include "IRangedMultiValue.hpp"
#include "Text.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "Polyline.hpp"

namespace RetroFuturaGUI
{
    /// @brief Draws every bound set as a line
    class LineDiagram final : public IRangedMultiValue
    {
    public:
        /// @brief Constructs a LineDiagram widget with non-owning data. Data must be updated externally, and must remain valid for the lifetime of the LineDiagram or be set to nullptr.
        LineDiagram(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        LineDiagram() = delete;
        LineDiagram(const LineDiagram&) = delete;
        LineDiagram(LineDiagram&&) = delete;
        ~LineDiagram() = default;
        auto operator =(const LineDiagram&) = delete;
        auto operator =(LineDiagram&&) = delete;

        void Draw() override;

        /// @brief Sets the label of the line drawn for set index - shown at the start of the line (ShowLineLabels) and next to the pointer while it is hovered (EnableHoverLineLabel).
        void SetLabel(const uSize index, std::string_view labelText);

        /// @brief Sets the color of the line drawn for set lineIndex, and of its label. It may be set before the set is bound.
        void SetLineColor(const uSize lineIndex, const glm::vec4 color);

        /// @brief Sets the width of every line, in pixels. The line under the cursor is drawn _hoverThicknessIncrease wider.
        void SetLineThickness(const f32 thickness);

        /// @brief Sets the font of the line labels and of the text next to the pointer. Text draws nothing until it has a font.
        void SetLabelFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Shows every line's label at the start of the line. Labels of lines that start close together overlap.
        void ShowLineLabels(const bool show);

        /// @brief Shows the hovered line's label next to the pointer.
        void EnableHoverLineLabel(const bool enable);

        /// @brief Shows the value at the pointer next to it - "Label: value" together with EnableHoverLineLabel. Hexadecimal while ShowHexNumbers is on.
        void EnableHoverValue(const bool enable);

        /// @brief Sets the color of the box behind the text next to the pointer. The text has its line's color, so without the box it would vanish
        /// where it crosses its own line. Translucent black by default.
        void SetHoverTextBackgroundColor(const glm::vec4 color);

    private:
        void alignElementsToTrack() override;
        void drawGraph() override;
        void interact();

        /// @brief Creates lines, each with its own points, until there are lineCount of them.
        void growLinesTo(const uSize lineCount);

        /// @brief Places the line over the track, inside its border.
        void alignLine(const uSize lineIndex, const Rectangle& track);

        /// @brief Rebuilds every line's points from its set. The data isn't owned and may change in place, so this runs every frame.
        void updatePoints();

        /// @brief Rebuilds the points of set lineIndex. Every line is spaced by the longest set, so the same index lines up across all of them.
        void updateLinePoints(const uSize lineIndex);

        void drawLine(const uSize lineIndex, const f32 thickness);

        /// @brief Whether point, in window pixels, lies on the line drawn thickness wide - within half of that of its center.
        bool isPointOnLine(const glm::vec2& point, const uSize lineIndex, const f32 thickness) const;

        /// @brief Finds the line under point, in window pixels - the topmost, as later lines are drawn over earlier ones.
        /// @return false when there is none, in which case lineIndex is left as it was.
        bool findHoveredLine(const glm::vec2& point, uSize& lineIndex) const;

        /// @brief The index of the line's data point nearest point, in window pixels. Every line shares the x spacing, so the nearest by x is the one.
        uSize nearestPointIndex(const glm::vec2& point, const uSize lineIndex) const;

        void drawLineLabels();
        void drawLineLabel(const uSize lineIndex, const Rectangle& track);

        /// @brief Draws the pointer on the hovered line's data point nearest the cursor, and the text next to it.
        void drawHoverPointer();
        void drawHoverMarker(const glm::vec2& pointerPosition, const Rectangle& track);
        void drawHoverText(const glm::vec2& pointerPosition, const Rectangle& track);

        /// @brief "Label: value", or whichever of the two is shown.
        std::string composeHoverText() const;
        std::string getLineLabelText(const uSize lineIndex) const;
        glm::vec4 getLineColor(const uSize lineIndex) const;

        /// @brief Gives text the label font and lays its content out again in it. Nothing until SetLabelFontFamily.
        void applyLabelFont(Text* text) const;

        /// @brief Text clips its glyphs to a box around its position - for a label floating over the plot, a box that just holds it.
        static void fitClipBox(Text& text);

        /// @brief Window pixels into the track's own frame, in pixels: its position and screen-plane rotation undone, as isPointInsideRect does.
        static glm::vec2 toTrackSpace(const glm::vec2& point, const Rectangle& track);

        /// @brief A point in the track's frame back into the world, zOffset above the track.
        static glm::vec3 toWorldSpace(const glm::vec2& trackPoint, const Rectangle& track, const f32 zOffset);

        static constexpr const f32
            _hoverThicknessIncrease = 2.0f,
            _labelGap = 4.0f,
            _hoverTextPadding = 3.0f;
        f32
            _lineThickness { 4.0f };
        std::vector<std::unique_ptr<Text>> _labels {};
        std::vector<std::unique_ptr<Polyline>> _lines {};
        std::vector<std::vector<glm::vec2>> _points {};
        glm::vec2 _plotSize { 0.0f };
        uSize
            _hoveredLineIndex { 0 },
            _hoveredPointIndex { 0 };
        bool _lineHovered { false };

    // Labels
        std::unique_ptr<Text> _hoverText { nullptr };
        std::unique_ptr<Rectangle> _hoverTextBackground { nullptr };
        std::vector<glm::vec4> _hoverTextBackgroundColor { glm::vec4(0.0f, 0.0f, 0.0f, 0.75f) };
        std::string _labelFontFamily {};
        f32 _labelFontSize { 16.0f };
        PlatformBridge::Fonts::Slant _labelFontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _labelFontWeight { PlatformBridge::Fonts::Weight::Normal };
        bool
            _showLineLabels { false },
            _showHoverLineLabel { false },
            _showHoverValue { false };
    };
}