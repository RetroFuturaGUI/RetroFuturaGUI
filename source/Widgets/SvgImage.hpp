#pragma once
#include "IWidget.hpp"
#include "SvgTexture.hpp"

namespace RetroFuturaGUI
{
    class SvgImage : public IWidget
    {
    public:
        /// @brief Constructs an SvgImage widget that loads and displays the texture at SvgImagePath.
        SvgImage(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view SvgImagePath);
        SvgImage() = delete;
        SvgImage(const SvgImage&) = delete;
        SvgImage(SvgImage&&) = delete;
        auto operator =(const SvgImage&) = delete;
        auto operator =(SvgImage&&) = delete;
        ~SvgImage() = default;

        /// @brief Draws the SvgImage.
        void Draw() override;

        /// @brief Sets the size of the SvgImage quad.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the SvgImage.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the SvgImage.
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Resizes the widget geometry to match the texture's native aspect ratio.
        void FitGeometryToTexture();

        /// @brief Sets or replaces a color-effect override (solid/gradient) for a named SVG path.
        void SetPathFill(std::string_view name, const SvgPathFill& fill);

        /// @brief Clears a named path's color override, reverting it to its authored SVG color.
        void ClearPathFill(std::string_view name);

        /// @brief Returns every named (id-bearing) path/shape discovered in the SVG.
        const std::vector<SvgPathInfo>& GetNamedPaths() const;

    private:
        glm::vec2 _quadSize { 1.0f };
        f32 _svgTextureAspectRatio { 0.0f };
        std::unique_ptr<SvgTexture> _svgTexture;
    };
}