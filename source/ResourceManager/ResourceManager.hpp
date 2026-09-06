#pragma once
#include "SvgTexture.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    class ResourceManager
    {
    public:
        static std::shared_ptr<SvgTexture> GetCheckmarkIcon();
        static constexpr const glm::vec4 _Eigengrau { 0.086f, 0.086f, 0.114f, 1.0f };
    
    private:
        ResourceManager();
        ~ResourceManager() = default;
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        auto operator=(const ResourceManager&) = delete;
        auto operator=(ResourceManager&&) = delete;
        static ResourceManager& GetInstance()
        {
            static ResourceManager Instance;
            return Instance;
        }

        static inline std::shared_ptr<SvgTexture> _checkmark { nullptr };
    };
}