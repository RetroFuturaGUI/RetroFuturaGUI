#include "FilePreview.hpp"
#include "Image.hpp"
#include "Label.hpp"
#include "Table.hpp"
#include "Video.hpp"
#include "Window.hpp"
#include <algorithm>
#include <array>
#include <format>

RetroFuturaGUI::FileExplorer::FilePreview::FilePreview(ViewHost& host, Lasagna& lasagna, const ViewCell& cell, const glm::vec2& windowSize, const ViewFont& font, const FileExplorerFlags flags, std::shared_ptr<StillCache> stills)
    : _host(host), _font(font), _flags(flags), _stills(std::move(stills))
{
    //The pane fills its cell, and its own cells are measured from that
    const glm::vec3 fixedSize { cellSize(_viewAxis, windowSize, cell) };
    const glm::vec2 paneSize { fixedSize.x + 2.0f * _margin, fixedSize.y + 2.0f * _margin };
    _pane = host.Attach<Lasagna>("PreviewPane", lasagna, cell, _axis);

    if(!_pane)
        return;

    const ViewCell
        pictureCell { ._Row = 0, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = static_cast<uSize>(MediaLayer::Picture) },
        messageCell { ._Row = 0, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = static_cast<uSize>(MediaLayer::Message) },
        detailsCell { ._Row = 2, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
    _mediaSize = cellSize(_axis, paneSize, pictureCell);
    shareStillSide();
    _picture = host.Attach<Image>("PreviewPicture", *_pane, pictureCell);
    _message = host.Attach<Label>("PreviewMessage", *_pane, messageCell);
    _details = host.Attach<Table>("PreviewDetails", *_pane, detailsCell);

    if(!_picture || !_message || !_details)
        return;

    _picture->SetSize(_mediaSize);
    styleLabel(*_message, "", _mediaSize, _font);
    _message->GetText().SetAlignment(TextAlignment::Center);
    _message->GetText().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.7f, 0.7f, 0.75f, 1.0f) }, ColorState::Enabled);

    //Like the list, without its header
    const glm::vec3 detailsSize { cellSize(_axis, paneSize, detailsCell) };
    _details->SetSize(detailsSize);
    _details->GetBorder().SetBuiltInWidth(styleKey(), 1.0f);
    _details->GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.35f, 0.35f, 0.45f, 1.0f) }, ColorState::Enabled);
    _details->SetTrackAlternatingColorCount(2);
    std::vector<glm::vec4>
        evenRowColors {{ 0.1f, 0.1f, 0.13f, 1.0f }},
        oddRowColors {{ 0.13f, 0.13f, 0.17f, 1.0f }},
        innerBorderColors {{ 0.18f, 0.18f, 0.22f, 1.0f }};
    _details->GetTrackStyle(0).SetBuiltInBackgroundColors(styleKey(), evenRowColors);
    _details->GetTrackStyle(1).SetBuiltInBackgroundColors(styleKey(), oddRowColors);
    _details->GetTrackStyle(0).SetBuiltInInnerBorderColors(styleKey(), innerBorderColors);
    _details->GetTrackStyle(1).SetBuiltInInnerBorderColors(styleKey(), innerBorderColors);
    _details->GetCellStyle().SetBuiltInInnerBorderWidth(styleKey(), 1.0f);
    _details->GetCellStyle().SetFontFamily(_font._Family, _font._Size, FontSizeUnit::Pixels, _font._Slant, _font._Weight);
    _details->GetCellStyle().SetTextAlignment(TextAlignment::Left);
    _details->GetCellStyle().SetTextPadding(6.0f);
    setDetails({});
    showLayer(std::nullopt);
}

bool RetroFuturaGUI::FileExplorer::FilePreview::IsBuilt() const
{
    return _pane && _picture && _message && _details;
}

