#include "ThumbnailCache.hpp"
#include "AudioMetadataReader.hpp"
#include "PlatformBridge.hpp"
#include "Table.hpp"
#include "VideoMetadata.hpp"
#include "VideoStill.hpp"
#include "Window.hpp"
#include <algorithm>
#include <string_view>

RetroFuturaGUI::FileExplorer::ThumbnailCache::ThumbnailCache()
{
    constexpr std::array<std::string_view, 5> iconFiles { "FolderIcon.svg", "ImageIcon.svg", "AudioIcon.svg", "VideoIcon.svg", "FileIcon.svg" }; //in FileKind's order
    const std::filesystem::path iconFolder { PlatformBridge::Paths::GetExecutablePathFSPath().parent_path() / "Resources" / "img" };

    for(uSize kind = 0; kind < iconFiles.size(); ++kind)
    {
        std::optional<DecodedImage> icon { Texture::DecodeFile(iconFolder / iconFiles[kind], true, _iconSide) };

        if(!icon)
            continue; //that kind shows nothing

        _icons[kind] = std::make_unique<Texture>(std::move(*icon));
    }
}

void RetroFuturaGUI::FileExplorer::ThumbnailCache::ShowIcons(Table& table, const std::vector<Entry>& entries, const uSize column)
{
    for(uSize row = 0; row < entries.size(); ++row)
        table.SetValue(iconOf(fileKindOf(entries[row])), TrackIndex { ._Row = row, ._Column = column, ._Layer = 0 });

    //No row holds a thumbnail now, so none has to be given its icon back
    _shownFirstRow = 0;
    _shownEndRow = 0;
}

void RetroFuturaGUI::FileExplorer::ThumbnailCache::Update(Table& table, const std::vector<Entry>& entries, const uSize column)
{
    letGoOfStuckJob(); //makes a new worker, so it comes before the guard

    if(!_work)
        return;

    ++_updateCount;
    _work->Drain();

    uSize
        firstRow { 0 },
        endRow { 0 };
    table.GetDisplayedRows(firstRow, endRow);
    endRow = std::min(endRow, entries.size());

    //Rows that left the view show their icons again, so a thumbnail let go below is never drawn
    for(uSize row = _shownFirstRow; row < _shownEndRow && row < entries.size(); ++row)
    {
        if(firstRow <= row && endRow > row)
            continue;

        table.SetValue(iconOf(fileKindOf(entries[row])), TrackIndex { ._Row = row, ._Column = column, ._Layer = 0 });
    }

    for(uSize row = firstRow; row < endRow; ++row)
        table.SetValue(pictureOf(entries[row]), TrackIndex { ._Row = row, ._Column = column, ._Layer = 0 });

    _shownFirstRow = firstRow;
    _shownEndRow = endRow;
    letGoOfOldest();
}

RetroFuturaGUI::FileExplorer::ThumbnailCache::Decoded RetroFuturaGUI::FileExplorer::ThumbnailCache::decode(Request&& request, std::stop_token)
{
    Decoded decoded { ._Path = std::move(request._Path), ._Image = {} };
    const std::string path { toUtf8(decoded._Path) };

    switch(request._Kind)
    {
        case FileKind::Image:
            decoded._Image = Texture::DecodeFile(decoded._Path, true, _pictureSide);
        break;
        case FileKind::Audio:
        {
            const std::optional<AudioMetadata> audio { AudioMetadataReader::Read(path) };

            if(!audio || audio->_CoverArt.empty())
                break; //no cover art: the icon shows

            decoded._Image = Texture::DecodeMemory(audio->_CoverArt, true, _pictureSide);
        }
        break;
        case FileKind::Video:
        {
            const std::optional<VideoMetadata> video { ReadVideoMetadata(path) };

            if(!video || !video->_VideoCodecIsFree)
                break;

            decoded._Image = DecodeVideoFrame(path, static_cast<i64>(video->_Seconds * 100.0), _pictureSide, true); //10% in, in milliseconds
        }
        break;
        default:
        break;
    }

    return decoded;
}

void RetroFuturaGUI::FileExplorer::ThumbnailCache::take(Decoded&& decoded)
{
    const std::filesystem::path::string_type& key { decoded._Path.native() };

    if(!_pendingKeys.empty() && _pendingKeys.front() == key)
        _pendingKeys.pop_front();

    _waitingSince = std::chrono::steady_clock::now(); //next job starts now

    const auto found { _thumbnails.find(key) };

    if(_thumbnails.end() == found)
        return;

    Thumbnail& thumbnail { found->second };
    thumbnail._IsPending = false;

    if(!decoded._Image)
        return; // file keeps its icon

    thumbnail._Texture = std::make_unique<Texture>(std::move(*decoded._Image)); //an older one goes; Update sets every row it showed again before the next draw
}

