#include "ResourceManager.hpp"

RetroFuturaGUI::ResourceManager::ResourceManager()
{
    _checkmark = std::make_shared<SvgTexture>(R"(\Resources\img\checkmark.svg)");
}

std::shared_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::GetCheckmarkIcon()
{
    if(!_checkmark)
        return nullptr;

    return _checkmark;
}