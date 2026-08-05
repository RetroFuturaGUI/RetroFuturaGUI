#pragma once
#include "IWidget.hpp"
#include "Texture.hpp"

namespace RetroFuturaGUI
{
    class Image2D : public IWidget
    {
    public:
        Image2D(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view imagePath);
        Image2D() = delete;
        Image2D(const Image2D&) = delete;
        Image2D(Image2D&&) = delete;
        auto operator =(const Image2D&) = delete;
        auto operator =(Image2D&&) = delete;
        ~Image2D() = default;
        
        void Draw() override;
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const f32 rotation) override;
        void FitGeometryToTexture();
        u32 GetTextureID();

    private:
        glm::vec2 _quadSize { 1.0f };
        f32 _textureAspectRatio { 0.0f };
        std::unique_ptr<Texture> _texture;
    };
}