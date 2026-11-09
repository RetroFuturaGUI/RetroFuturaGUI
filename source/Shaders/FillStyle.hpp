#pragma once
#include "IncludeHelper.hpp"
#include "Shader.hpp"
#include "Transition.hpp"
#include "Noise.hpp"
#include <chrono>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    enum class RasterPattern : u32
    {
        Dotted,
        Checkered,
        Striped,
        Grid
    };

    /// @brief A CRT screen's look laid over what was drawn behind: its color channels pulled apart sideways and dark
    ///        horizontal scanlines. Sizes are in pixels. With neither a displacement nor scanlines it is off
    struct CrtEffect
    {
        f32 
            _ChannelDisplacement { 0.0f },
            _ScanlineWidth { 0.0f },
            _ScanlineAlpha { 0.5f };

        bool operator ==(const CrtEffect&) const = default;
    };

    /// @brief How an image or a mask is projected
    enum class ImageFit : u32
    {
        Stretch, // across the whole area, whatever its proportions
        Fill, // keeping its proportions and covering the whole area. The overhang gets cut off
        Fit // keeping its proportions and shown whole, centered, leaving the rest of the area without it
    };

    /// @brief What a grayscale mask does to the color under it.
    enum class MaskMode : u32
    {
        Alpha, // black is see-through, white opaque
        Shade // below mid-gray darkens, above it lightens: bright and dark bands turn an image into chrome
    };

    /// @brief The fill and what is laid over it
    class FillStyle
    {
    public:
        /// @brief Has the colors, and a rectangle's corner radii, move to new values over the widget's transitions, or the global
        ///        default where the widget has none. Without settings they switch at once. The settings must outlive this.
        void SetTransitionSettings(const TransitionSettings* settings);

    protected:
        FillStyle() = default;
        ~FillStyle();

        /// @brief Sets the colors of the fill
        void SetColors(std::span<const glm::vec4> colors);

        /// @brief Sets the fill type (solid, linear, radial or conic shift)
        void SetFillType(const FillType fillType);

        /// @brief Sets the offset applied to the gradient's start position.
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets how fast the gradient's colors move on, in runs through all its colors per second. 1 runs through them once a second
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of a linear shift in degrees.
        void SetGradientDegree(const f32 degree);

        /// @brief Sets how fast the gradient turns, in degrees per second
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Returns the offset of the gradient's start position.
        f32 GetGradientOffset() const;

        /// @brief Returns the angle of the gradient, in degrees.
        f32 GetGradientDegree() const;

        /// @brief Sets the window-wide backdrop the glass shows, which turns the glass on. 0 turns it off.
        void SetWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the noise the glass is made of. How big its cells are, how far it shifts the window behind, its smoothing and seed
        void SetGlassNoise(const GlassNoise& noise);

        /// @brief Pixelates what was drawn behind, before this draws, in square blocks this many pixels wide on its own grid, so
        ///        they tilt with it. Laid under the fill like the glass: the fill's alpha decides how much of it shows, so a
        ///        transparent fill shows the pixelation alone. It takes the glass's place where both are on. 0 turns it off
        void SetPixelateBlockSize(const f32 blockSize);

        /// @brief Returns the pixelation's block width in pixels, 0 when it is off
        f32 GetPixelateBlockSize() const;

        /// @brief Shows what was drawn behind, before this draws, as on a CRT screen. Red and blue pulled apart sideways by the
        ///        displacement, and every other line of the scanline width darkened, counted from the top edge. Laid under the
        ///        fill like the pixelation, and never pixelated: where both are on, it takes the pixelation's place
        void SetCrtEffect(const CrtEffect& crtEffect);

        const CrtEffect& GetCrtEffect() const;

        /// @brief Copies the part of the window the quad covers, a block and the CRT displacement wider all round, for the pixelation to show. Called
        ///        right before the draw, so the copy holds everything drawn behind until then. Does nothing while it is off
        /// @param placement The quad's placement into clip space: projection, frame, position, rotation and scaling
        void captureWindowBehind(const glm::mat4& placement);

        /// @brief Sets an image of its own, laid over what is drawn as SetBackgroundImageFit says and inset by
        ///        SetBackgroundImagePadding. It sits behind the fill, whose alpha decides how much of it shows: a fill alpha of 0
        ///        shows the image alone. 0 shows no image.
        /// @note Unlike SetWindowBackgroundImageTextureID, which shows a window-wide backdrop behind the glass, this belongs to
        ///       what is drawn alone. The two can be shown together: the image sits in front of the glass.
        void SetBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets how far the background image stays clear of the edges, per side in pixels.
        void SetBackgroundImagePadding(const f32 padding);

        /// @brief Sets how the background image is laid over what is drawn: stretched (default) or keeping its proportions.
        void SetBackgroundImageFit(const ImageFit fit);

        /// @brief Sets a grayscale mask laid over everything drawn, its red channel the gray, applied as SetMaskMode says. 0 shows no mask.
        void SetMaskTextureID(const u32 textureID);

        /// @brief Sets what the mask does: make the color see-through (Alpha, the default) or shade it (Shade).
        void SetMaskMode(const MaskMode mode);

        /// @brief Sets how the mask is laid over what is drawn: stretched, the default, or keeping its proportions.
        void SetMaskFit(const ImageFit fit);

        /// @brief Sets which pattern the raster draws.
        void SetRasterPattern(const RasterPattern rasterPattern);

        /// @brief Sets the primary raster color. The dot color for Dotted, one of the two square colors for Checkered, every other stripe for Striped, and the lines across the RasterDegree direction for Grid. Alpha blends over whatever the shader has already drawn
        void SetPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the secondary raster color. The other square color for Checkered, every other stripe for Striped, and the lines along the RasterDegree direction for Grid. Alpha blends over whatever the shader has already drawn
        void SetSecondaryRasterColor(const glm::vec4& color);

        /// @brief Sets the gap, in pixels, between the widest raster elements: each element sits centered in a grid cell as large as
        ///        the largest width plus this gap, so 0 lets the widest elements touch while narrower ones keep more room. The
        ///        elements keep their size. A larger largest width spreads the grid with it
        void SetRasterSpacing(const f32 spacing);

        /// @brief Sets the direction in degrees along which raster widths
        void SetRasterDegree(const f32 degree);

        /// @brief Sets the per-position raster width curve in pixels sampled along the RasterDegree direction across what is drawn. Each value is a dot's full width across for Dotted, a square's side length for Checkered, and a line's width for Striped and Grid, each element sized by the value at its center (Grid's lines along the direction by the value where they pass). The largest value plus RasterSpacing sets the grid. Turns the raster on when non-empty
        void SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each dot's opacity reaches from its center before fading to transparent. 1.0 = fully opaque disc, 0.5 = opaque inner half then fades out, 0.0 = only the center pixel is opaque.
        void SetDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets how fast the raster pattern moves along the RasterDegree direction, in pixels per second.
        void SetRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Uploads the feature bits with extraFeatures added, the fill, and the raster and the glass when they are on. A
        ///        gradient moves on by one step. Needs at least one color.
        void uploadFill(Shader& shader, const u32 extraFeatures);

        /// @brief Returns the colors to draw now: the colors given, or on the way to them while a transition runs.
        std::span<const glm::vec4> getShownColors();

        /// @brief Uploads the background image when it is on, on unit 1, and the mask when it is on, on unit 3. size is what
        ///        they are laid over, in pixels, which the padding and the fit are measured against.
        void uploadImage(Shader& shader, const glm::vec2& size) const;

        /// @brief How much of an area of the given size a texture spans per axis when laid over it with the fit: 1 when
        ///        stretched, more along the axis it overhangs when filling, less along the axis it leaves free when fitting.
        ///        Reads the size of the texture bound to GL_TEXTURE_2D.
        static glm::vec2 getCoverage(const ImageFit fit, const glm::vec2& size);

    //Fill
        std::span<const glm::vec4> _colors; //this considers the color source may change many during runtime
        FillType _fillType { FillType::Solid };
        uSize _colorCount { 0 };
        f32
            _gradientOffset { 0.0f },
            _gradientAnimationSpeed { 0.018f }, //runs per second
            _gradientDegree { 45.0f },
            _gradientRotationSpeed { 1.2f }; //degrees per second
        u32
            _shaderFeatureDIP { 0 },
            _windowBackgroundTextureID { 0 };

    //Image (ShaderFeatures::BackgroundImage)
        u32 _backgroundImageTextureID { 0 };
        f32 _backgroundImagePadding { 0.0f };
        ImageFit _backgroundImageFit { ImageFit::Stretch };

    //Mask (ShaderFeatures::Mask)
        u32 _maskTextureID { 0 };
        MaskMode _maskMode { MaskMode::Alpha };
        ImageFit _maskFit { ImageFit::Stretch };

    //Raster (ShaderFeatures::Raster) - the transfer, the spacing, the colors, the angle and the animation serve every
    //pattern; _dotTransparencyTransfer is Dotted's own.
        RasterPattern _rasterPattern { RasterPattern::Dotted };
        std::span<f32> _primaryRasterWidthTransfer;
        uSize _primaryRasterWidthTransferCount { 0 };
        glm::vec4
            _primaryRasterColor { 1.0f },
            _secondaryRasterColor { 1.0f };
        f32
            _rasterSpacing { 0.0f },
            _rasterDegree { 45.0f },
            _dotTransparencyTransfer { 1.0f },
            _rasterAnimationSpeed { 0.0f },
            _rasterAnimationOffset { 0.0f };

    //Transition, with settings only: the fill moves from _fromColors to _targetColors, a copy of the colors last given
        GlassNoise _glassNoise {};
    //Pixelate and CRT (ShaderFeatures::Pixelate, Crt): a copy of the window behind, made right before each draw, serves both
        f32 _pixelateBlockSize { 0.0f };
        CrtEffect _crtEffect {};
        u32 _windowBehindTextureID { 0 };
        glm::ivec2 _windowBehindTextureSize { 0 }; //grown when a larger copy is needed, never shrunk
        glm::vec4 _windowBehindRegion { 0.0f }; //where the copy came from in the window: x, y, width, height
        glm::vec2 _windowBehindViewportSize { 1.0f };
        glm::mat4 _windowBehindPlacement { 1.0f };

        StepClock _stepClock {}; //the time between draws, which the gradient and the raster move on by.

        const TransitionSettings* _transitionSettings { nullptr };
        std::vector<glm::vec4>
            _targetColors {},
            _fromColors {},
            _shownColors {},
            _fromStops {},
            _targetStops {};
        std::chrono::steady_clock::time_point _colorTransitionStart {};
        bool
            _hasShownColors { false }, //colors given before the first draw show at once
            _isColorTransitionRunning { false };

    private:
        /// @brief Takes the colors given as the new target: at once, or from the colors shown now over the transition.
        void startColorTransition(std::span<const glm::vec4> target, const std::chrono::steady_clock::time_point now);

        /// @param seconds The time since the last draw, which the pattern moves on by.
        void uploadRaster(Shader& shader, const f32 seconds);
        void uploadBackgroundImage(Shader& shader, const glm::vec2& size) const;
        void uploadMask(Shader& shader, const glm::vec2& size) const;
        void uploadDotUniforms(Shader& shader) const;
        void uploadSecondaryRasterColor(Shader& shader) const;
        void uploadWindowBehind(Shader& shader) const;
    };
}