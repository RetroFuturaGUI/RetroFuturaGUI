#include "ResourceManager.hpp"
#include "PlatformBridge.hpp"

RetroFuturaGUI::ResourceManager::ResourceManager()
{
    std::filesystem::path imgPath = PlatformBridge::Paths::GetExecutablePathFSPath().parent_path();
    imgPath /= "Resources";
    imgPath /= "img";

    std::filesystem::path checkmarkPath = imgPath / "checkmark.svg";
    _checkmark = std::make_shared<SvgTexture>(checkmarkPath.string(), true);

    std::filesystem::path arrowDownPath = imgPath / "DropDownArrow.svg";
    _arrowDown = std::make_shared<SvgTexture>(arrowDownPath.string(), true);
}

std::shared_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::GetCheckmarkIcon()
{
    if(!GetInstance()._checkmark)
        return nullptr;

    return GetInstance()._checkmark;
}

std::shared_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::GetDropDownArrowIcon()
{
    if(!GetInstance()._arrowDown)
        return nullptr;

    return GetInstance()._arrowDown;
}