void RetroFuturaGUI::FileExplorer::FilePreview::Show(const std::vector<Entry>& selection)
{
    //The same file, unchanged, keeps what shows: the folder listed again, or the row picked again
    const bool isSameFile { 1 == selection.size() && 1 == _shownCount && selection.front()._Path == _shownPath && selection.front()._LastWriteTime == _shownWriteTime };

    if(isSameFile)
        return;

    ++_generation; //what a job still loads for the selection before is dropped once it arrives
    closeVideo();
    _shownCount = selection.size();
    _shownPath = 1 == selection.size() ? selection.front()._Path : std::filesystem::path {};
    _shownWriteTime = 1 == selection.size() ? selection.front()._LastWriteTime : std::filesystem::file_time_type {};
    showLayer(std::nullopt);

    if(selection.empty())
    {
        _kind = Kind::None;
        setDetails({});
        return;
    }

    if(1 < selection.size())
    {
        _kind = Kind::Several;
        uSize
            fileCount { 0 },
            folderCount { 0 };
        u64 totalSize { 0 };

        for(const Entry& entry : selection)
        {
            if(PlatformBridge::FileSystem::EntryKind::Directory == entry._Kind)
            {
                ++folderCount;
                continue;
            }

            ++fileCount;
            totalSize += entry._Size;
        }

        std::vector<std::pair<std::string, std::string>> details { { "Selected", std::format("{} items", selection.size()) } };

        if(0 < fileCount)
            details.emplace_back("Files", std::format("{}, {}", fileCount, formatSize(totalSize)));

        if(0 < folderCount)
            details.emplace_back("Folders", std::to_string(folderCount)); //their contents aren't counted

        setDetails(details);
        return;
    }

    const Entry& entry { selection.front() };
    _kind = kindOf(entry, _flags);
    setDetails(fileDetails(entry)); //at once, from the listing; the worker's come on top

    if(Kind::Other == _kind)
        return;

    //A cloud file that isn't on this computer: a provider fetching only what is read gives the headers and a picture; one
    //downloading whole files would fetch all of it, so it isn't previewed
    if(entry._IsOnlineOnly && Kind::Folder != _kind)
    {
        if(PlatformBridge::FileSystem::CloudHydration::Full == PlatformBridge::FileSystem::GetCloudHydration(entry._Path))
        {
            showMessage("Not on this computer");
            return;
        }

        showMessage("Downloading\nto preview...");
    }

    post(Request { ._Generation = _generation, ._Kind = _kind, ._Entry = entry, ._PictureSide = getPictureSide() });
}

void RetroFuturaGUI::FileExplorer::FilePreview::Update()
{
    if(!_work)
        return;

    _work->Drain();
}

RetroFuturaGUI::FileExplorer::FilePreview::Kind RetroFuturaGUI::FileExplorer::FilePreview::GetKind() const
{
    return _kind;
}

void RetroFuturaGUI::FileExplorer::FilePreview::post(Request&& request)
{
    const std::chrono::steady_clock::time_point now { std::chrono::steady_clock::now() };

    //A job still running holds the next one up; one busy this long is stuck, on a download say, and is let go to finish alone, with its own decoder
    if(!_work || (_work->IsBusy() && _jobPatience < now - _lastPostAt))
    {
        _work = std::make_unique<BackgroundWork>(BackgroundWork::WhenDestroyed::LetJobFinish);
        _decoder = std::make_shared<IsolatedDecoder>();
    }

    if(!_work)
        return;

    if(!_decoder)
        return;

    _work->ClearPending(); //only the newest selection is worth loading
    _lastPostAt = now;
    _work->Post([request = std::move(request), decoder = _decoder, stills = _stills](std::stop_token stop) mutable { return load(std::move(request), *decoder, stills.get(), stop); },
                [this](Loaded&& loaded) { showLoaded(std::move(loaded)); });
}

