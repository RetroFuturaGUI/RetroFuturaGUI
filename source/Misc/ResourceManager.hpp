#pragma once
#include "SvgTexture.hpp"
#include "Texture.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace RetroFuturaGUI
{
    /// @brief Manages, shares and copies resources loaded from disk
    class ResourceManager
    {
    public:
        /// @brief The texture loaded from the image file at path (UTF-8). It is kept until Free(path), so every widget showing the file borrows the same one.
        static Texture* GetTexture(std::string_view path);

        /// @brief A new SvgTexture made from the SVG file at path (UTF-8). Its colors and size can be changed without touching anyone else's. After Free(path) no new copies can be acquired.
        static std::unique_ptr<SvgTexture> CreateSvgTexture(std::string_view path, Projection* projection = nullptr, const glm::i32vec2 initialResolution = glm::i32vec2(0));

        /// @brief A new checkmark icon of its own; see CreateSvgTexture.
        static std::unique_ptr<SvgTexture> CreateCheckmarkIcon(Projection* projection);

        /// @brief A new drop-down arrow icon of its own; see CreateSvgTexture.
        static std::unique_ptr<SvgTexture> CreateDropDownArrowIcon(Projection* projection);

        /// @brief Frees the texture/svg loaded from path. Whoever frees a Texture must make sure no widget still uses it. SvgTextures already made stay with their owners.
        static void Free(std::string_view path);

        /// @brief Frees every texture and SVG image, so the textures go while GL still runs.
        static void Release();
        static constexpr const glm::vec4 _Eigengrau { 0.086f, 0.086f, 0.114f, 1.0f };

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        auto operator=(const ResourceManager&) = delete;
        auto operator=(ResourceManager&&) = delete;

        /// @brief The key a texture or an SVG text is kept under.
        static uSize hashPath(std::string_view path);

        /// @brief The UTF-8 path of an icon in the Resources/img folder next to the executable.
        static std::string getIconPath(std::string_view fileName);

        static inline std::unordered_map<uSize, std::unique_ptr<Texture>> _textures {}; //by their path's hash
        static inline std::unordered_map<uSize, std::string> _svgSources {}; //each SVG file's text, by its path's hash
    };
}