const RetroFuturaGUI::ITexture* RetroFuturaGUI::FileExplorer::ThumbnailCache::pictureOf(const Entry& entry)
{
    const FileKind kind { fileKindOf(entry) };

    if(!_work)
        return iconOf(kind);

    const bool hasPicture { FileKind::Image == kind || FileKind::Audio == kind || FileKind::Video == kind };

    //Reading a cloud file that isn't on this computer would download all of it
    if(!hasPicture || entry._IsOnlineOnly)
        return iconOf(kind);

    const std::filesystem::path::string_type& key { entry._Path.native() };
    const auto found { _thumbnails.find(key) };

    if(_thumbnails.end() != found)
    {
        Thumbnail& thumbnail { found->second };
        thumbnail._LastShownUpdate = _updateCount;

        //A file changed since its thumbnail was made gets a new one, once the one being made is done
        const bool isCurrent { thumbnail._WriteTime == entry._LastWriteTime };

        if(isCurrent || thumbnail._IsPending)
            return thumbnail._Texture ? static_cast<const ITexture*>(thumbnail._Texture.get()) : iconOf(kind);
    }

    if(_pendingLimit <= _pendingKeys.size())
        return iconOf(kind); //asked for once the queue has room

    Thumbnail& thumbnail { _thumbnails[key] };
    const ITexture* shown { thumbnail._Texture ? static_cast<const ITexture*>(thumbnail._Texture.get()) : iconOf(kind) }; //the old thumbnail until the new one is ready
    thumbnail._WriteTime = entry._LastWriteTime;
    thumbnail._LastShownUpdate = _updateCount;
    thumbnail._IsPending = true;

    if(_pendingKeys.empty())
        _waitingSince = std::chrono::steady_clock::now();

    _pendingKeys.push_back(key);
    _work->Post([request = Request { ._Path = entry._Path, ._Kind = kind }](std::stop_token stop) mutable { return decode(std::move(request), stop); },
                [this](Decoded&& decoded) { take(std::move(decoded)); });
    return shown;
}

const RetroFuturaGUI::ITexture* RetroFuturaGUI::FileExplorer::ThumbnailCache::iconOf(const FileKind kind) const
{
    const uSize index { static_cast<uSize>(kind) };
    return index < _icons.size() ? _icons[index].get() : nullptr;
}

void RetroFuturaGUI::FileExplorer::ThumbnailCache::letGoOfOldest()
{
    if(_thumbnailLimit >= _thumbnails.size())
        return;

    //The rows in view were shown in this update, and a thumbnail being made is still waited for
    std::vector<std::pair<u64, std::filesystem::path::string_type>> candidates {};

    for(const auto& [key, thumbnail] : _thumbnails)
    {
        if(_updateCount == thumbnail._LastShownUpdate || thumbnail._IsPending)
            continue;

        candidates.emplace_back(thumbnail._LastShownUpdate, key);
    }

    std::sort(candidates.begin(), candidates.end(), [](const auto& left, const auto& right) { return left.first < right.first; });

    for(const auto& candidate : candidates)
    {
        if(_thumbnailLimit >= _thumbnails.size())
            return;

        _thumbnails.erase(candidate.second);
    }
}

void RetroFuturaGUI::FileExplorer::ThumbnailCache::letGoOfStuckJob()
{
    if(_pendingKeys.empty())
        return;

    if(_jobPatience > std::chrono::steady_clock::now() - _waitingSince)
        return;

    //The jobs run in the order posted, so the first still waited for is the one stuck; it stays without a thumbnail
    const auto stuck { _thumbnails.find(_pendingKeys.front()) };

    if(_thumbnails.end() != stuck)
        stuck->second._IsPending = false;

    _pendingKeys.pop_front();

    //The jobs queued behind it go with the worker, so their files are asked for again. Update gives every row it showed a
    //picture again right after this, so no cell keeps a thumbnail erased here.
    for(const std::filesystem::path::string_type& key : _pendingKeys)
        _thumbnails.erase(key);

    _pendingKeys.clear();
    _work = std::make_unique<BackgroundWork>(BackgroundWork::WhenDestroyed::LetJobFinish);
}