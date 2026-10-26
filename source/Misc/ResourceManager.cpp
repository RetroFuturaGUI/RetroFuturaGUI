#include "ResourceManager.hpp"
#include "PlatformBridge.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <print>

RetroFuturaGUI::Texture* RetroFuturaGUI::ResourceManager::GetTexture(std::string_view path)
{
    const uSize hash { hashPath(path) };
    const auto found { _textures.find(hash) };

    if(_textures.end() != found)
        return found->second.get();

    return _textures.emplace(hash, std::make_unique<Texture>(path, true)).first->second.get();
}

std::unique_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::CreateSvgTexture(std::string_view path, Projection* projection, const glm::i32vec2 initialResolution)
{
    const uSize hash { hashPath(path) };
    auto found { _svgSources.find(hash) };

    if(_svgSources.end() == found)
    {
        //UTF-8 into a native path, so names outside the system's code page open too!
        std::ifstream file { std::filesystem::path(std::u8string(path.begin(), path.end())), std::ios::binary };

        if(!file)
        {
            std::println("Error reading SVG: {}", path);
            return nullptr;
        }

        found = _svgSources.emplace(hash, std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>())).first;
    }

    return std::make_unique<SvgTexture>(found->second, path, true, projection, initialResolution);
}

std::unique_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::CreateCheckmarkIcon(Projection* projection)
{
    return CreateSvgTexture(getIconPath("checkmark.svg"), projection);
}

std::unique_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::CreateDropDownArrowIcon(Projection* projection)
{
    return CreateSvgTexture(getIconPath("DropDownArrow.svg"), projection);
}

void RetroFuturaGUI::ResourceManager::Free(std::string_view path)
{
    const uSize hash { hashPath(path) };
    _textures.erase(hash);
    _svgSources.erase(hash);
}

void RetroFuturaGUI::ResourceManager::Release()
{
    _textures.clear();
    _svgSources.clear();
}

uSize RetroFuturaGUI::ResourceManager::hashPath(std::string_view path)
{
    return std::hash<std::string_view>{}(path);
}

std::string RetroFuturaGUI::ResourceManager::getIconPath(std::string_view fileName)
{
    const std::filesystem::path iconPath { PlatformBridge::Paths::GetExecutablePathFSPath().parent_path() / "Resources" / "img" / fileName };
    const std::u8string text { iconPath.u8string() };
    return std::string(text.begin(), text.end());
}