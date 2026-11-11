#pragma once
#include "IncludeHelper.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "Icon.hpp"
#include "StyledPart.hpp"
#include <optional>
#include "FontManager.hpp"
#include "Text.hpp"
#include <functional>
#include <string>
#include <string_view>
#include "ResourceManager.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The font, alignment and padding a group of the Table's texts share: its cells' or its headers'.
    struct TableTextSettings
    {
        bool _HasFont { false }; //without one, the texts keep Text's own default font
        std::string _FontFamily {};
        f32 _FontSize { FontManager::ToPixels(16.0f, FontSizeUnit::Points) };
        PlatformBridge::Fonts::Slant _Slant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _Weight { PlatformBridge::Fonts::Weight::Normal };
        TextAlignment _Alignment { TextAlignment::Left };
        f32 _Padding { 0.0f };

        bool operator==(const TableTextSettings&) const = default;
    };

    /// @brief A group of the Table's texts' font, alignment and padding through the style layers. Each set on the style itself,
    ///        else by the first of the table's flavors (then .default) that sets it, else the default
    class TableTextLayers final
    {
    public:
        /// @brief Which of a flavor's text settings the group takes, nullptr when the flavor sets none for it
        using FlavorPick = const TableTextLayer* (*)(const Flavor& flavor);

        explicit TableTextLayers(FlavorPick pick);

        /// @brief Returns the settings set on the style itself, to set them
        TableTextLayer& GetOwn();

        /// @brief Returns the settings shown.
        const TableTextSettings& GetShown() const;

        /// @brief Takes the settings through the layers; true when what shows changed
        bool Resolve(std::span<const u32> flavors, const bool isStyled);

    private:
        /// @brief One setting: the own one, else the first flavor's that sets it
        template<typename T> std::optional<T> find(std::optional<T> TableTextLayer::* setting, std::span<const u32> flavors, const bool isStyled) const
        {
            if(_own.*setting)
                return _own.*setting;

            if(!isStyled)
                return std::nullopt;

            return StylingManager::FindFirst<std::optional<T>>(flavors, [this, setting](const Flavor& flavor) -> std::optional<T>
            {
                const TableTextLayer* layer { _pick(flavor) };

                if(!layer)
                    return std::nullopt;

                return layer->*setting;
            });
        }

        FlavorPick _pick;
        TableTextLayer _own {};
        TableTextSettings _shown {};
    };

    /// @brief One track variant's look: the colors its rows (or columns, in Column orientation) show while enabled. The tracks
    ///        take the variants in turn, so neighbors alternate. Every other state looks the same on every track (see CellStyle).
    ///        A color set here wins over the table's flavors' (Flavor::GetTrackStyle(n) for the nth variant), which win over the
    ///        built-in ones. Reached through table.GetTrackStyle(n)
    class TrackStyle final
    {
    public:
        /// @brief Sets the cells' background colors: one for a solid fill, the stops for a gradient. None leave them to the flavors
        void SetBackgroundColors(std::span<const glm::vec4> colors);

        /// @brief Returns the cells' background colors set here, empty when unset
        const std::vector<glm::vec4>& GetBackgroundColors() const;

        /// @brief Sets the colors of the border around each cell
        void SetInnerBorderColors(std::span<const glm::vec4> colors);

        /// @brief Returns the colors of the border around each cell set here, empty when unset
        const std::vector<glm::vec4>& GetInnerBorderColors() const;

        /// @brief Sets the text colors
        void SetTextColors(std::span<const glm::vec4> colors);

        /// @brief Returns the text colors set here, empty when unset
        const std::vector<glm::vec4>& GetTextColors() const;

        /// @brief Sets the icon colors. The checkmark of the track's checkbox cells
        void SetIconColors(std::span<const glm::vec4> colors);

        /// @brief Returns the icon colors set here, empty when unset
        const std::vector<glm::vec4>& GetIconColors() const;

    //Called by the Table
        /// @brief The colors shown: set here, else the flavors' for this variant, else the built-in ones
        const std::vector<glm::vec4>& ShownBackgroundColors() const;
        const std::vector<glm::vec4>& ShownInnerBorderColors() const;
        const std::vector<glm::vec4>& ShownTextColors() const;
        const std::vector<glm::vec4>& ShownIconColors() const;

    //Called by the owner of the table, which alone holds a StyleKey
        /// @brief Sets the built-in background colors. The look everything else overrides
        void SetBuiltInBackgroundColors(const StyleKey& key, std::span<const glm::vec4> colors);

        /// @brief Sets the built-in colors of the border around each cell
        void SetBuiltInInnerBorderColors(const StyleKey& key, std::span<const glm::vec4> colors);

    private:
        friend class Table; //hands it its flavors and its variant

        /// @brief Has the style take unset colors from the table's flavors, as their variant index
        void setStyleSource(const std::vector<u32>* flavors, const uSize index);

        /// @brief One element's shown colors: own, else the first flavor's for the variant, else built-in.
        const std::vector<glm::vec4>& shown(const std::vector<glm::vec4>& own, const std::vector<glm::vec4>& builtIn, const std::vector<glm::vec4>& (FlavorTrackStyle::*get)() const) const;

        std::vector<glm::vec4>
            _background {}, //set here.
            _innerBorder {},
            _text {},
            _icon {},
            _builtInBackground { ResourceManager::_Eigengrau },
            _builtInInnerBorder { glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) },
            _builtInText { glm::vec4(1.0f) },
            _builtInIcon { glm::vec4(1.0f) };
        const std::vector<u32>* _flavors { nullptr }; //the table's flavors, kept by the table
        uSize _index { 0 }; //which variant it is
    };

    /// @brief The look every track shares in the states besides Enabled: Disabled while the table is, Selected for selected
    ///        tracks, Hover for the track under the pointer and Clicked while it is pressed. A state without colors shows the track's own Enabled colors,
    ///        so the alternation stays. What is set here wins over the table's flavors' (Flavor::GetCellStyle()), which win over
    ///        the built-in look. Reached through table.GetCellStyle()
    class CellStyle final : public IStyledPart
    {
    public:
        /// @param onTextChange What the Table does when the font, alignment or padding change.
        explicit CellStyle(std::function<void()> onTextChange = {});

        /// @brief Sets the cells' font, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Returns the cells' font size in the given unit.
        f32 GetFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets the horizontal alignment of the cells' text.
        void SetTextAlignment(const TextAlignment alignment);

        /// @brief Returns the horizontal alignment of the cells' text.
        TextAlignment GetTextAlignment() const;

        /// @brief Sets the padding around the cells' text.
        void SetTextPadding(const f32 padding);

        /// @brief Returns the padding around the cells' text.
        f32 GetTextPadding() const;

        /// @brief Sets the cells' background colors for the given state. Enabled is each track's own, so it is ignored here
        void SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the cells' background colors set here for the given state, empty when unset
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;

        /// @brief Sets the colors of the border around each cell for the given state. Enabled is ignored
        void SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors of the border around each cell set here for the given state, empty when unset
        const std::vector<glm::vec4>& GetInnerBorderColors(const ColorState state) const;

        /// @brief Sets the text colors for the given state. Enabled is ignored
        void SetTextColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the text colors set here for the given state, empty when unset
        const std::vector<glm::vec4>& GetTextColors(const ColorState state) const;

        /// @brief Sets the icon colors (the checkbox cells' checkmark) for the given state
        void SetIconColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the icon colors set here for the given state, empty when unset
        const std::vector<glm::vec4>& GetIconColors(const ColorState state) const;

        /// @brief Sets the background colors of the track a drag would drop on, see Table::SetDropTargetRow
        void SetDropTargetColors(std::span<const glm::vec4> colors);

        /// @brief Returns the background colors of the drop target track set here, empty when unset
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

        /// @brief The icon colors (the checkmark) a cell of the track shows in the state. The shared ones, or the track's own
        const std::vector<glm::vec4>& ShownIconColors(const TrackStyle& track, const ColorState state) const;

        /// @brief The background colors of the drop target track
        const std::vector<glm::vec4>& ShownDropTargetColors() const;

        /// @brief The font, alignment and padding, for the texts the Table builds.
        const TableTextSettings& GetTextSettings() const;

    //Called by the owner of the table, which alone holds a StyleKey
        /// @brief Sets the built-in width of the border around each cell. The look everything else overrides
        void SetBuiltInInnerBorderWidth(const StyleKey& key, const f32 width);

    private:
        void restyle() override;

        /// @brief Takes the text settings through the layers and tells the Table when what shows changed
        void resolveText();

        /// @brief Tells the Table the texts' font, alignment or padding changed, so it styles its texts again.
        void notifyTextChange();

        /// @brief One element's colors in the state: the shared ones through the layers, or the track's own for Enabled or a state without any
        const std::vector<glm::vec4>& shown(const LayeredColors& colors, const StylePart part, const std::vector<glm::vec4>& own, const ColorState state) const;

        LayeredColors
            _background {}, //Enabled is each track's own, so only the shared states are kept
            _innerBorder {},
            _text {},
            _icon {},
            _dropTarget {}; //its Enabled colors
            //Caret and graph colors per state aren't needed for now; kept here in case they turn out useful later
            //_caret { ._Disabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ._Hover = {}, ._Clicked = {}, ._Selected = {} },
            //_graph { ._Disabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ._Hover = {}, ._Clicked = {}, ._Selected = {} };
        std::optional<f32> _innerBorderWidth {}; //set here
        f32 _builtInInnerBorderWidth { 1.0f };
        TableTextLayers _textSettings;
        std::function<void()> _onTextChange {};
    };

    /// @brief The header cells' look: Enabled, and Disabled while the table is. What is set here wins over the table's
    ///        flavors' (Flavor::GetHeaderStyle()), which win over the built-in look. Reached through table.GetHeaderStyle()
    class HeaderStyle final : public IStyledPart
    {
    public:
        /// @param onTextChange What the Table does when the font, alignment or padding change.
        explicit HeaderStyle(std::function<void()> onTextChange = {});

        /// @brief Sets the headers' font, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Returns the headers' font size in the given unit.
        f32 GetFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets the horizontal alignment of the headers' text.
        void SetTextAlignment(const TextAlignment alignment);

        /// @brief Returns the horizontal alignment of the headers' text.
        TextAlignment GetTextAlignment() const;

        /// @brief Sets the padding around the headers' text.
        void SetTextPadding(const f32 padding);

        /// @brief Returns the padding around the headers' text.
        f32 GetTextPadding() const;

        /// @brief Sets the header cells' background colors for Enabled or Disabled. Any other state sets the Enabled ones
        void SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the header cells' background colors set here for Enabled or Disabled, empty when unset
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;

        /// @brief Sets the colors of the border around each header cell for Enabled or Disabled
        void SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors of the border around each header cell set here for Enabled or Disabled, empty when unset
        const std::vector<glm::vec4>& GetInnerBorderColors(const ColorState state) const;

        /// @brief Sets the colors of the border around the header band for Enabled or Disabled
        void SetOuterBorderColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors of the border around the header band set here for Enabled or Disabled, empty when unset
        const std::vector<glm::vec4>& GetOuterBorderColors(const ColorState state) const;

        /// @brief Sets the header text colors for Enabled or Disabled. A header can't be selected
        void SetTextColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the header text colors set here for Enabled or Disabled, empty when unset
        const std::vector<glm::vec4>& GetTextColors(const ColorState state) const;

        /// @brief Sets the width of the border around each header cell
        void SetInnerBorderWidth(const f32 width);

        /// @brief Returns the width of the border around each header cell
        f32 GetInnerBorderWidth() const;

        /// @brief Sets the width of the border around the header band
        void SetOuterBorderWidth(const f32 width);

        /// @brief Returns the width of the border around the header band
        f32 GetOuterBorderWidth() const;

    //Called by the Table
        /// @brief The colors shown for Enabled or Disabled (any other state shows Enabled): set here, else the flavors', else built-in
        const std::vector<glm::vec4>& ShownBackgroundColors(const ColorState state) const;
        const std::vector<glm::vec4>& ShownInnerBorderColors(const ColorState state) const;
        const std::vector<glm::vec4>& ShownOuterBorderColors(const ColorState state) const;
        const std::vector<glm::vec4>& ShownTextColors(const ColorState state) const;

        /// @brief The font, alignment and padding, for the texts the Table builds.
        const TableTextSettings& GetTextSettings() const;

    //Called by the owner of the table, which alone holds a StyleKey
        /// @brief Sets the built-in header background colors for Enabled or Disabled: the look everything else overrides
        void SetBuiltInBackgroundColors(const StyleKey& key, std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Sets the built-in colors of the border around the header band for Enabled or Disabled
        void SetBuiltInOuterBorderColors(const StyleKey& key, std::span<const glm::vec4> colors, const ColorState state);

    private:
        void restyle() override;

        /// @brief Takes the text settings through the layers and tells the Table when what shows changed
        void resolveText();

        /// @brief Tells the Table the texts' font, alignment or padding changed, so it styles its texts again.
        void notifyTextChange();

        /// @brief Enabled or Disabled: the states a header has
        static ColorState headerState(const ColorState state);

        /// @brief A width: set here, else the flavors', else 1 pixel
        f32 shownWidth(const std::optional<f32>& own, const StylePart part) const;

        LayeredColors
            _background {},
            _innerBorder {},
            _outerBorder {},
            _text {};
        std::optional<f32>
            _innerBorderWidth {}, //set here
            _outerBorderWidth {};
        TableTextLayers _textSettings;
        std::function<void()> _onTextChange {};
    };

    /// @brief The checkbox drawn in every checkbox cell: one background, border and checkmark, colored per cell. Enabled, Hover
    ///        and Clicked, and Disabled for a read-only track or a disabled table; the checkmark follows its track's state and
    ///        icon colors. Reached through table.GetCellCheckBox()
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

        /// @brief Returns the checkmark drawn in checked boxes. Its colors are the tracks' icon colors (GetTrackStyle(n) and
        ///        GetCellStyle()), set for each cell; its fill type and gradient are set here
        Icon& GetCheckmark();

        /// @brief Returns the checkmark drawn in checked boxes
        const Icon& GetCheckmark() const;

    //Called by the Table
        /// @brief Shows the colors of the given state, for the cell drawn next
        void SetColorState(const ColorState state);

        /// @brief Colors the checkmark for the cell drawn next: its track's icon colors in the track's state
        void SetCheckmarkColors(std::span<const glm::vec4> colors);

    private:
        Background _background;
        Border _border;
        Icon _checkmark;
    };
}