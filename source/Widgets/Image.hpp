#pragma once
#include "IWidget.hpp"
#include "Texture.hpp"

namespace RetroFuturaGUI
{
    class Image : public IWidget
    {
    public:
        Image(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view imagePath);
        Image() = delete;
        Image(const Image&) = delete;
        Image(Image&&) = delete;
        auto operator =(const Image&) = delete;
        auto operator =(Image&&) = delete;
        ~Image() = default;
        
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