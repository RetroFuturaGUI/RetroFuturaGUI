#include "StillCache.hpp"
#include "Window.hpp" //FileExplorer.hpp holds its window by unique_ptr, which deletes it here
#include <algorithm>

std::optional<RetroFuturaGUI::DecodedImage> RetroFuturaGUI::FileExplorer::StillCache::Find(const std::filesystem::path& path, const std::filesystem::file_time_type writeTime, const i32 minSide)
{
    const std::lock_guard lock { _mutex };
    const auto found { _stills.find(path.native()) };

    if(_stills.end() == found)
        return std::nullopt;

    Still& still { found->second };

    if(still._WriteTime != writeTime)
        return std::nullopt; //the file changed since

    //Large enough, or as large as the video gets
    const glm::i32vec2 resolution { still._Image._Resolution };
    const bool isLargeEnough { minSide <= std::max(resolution.x, resolution.y) || resolution == still._Image._SourceResolution };

    if(!isLargeEnough)
        return std::nullopt;

    still._LastUsed = ++_useCount;
    return still._Image;
}

void RetroFuturaGUI::FileExplorer::StillCache::Keep(const std::filesystem::path& path, const std::filesystem::file_time_type writeTime, DecodedImage still)
{
    const std::lock_guard lock { _mutex };
    _stills.insert_or_assign(path.native(), Still { ._WriteTime = writeTime, ._Image = std::move(still), ._LastUsed = ++_useCount });
    letGoOfOldest();
}

void RetroFuturaGUI::FileExplorer::StillCache::SetStillSide(const i32 side)
{
    _stillSide = side;
}

i32 RetroFuturaGUI::FileExplorer::StillCache::GetStillSide() const
{
    return _stillSide;
}

void RetroFuturaGUI::FileExplorer::StillCache::letGoOfOldest()
{
    while(_stillLimit < _stills.size())
    {
        const auto oldest { std::ranges::min_element(_stills, {}, [](const auto& entry) { return entry.second._LastUsed; }) };
        _stills.erase(oldest);
    }
}