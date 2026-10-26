#pragma once
#include "IWidget.hpp"
#include "Plane.hpp"
#include "Texture.hpp"
#include <memory>
#include <vector>

namespace RetroFuturaGUI
{
    class Image final : public IWidget
    {
    public:
        /// @brief Constructs an Image widget that shows the image at imagePath (UTF-8). ResourceManager loads it and keeps it
        ///        until ResourceManager::Free.
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
        Texture* _texture { nullptr }; //the picture shown: ResourceManager's for a file, _decodedTexture for a decoded picture
        std::unique_ptr<Texture> _decodedTexture { nullptr };
        std::unique_ptr<Plane> _plane { nullptr }; //shows the texture as its image
        std::vector<glm::vec4> _planeColors { glm::vec4(0.0f) }; //transparent, so the picture shows as it is. The plane keeps a span over it

        /// @brief Shows the texture, or nothing for null.
        void showTexture(Texture* texture);
    };
}