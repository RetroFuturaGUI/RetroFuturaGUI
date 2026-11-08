#pragma once
#include "IncludeHelper.hpp"
#include <array>
#include <functional>
#include <memory>
#include <span>
#include "Plane.hpp"
#include "ShadowRectangle.hpp"
#include "GlowRectangle.hpp"
#include "SvgTexture.hpp"
#include "Texture.hpp"
#include "IWidgetDecoration.hpp"

namespace RetroFuturaGUI
{
    class Border;

    /// @brief What a background tells its owning widget, for a layout that depends on it
    struct BackgroundHooks
    {
        std::function<void()> _OnCornerRadiiChange {};
    };

    /// @brief A widget's background: a filled or gradient rectangle with optional raster, fog, glass, mask and image. The widget
    ///        owns it and hands it out through GetBackground(), so it is styled as button.GetBackground().SetColors(...)
    class Background final : public IWidgetDecoration
    {
        friend class IWidget; //sets the inset of the widget's border gap
        friend class Border; //draws the glow over itself when it may cover the border

    public:
        explicit Background(Projection* projection, BackgroundHooks hooks = {});
        Background() = delete;
        Background(const Background&) = delete;
        Background(Background&&) = delete;
        ~Background() = default;
        auto operator =(const Background&) = delete;
        auto operator =(Background&&) = delete;

        /// @brief Draws the fill, then an SVG image over it
        void Draw() override;

        /// @brief Sizes the background.
        void SetSize(const glm::vec2& size) override;

        /// @brief Places the background
        void SetPosition(const glm::vec3& position) override;

        /// @brief Turns the background
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Returns the background's size
        glm::vec2 GetSize() const override;

        /// @brief Returns the background's position
        glm::vec3 GetPosition() const override;

        /// @brief Returns the background's rotation
        glm::vec3 GetRotation() const override;

        /// @brief Has the colors and corner radii move to new values over the owning widget's transitions.
        void SetTransitionSettings(const TransitionSettings* settings);

        /// @brief Returns the drop shadow of the background: GetDropShadow().Show(true). Drawn just behind it
        DropShadow& GetDropShadow();
        const DropShadow& GetDropShadow() const;

        /// @brief Returns the glow around the background: GetGlow().Show(true). Under the border unless EnableGlowOverBorder
        Glow& GetGlow();
        const Glow& GetGlow() const;

        /// @brief Sets whether the glow is drawn over the widget's border, or under it, showing in the gap between them and
        ///        outside the widget. Off by default. Works where the widget links its border and background (the widgets with
        ///        SetBorderGap); elsewhere the glow stays under the border
        void EnableGlowOverBorder(const bool enable);
        bool IsGlowOverBorderEnabled() const;

        /// @brief Sets the offset applied to the background gradient's start position.
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the background gradient animates over time.
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the background's linear gradient, in degrees.
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the background gradient rotates over time.
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the background's glass-effect-with-image shader feature. 0 turns the glass off.
        void SetWindowImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the background's Dotted raster pattern.
        void SetPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the gap, in pixels, between the largest elements of the background's raster: 0 lets them touch. Spreads the grid without changing any element's size.
        void SetRasterSpacing(const f32 spacing);

        /// @brief Sets the direction, in degrees, along which the background's raster widths/animation transfer.
        void SetRasterDegree(const f32 degree);

        /// @brief Sets the per-position dot width curve, in pixels (each value is a dot's full width across), for the background's Dotted raster pattern. Enables the Raster shader feature when non-empty.
        void SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each background dot's opacity reaches from its center before fading to transparent.
        void SetDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the background's raster pattern animates.
        void SetRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the background's FogEffect shader feature.
        void SetFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the background's fog drifts over time.
        void SetFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the background's fog. Enables the FogEffect shader feature when non-empty.
        void SetFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the background's fog appears; higher values carve larger clear gaps out of the cloud.
        void SetFogClearing(const f32 clearing);

