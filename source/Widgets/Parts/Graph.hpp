#pragma once
#include "IncludeHelper.hpp"
#include "IWidgetDecoration.hpp"
#include "WavePlane.hpp"
#include <functional>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief How a ranged widget's graph is drawn: a solid fill (Bar) or a wave (Wave)
    enum class GraphMode : u32
    {
        Bar,
        Wave
    };

    /// @brief What a graph tells its owning widget, for a layout that depends on it
    struct GraphHooks
    {
        std::function<void()> _OnLayoutChange {};
    };

    /// @brief The fill growing along a ranged widget's track: a bar, or a wave. The widget owns it and hands it out through
    ///        GetGraph(), so it is styled as progressBar.GetGraph().SetColors(...)
    class Graph final : public IWidgetDecoration
    {
    public:
        explicit Graph(Projection* projection, GraphHooks hooks = {});
        Graph() = delete;
        Graph(const Graph&) = delete;
        Graph(Graph&&) = delete;
        ~Graph() = default;
        auto operator =(const Graph&) = delete;
        auto operator =(Graph&&) = delete;

        /// @brief Shows or hides the graph. Hidden by default
        void Show(const bool show);

        /// @brief Returns whether the graph is shown
        bool IsShown() const;

        /// @brief Sets whether the graph is drawn as a solid fill (Bar) or as a wave (Wave)
        void SetMode(const GraphMode mode);

        /// @brief Returns how the graph is drawn
        GraphMode GetMode() const;

        /// @brief Sets the thickness of the wave's line, in pixels (Wave mode only)
        void SetWaveThickness(const f32 thickness);

        /// @brief Sets the amplitude of the wave, in pixels (Wave mode only)
        void SetWaveHeight(const f32 height);

        /// @brief Sets the distance between the wave's peaks, in pixels (Wave mode only)
        void SetWaveLength(const f32 length);

        /// @brief Sets the graph's thickness, its size across the track. Values at or above the track's height are clamped to it.
        ///        0 is the default which matches the track's height
        void SetWidth(const f32 width);

        /// @brief Returns the graph's thickness as set; 0 matches the track's height
        f32 GetWidth() const;

        /// @brief Returns the colors for the given state
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Sets the fill type (solid, linear/radial/huestar gradient)
        void SetFillType(const FillType fillType);

        /// @brief Sets the offset applied to the gradient's start position
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the gradient animates over time
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the linear gradient (in degrees)
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the gradient rotates over time
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the glass-effect-with-image shader feature. 0 turns the glass off
        void SetWindowImageTextureID(const u32 textureID);

        /// @brief Sets the noise the glass is made of. How big its cells are, how far it shifts the window behind, its smoothing and seed
        void SetGlassNoise(const GlassNoise& noise);

        /// @brief Pixelates what was drawn behind, before this draws, in square blocks this many pixels wide, under the fill.
        ///        The fill's alpha decides how much of it shows. It takes the glass's place where both are on. 0 turns it off
        void SetPixelateBlockSize(const f32 blockSize);

        /// @brief Shows what was drawn behind, before this draws, as on a CRT screen. Red and blue pulled apart sideways and
        ///        dark horizontal scanlines, under the fill. Never pixelated: where both are on, it takes the pixelation's place
        void SetCrtEffect(const CrtEffect& crtEffect);

        /// @brief Sets which pattern the raster drawn. Dotted (the default), Checkered, Striped or Grid
        void SetRasterPattern(const RasterPattern rasterPattern);

        /// @brief Sets the raster's primary color. The dots, one of the two square colors, every other stripe, or the grid's
        ///        lines across the raster degree
        void SetPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the raster's secondary color. The other square color, the stripes between the primary ones, or the
        ///        grid's lines along the raster degree
        void SetSecondaryRasterColor(const glm::vec4& color);

        /// @brief Sets the gap, in pixels, between the widest raster elements: each element sits centered in a grid cell as large as
        ///        the largest width plus this gap, so 0 lets the widest elements touch while narrower ones keep more room. The
        ///        elements keep their size; a larger largest width spreads the grid with it
        void SetRasterSpacing(const f32 spacing);

        /// @brief Sets the direction, in degrees, along which the raster widths and animation transfer
        void SetRasterDegree(const f32 degree);

        /// @brief Sets the per-position dot width curve, in pixels, for the Dotted raster pattern. Enables the raster when non-empty
        void SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each dot's opacity reaches from its center before fading to transparent
        void SetDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the raster pattern animates
        void SetRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the fog effect
        void SetFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the fog drifts over time
        void SetFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density curve driving the fog. Enables the fog when non-empty
        void SetFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the fog appears; higher values carve larger clear gaps out of the cloud
        void SetFogClearing(const f32 clearing);

        /// @brief Sets the noise the fog is made of. How big its cloud and shading cells are, its smoothing and seed
        void SetFogNoise(const FogNoise& noise);

    //Called by the owning widget
        /// @brief Draws the graph, while it is shown
        void Draw() override;

        /// @brief Sizes the graph: its length along the track and its thickness across it
        void SetSize(const glm::vec2& size) override;

        /// @brief Places the graph on the track
        void SetPosition(const glm::vec3& position) override;

        /// @brief Turns the graph with the track
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Returns the size the graph is drawn at
        glm::vec2 GetSize() const override;

        /// @brief Returns where the graph is drawn
        glm::vec3 GetPosition() const override;

        /// @brief Returns how the graph is turned
        glm::vec3 GetRotation() const override;

        /// @brief Has the colors and corner radii move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

    private:
        /// @brief Hands the wave plane the colors of the current state. A Selected state without colors shows the Enabled ones
        void setColors() override;
        void setCornerShape() override;
        void setFill() override;

        /// @brief Tells the widget its layout changed, if it asked to know
        void notifyLayoutChange();

        WavePlane _wavePlane;
        std::function<void()> _onLayoutChange {};
        std::vector<f32>
            _primaryRasterWidthTransfer {},
            _fogDensity {}; //the wave plane keeps spans over these
        GraphMode _mode { GraphMode::Bar };
        f32 _width { 0.0f };
        bool _isShown { false };
    };
}