#pragma once
#include "IWidget.hpp"
#include "SvgTexture.hpp"

namespace RetroFuturaGUI
{
    class SvgImage final : public IWidget
    {
    public:
        /// @brief Constructs an SvgImage widget that loads and displays the texture at SvgImagePath.
        SvgImage(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view SvgImagePath);
        SvgImage() = delete;
        SvgImage(const SvgImage&) = delete;
        SvgImage(SvgImage&&) = delete;
        auto operator =(const SvgImage&) = delete;
        auto operator =(SvgImage&&) = delete;
        ~SvgImage() = default;

        /// @brief Draws the SvgImage.
        void Draw() override;

        /// @brief Sizes the drawing to the widget as its image fit says.
        void FitGeometryToTexture();

        /// @brief Sets how the drawing fills the widget. Fit (the default) keeps its proportions and shows it whole, centered. Stretch
        ///        fills the widget exactly, whatever its proportions, such as a frame over a whole scene. Fill keeps its proportions and
        ///        covers the widget; unlike a background's image, the overhang isn't cut off
        void SetImageFit(const ImageFit fit);

        ImageFit GetImageFit() const;

        /// @brief Sets or replaces a color-effect override (solid/gradient) for a named SVG path.
        void SetPathFill(std::string_view name, const SvgPathFill& fill);

        /// @brief Clears a named path's color override, reverting it to its authored SVG color.
        void ClearPathFill(std::string_view name);

        /// @brief Returns every named (id-bearing) path/shape discovered in the SVG.
        const std::vector<SvgPathInfo>& GetNamedPaths() const;

    protected:
        /// @brief Sets the size of the SvgImage quad.
        void applySize(const glm::vec3& size) override;

        /// @brief Sets the world position of the SvgImage.
        void applyPosition(const glm::vec3& position) override;

    private:
        /// @brief Rasterizes the drawing again when the size it is shown at changed, so it stays sharp however it is stretched or
        ///        enlarged. Called while drawing, since uploading the picture needs the window's context.
        void rasterizeAtShownSize();

        static constexpr i32 _maxRasterSide { 8192 }; //pixels per side, so a huge widget doesn't ask for a huge picture.
        glm::vec2 _quadSize { 1.0f };
        f32 _svgTextureAspectRatio { 0.0f };
        ImageFit _imageFit { ImageFit::Fit };
        std::unique_ptr<SvgTexture> _svgTexture;
    };
}