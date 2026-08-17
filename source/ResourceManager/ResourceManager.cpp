#include "ResourceManager.hpp"
#include "PlatformBridge.hpp"

RetroFuturaGUI::ResourceManager::ResourceManager()
{
    std::filesystem::path checkmarkPath = PlatformBridge::Paths::GetExecutablePathFSPath().parent_path();
    checkmarkPath /= "Resources";
    checkmarkPath /= "img";
    checkmarkPath /= "checkmark.svg";

    _checkmark = std::make_shared<SvgTexture>(checkmarkPath.string());
}

std::shared_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::GetCheckmarkIcon()
{
    if(!GetInstance()._checkmark)
        return nullptr;

    return GetInstance()._checkmark;
}