RetroFuturaGUI::FileExplorer::FilePreview::Loaded RetroFuturaGUI::FileExplorer::FilePreview::load(Request&& request, IsolatedDecoder& decoder, StillCache* stills, std::stop_token stop)
{
    const Entry& entry { request._Entry };
    Loaded loaded { ._Generation = request._Generation, ._Kind = request._Kind, ._Path = entry._Path, ._Picture = {}, ._Details = {}, ._Message = {}, ._CanShowFrame = false };
    const std::string path { toUtf8(entry._Path) };
    std::optional<MediaKind> mediaKind {};

    switch(request._Kind)
    {
        case Kind::Image: mediaKind = MediaKind::Image; break;
        case Kind::Audio: mediaKind = MediaKind::Audio; break;
        case Kind::Video: mediaKind = MediaKind::Video; break;
        default: break;
    }

    //What only this kind of file has comes first, since the pane shows as many rows as fit
    if(mediaKind)
    {
        MediaPreviewResult media { readMedia(*mediaKind, request, decoder, stills) };
        loaded._Details = std::move(media._Details);
        loaded._Picture = std::move(media._Picture);
        loaded._Message = std::move(media._Message);
        loaded._CanShowFrame = media._CanShowFrame;
    }

    if(Kind::Folder == request._Kind)
    {
        //Counted here, off the main thread, since a big folder or a slow drive takes a while
        uSize itemCount { 0 };
        std::error_code error {};

        for(std::filesystem::directory_iterator iterator { entry._Path, std::filesystem::directory_options::skip_permission_denied, error }, end {}; !error && iterator != end && !stop.stop_requested(); iterator.increment(error))
            ++itemCount;

        if(!error)
            loaded._Details.emplace_back("Items", std::to_string(itemCount));
    }

    for(std::pair<std::string, std::string>& detail : fileDetails(entry))
        loaded._Details.push_back(std::move(detail));

    return loaded;
}

void RetroFuturaGUI::FileExplorer::FilePreview::showLoaded(Loaded&& loaded)
{
    if(loaded._Generation != _generation)
        return; //something else is selected now

    if(!_picture)
        return;

    setDetails(loaded._Details);

    if(loaded._Picture)
    {
        _picture->SetImage(std::move(*loaded._Picture));
        showLayer(MediaLayer::Picture);
        return;
    }

    if(!loaded._Message.empty())
    {
        showMessage(loaded._Message);
        return;
    }

    showLayer(std::nullopt);
}

void RetroFuturaGUI::FileExplorer::FilePreview::showFrame(const std::filesystem::path& path)
{
    Video* video { frameWidget() };

    if(!video)
    {
        showMessage("Can't show videos here");
        return;
    }

    if(!video->OpenVideoFile(toUtf8(path)))
    {
        showMessage("Can't open this video");
        return;
    }

    //After a seek the first picture shows at once, even while paused
    video->Seek(video->GetDuration() / 10);
    showLayer(MediaLayer::Frame);
}

void RetroFuturaGUI::FileExplorer::FilePreview::showMessage(std::string_view message)
{
    if(!_message)
        return;

    _message->GetText().SetString(message, false);
    showLayer(MediaLayer::Message);
}

void RetroFuturaGUI::FileExplorer::FilePreview::showLayer(const std::optional<MediaLayer> layer)
{
    if(!_pane)
        return;

    for(uSize index = 0; index < _axis._LayerDefinition.size(); ++index)
    {
        LasagnaCell* cell { _pane->GetCell(TrackIndex { ._Row = 0, ._Column = 0, ._Layer = index }) };

        if(!cell)
            continue;

        cell->_Show = layer && static_cast<uSize>(*layer) == index; //a hidden cell's widget doesn't draw
    }
}

void RetroFuturaGUI::FileExplorer::FilePreview::setDetails(const std::vector<std::pair<std::string, std::string>>& details)
{
    if(!_details)
        return;

    const std::vector<Table::TrackDefinition>
        rowDefinition(details.size(), Table::TrackDefinition { ._Sizing = Table::TrackSizing::Fixed, ._Value = rowHeight(_font) }),
        columnDefinition
        {
            Table::TrackDefinition { ._Sizing = Table::TrackSizing::Fixed, ._Value = _font._Size * _keyColumnFontSizes },
            Table::TrackDefinition { ._Sizing = Table::TrackSizing::Star, ._Value = 1.0f }
        };
    _details->SetTrackDefinitions(rowDefinition, columnDefinition);
    _details->SetTrackReadOnly(0, true);
    _details->SetTrackReadOnly(1, true);

    for(uSize row = 0; row < details.size(); ++row)
    {
        _details->SetValue(details[row].first, TrackIndex { ._Row = row, ._Column = 0, ._Layer = 0 }, false);
        _details->SetValue(details[row].second, TrackIndex { ._Row = row, ._Column = 1, ._Layer = 0 }, false);
    }
}

