#pragma once
#include "IncludeHelper.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    class Texture
    {
    public:
        Texture(std::string_view path, const bool flipVertically = true);

        bool IsTextureVerticallyFlipped() const;
        glm::i32vec2 GetResolution() const;
        i32 GetColorChannelCount() const;
        f32 GetAspectRatio() const;
        std::vector<u8>* GetTextureData();

    private:
        void loadTexture(std::string_view path);

        bool _verticallyFlipped = false;
        f32 _aspectRatio = 0.0f;
        //std::string _path;
        glm::i32vec2 _resolution = glm::u32vec2(0);
        i32 _colorChannelCount = 0;
        std::vector<u8> _texture;
    };
}