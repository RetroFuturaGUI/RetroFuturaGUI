#pragma once
#include "IWidget.hpp"
#include "Texture.hpp"

namespace RetroFuturaGUI
{
    class Image final : public IWidget
    {
    public:
        /// @brief Constructs an Image widget that loads and displays the texture at imagePath (UTF-8).
        Image(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view imagePath);

        /// @brief Constructs an Image widget that shows nothing until SetImage gives it a picture.
        Image(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Image() = delete;
        Image(const Image&) = delete;
        Image(Image&&) = delete;
        auto operator =(const Image&) = delete;
        auto operator =(Image&&) = delete;
        ~Image() = default;

        /// @brief Draws the image.
        void Draw() override;

        /// @brief Sets the box the image is fitted into, its aspect ratio kept.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the image.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the image.
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Shows a picture decoded before, on any thread (Texture::DecodeFile, DecodeMemory), in place of the one before
        void SetImage(DecodedImage&& image);

        /// @brief Shows nothing again.
        void ClearImage();

        /// @brief Fits the picture inside the widget's size, as large as it goes with its aspect ratio kept.
        void FitGeometryToTexture();

        /// @brief Returns the OpenGL texture object ID backing this image, 0 without a picture.
        u32 GetTextureID();

        /// @brief The picture's width over its height, 0 without a picture.
        f32 GetAspectRatio() const;

        /// @brief The picture's size in pixels, 0 by 0 without one.
        glm::i32vec2 GetResolution() const;

    private:
        f32 _textureAspectRatio { 0.0f };
        std::unique_ptr<Texture> _texture;
    };
}