        /// @brief Shows an image file (UTF-8) on the background. A picture (png, jpg, ...) sits behind the fill, and comes from
        ///        ResourceManager, which keeps it until ResourceManager::Free. An SVG sits over the fill as an SvgTexture of the
        ///        background's own, so SetImagePathFill can recolor its paths
        void SetImage(std::string_view imagePath);
        /// @brief Returns the raster image texture ID, or 0 when the background has no raster image.
        u32 GetImageTextureID() const;

        /// @brief Sets how far the background image stays clear of the widget's edges
        void SetImagePadding(const f32 padding);

        /// @brief Sets how the background image is laid over the widget: stretched (default), cropped or fitting into it.
        void SetImageFit(const ImageFit fit);

        /// @brief Sets a grayscale mask laid over the background, its red channel the gray: as transparency or as shading
        ///        (SetMaskMode). 0 shows no mask
        void SetMaskTextureID(const u32 textureID);

        /// @brief Sets what the background mask does: make the background transparent/Alpha (default) or shade it (Shade).
        void SetMaskMode(const MaskMode mode);

        /// @brief Sets how the background mask is laid over the widget: stretched (default) or keeping its proportions.
        void SetMaskFit(const ImageFit fit);

        /// @brief Sets or replaces the color override of a named path in an SVG background image. Does nothing for a picture.
        void SetImagePathFill(std::string_view name, const SvgPathFill& fill);

        /// @brief Gives a named path in an SVG background image its own color back.
        void ClearImagePathFill(std::string_view name);

        /// @brief Returns the background colors configured for the given color state.
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Sets the background fill type (solid, linear/radial/huestar gradient).
        void SetFillType(const FillType fillType);

        /// @brief Sets sections of the background to skip drawing; see Plane::SetBackgroundGaps.
        void SetGaps(const BackgroundGap& gap);

    private:
        /// @brief Hands the colors of the current state to the fill.
        void setColors() override;
        
        /// @brief Draws the SVG background image over the fill, inset by the padding wherever the widget put _background.
        void drawSvg();

        /// @brief Whether the path names an SVG file, by its extension in any case.
        static bool isSvgPath(std::string_view path);

        void setCornerShape() override;

        /// @brief Draws the drop shadow of the element as it draws now, if the shadow is shown.
        void drawShadow();

        /// @brief Draws the glow outward from the fill's edge, at the given depth
        void drawGlow(const f32 depth);

        /// @brief Draws the linked border's inner glow under the fill or over it, whichever the border allows, when isOver says so
        void drawBorderGlow(const bool isOver);

        /// @brief Whether the linked border draws the glow, over itself
        bool isGlowDrawnByBorder() const;

        /// @brief Draws the fill this far inside the size and position set, per side: top, right, bottom, left. IWidget sets it for its border gap
        void setInset(const glm::vec4& inset);

        /// @brief Sizes and places the fill inside the size and position set, less the inset
        void applyInset();

    //Geometry
        glm::vec2 _size { 0.0f }; //as set by the widget; GetSize returns it, whatever the inset
        glm::vec3 _position { 0.0f };
        glm::vec4 _inset { 0.0f };

    //Elements
        std::unique_ptr<Plane> _backgroundPlane { nullptr };
        DropShadow _dropShadow;
        std::unique_ptr<ShadowRectangle> _shadowRectangle { nullptr };
        Glow _glow;
        std::unique_ptr<GlowRectangle> _glowRectangle { nullptr };
        Border* _linkedBorder { nullptr }; //the widget's border, linked by IWidget::useBorderGap
        bool _isGlowOverBorder { false };
        static constexpr f32
            _glowDepthOffset { 0.0015f }, //behind the fill, in front of the drop shadow
            _coverDepthOffset { 0.001f }; //the border's inner glow, in front of the fill or just behind it
        Texture* _backgroundImage { nullptr }; //a picture borrowed from ResourceManager, shown by _background
        std::unique_ptr<SvgTexture> _backgroundSvg { nullptr }; //an SVG background image, drawn over the fill
        f32 _backgroundImagePadding { 0.0f };

    //Style
        std::vector<f32> _backgroundPrimaryRasterWidthTransfer;
        std::vector<f32> _backgroundFogDensity;
    };
}