void RetroFuturaGUI::FileExplorer::FilePreview::closeVideo()
{
    if(!_frame)
        return;

    _frame->Close();
}

RetroFuturaGUI::Video* RetroFuturaGUI::FileExplorer::FilePreview::frameWidget()
{
    if(!_pane)
        return nullptr;

    //Made the first time, since it opens an audio engine of its own
    if(!_frame)
    {
        const ViewCell frameCell { ._Row = 0, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = static_cast<uSize>(MediaLayer::Frame) };
        _frame = _host.Attach<Video>("PreviewFrame", *_pane, frameCell);
    }

    if(!_frame)
        return nullptr;

    _frame->SetSize(_mediaSize);
    return _frame;
}

RetroFuturaGUI::FileExplorer::FilePreview::Kind RetroFuturaGUI::FileExplorer::FilePreview::kindOf(const Entry& entry, const FileExplorerFlags flags)
{
    switch(fileKindOf(entry))
    {
        case FileKind::Folder:
            return Kind::Folder;
        case FileKind::Image:
            return HasFlag(flags, FileExplorerFlags::ImagePreview) ? Kind::Image : Kind::Other;
        case FileKind::Audio:
            return HasFlag(flags, FileExplorerFlags::MediaPreview) ? Kind::Audio : Kind::Other;
        case FileKind::Video:
            return HasFlag(flags, FileExplorerFlags::MediaPreview) ? Kind::Video : Kind::Other;
        default:
            return Kind::Other;
    }
}

std::vector<std::pair<std::string, std::string>> RetroFuturaGUI::FileExplorer::FilePreview::fileDetails(const Entry& entry)
{
    const bool isFolder { PlatformBridge::FileSystem::EntryKind::Directory == entry._Kind };
    LocalClock clock { ._Zone = getLocalZone(), ._Info = {}, ._HasInfo = false };
    std::vector<std::pair<std::string, std::string>> details { { "Name", toUtf8(entry._Path.filename()) } };

    if(!isFolder)
        details.emplace_back("Size", formatSize(entry._Size));

    details.emplace_back("Date", formatTime(entry._LastWriteTime, clock));
    details.emplace_back("Type", typeName(entry));
    return details;
}

RetroFuturaGUI::MediaPreviewResult RetroFuturaGUI::FileExplorer::FilePreview::readMedia(const MediaKind kind, const Request& request, IsolatedDecoder& decoder, StillCache* stills)
{
    const std::string path { toUtf8(request._Entry._Path) };

    if(!stills)
        return decoder.ReadPreview(kind, path, request._PictureSide);

    if(MediaKind::Video != kind)
        return decoder.ReadPreview(kind, path, request._PictureSide);

    //A still the thumbnails decoded already: only the details are read
    std::optional<DecodedImage> still { stills->Find(request._Entry._Path, request._Entry._LastWriteTime, request._PictureSide) };

    if(still)
    {
        MediaPreviewResult media { decoder.ReadPreviewDetails(kind, path, request._PictureSide) };
        media._Picture = std::move(still);
        media._CanShowFrame = true;
        return media;
    }

    MediaPreviewResult media { decoder.ReadPreview(kind, path, request._PictureSide) };

    if(media._Picture)
        stills->Keep(request._Entry._Path, request._Entry._LastWriteTime, *media._Picture);

    return media;
}

i32 RetroFuturaGUI::FileExplorer::FilePreview::getPictureSide() const
{
    return std::max(_minimumPictureSide, static_cast<i32>(std::max(_mediaSize.x, _mediaSize.y)));
}

void RetroFuturaGUI::FileExplorer::FilePreview::shareStillSide()
{
    if(!_stills)
        return;

    _stills->SetStillSide(getPictureSide());
}