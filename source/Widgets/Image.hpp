#pragma once
#include "IWidget.hpp"
#include "Texture.hpp"

namespace RetroFuturaGUI
{
    class Image : public IWidget
    {
    public:
        /// @brief Constructs an Image widget that loads and displays the texture at imagePath.
        Image(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view imagePath);
        Image() = delete;
        Image(const Image&) = delete;
        Image(Image&&) = delete;
        auto operator =(const Image&) = delete;
        auto operator =(Image&&) = delete;
        ~Image() = default;

        /// @brief Draws the image.
        void Draw() override;

        /// @brief Sets the size of the image quad.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the image.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the image.
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Resizes the widget geometry to match the texture's native aspect ratio.
        void FitGeometryToTexture();

        /// @brief Returns the OpenGL texture object ID backing this image.
        u32 GetTextureID();

    private:
        glm::vec2 _quadSize { 1.0f };
        f32 _textureAspectRatio { 0.0f };
        std::unique_ptr<Texture> _texture;
    };
}