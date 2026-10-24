#include "FilePreview.hpp"
#include "AudioMetadata.hpp"
#include "Image.hpp"
#include "Label.hpp"
#include "Table.hpp"
#include "Video.hpp"
#include "VideoMetadata.hpp"
#include "Window.hpp"
#include <algorithm>
#include <array>
#include <format>

RetroFuturaGUI::FileExplorer::FilePreview::FilePreview(ViewHost& host, Lasagna& grid, const ViewCell& cell, const glm::vec2& windowSize, const ViewFont& font, const FileExplorerFlags flags)
    : _host(host), _font(font), _flags(flags)
{
    //The pane fills its cell, and its own cells are measured from that
    const glm::vec3 fixedSize { cellSize(_viewAxis, windowSize, cell) };
    const glm::vec2 paneSize { fixedSize.x + 2.0f * _margin, fixedSize.y + 2.0f * _margin };
    _pane = host.Attach<Lasagna>("PreviewPane", grid, cell, _axis);

    if(!_pane)
        return;

    const ViewCell
        pictureCell { ._Row = 0, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = static_cast<uSize>(MediaLayer::Picture) },
        messageCell { ._Row = 0, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = static_cast<uSize>(MediaLayer::Message) },
        detailsCell { ._Row = 2, ._Column = 0, ._ColumnSpan = 1, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
    _mediaSize = cellSize(_axis, paneSize, pictureCell);
    _picture = host.Attach<Image>("PreviewPicture", *_pane, pictureCell);
    _message = host.Attach<Label>("PreviewMessage", *_pane, messageCell);
    _details = host.Attach<Table>("PreviewDetails", *_pane, detailsCell);

    if(!_picture || !_message || !_details)
        return;

    _picture->SetSize(_mediaSize);
    styleLabel(*_message, "", _mediaSize, _font);
    _message->SetTextAlignment(TextAlignment::Center);
    _message->SetTextColor(glm::vec4(0.7f, 0.7f, 0.75f, 1.0f), ColorState::Enabled);

    //Like the list, without its header
    const glm::vec3 detailsSize { cellSize(_axis, paneSize, detailsCell) };
    _details->SetSize(detailsSize);
    _details->SetBorderWidth(1.0f);
    _details->SetBorderColor(glm::vec4(0.35f, 0.35f, 0.45f, 1.0f), ColorState::Enabled);
    _details->SetTrackAlternatingColorCount(2);
    std::vector<glm::vec4>
        evenRowColors {{ 0.1f, 0.1f, 0.13f, 1.0f }},
        oddRowColors {{ 0.13f, 0.13f, 0.17f, 1.0f }},
        innerBorderColors {{ 0.18f, 0.18f, 0.22f, 1.0f }};
    _details->SetTrackBackgroundColors(evenRowColors, ColorState::Enabled, 0);
    _details->SetTrackBackgroundColors(oddRowColors, ColorState::Enabled, 1);
    _details->SetTrackBorderColors(innerBorderColors, ColorState::Enabled, 0);
    _details->SetTrackBorderColors(innerBorderColors, ColorState::Enabled, 1);
    _details->SetInnerBorderWidth(1.0f);
    _details->SetFontFamily(_font._Family, _font._Size, _font._Slant, _font._Weight);
    _details->SetTextAlignment(TextAlignment::Left);
    _details->SetTextPadding(6.0f);
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

    //Reading a cloud file that isn't on this computer downloads all of it first
    if(entry._IsOnlineOnly && Kind::Folder != _kind)
        showMessage("Downloading\nto preview...");

    const i32 pictureSide { (std::max)(_minimumPictureSide, static_cast<i32>((std::max)(_mediaSize.x, _mediaSize.y))) };
    post(Request { ._Generation = _generation, ._Kind = _kind, ._Entry = entry, ._PictureSide = pictureSide });
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

    //A job still running holds the next one up; one busy this long is stuck, on a download say, and is let go to finish alone
    if(!_work || (_work->IsBusy() && _jobPatience < now - _lastPostAt))
        _work = std::make_unique<BackgroundWork>(BackgroundWork::WhenDestroyed::LetJobFinish);

    if(!_work)
        return;

    _work->ClearPending(); //only the newest selection is worth loading
    _lastPostAt = now;
    _work->Post([request = std::move(request)](std::stop_token stop) mutable { return load(std::move(request), stop); },
                [this](Loaded&& loaded) { showLoaded(std::move(loaded)); });
}

RetroFuturaGUI::FileExplorer::FilePreview::Loaded RetroFuturaGUI::FileExplorer::FilePreview::load(Request&& request, std::stop_token stop)
{
    const Entry& entry { request._Entry };
    Loaded loaded { ._Generation = request._Generation, ._Kind = request._Kind, ._Path = entry._Path, ._Picture = {}, ._Details = {}, ._Message = {}, ._CanShowFrame = false };
    const std::string path { toUtf8(entry._Path) };
    const auto add { [&loaded](std::string_view key, std::string value)
        {
            if(value.empty())
                return;

            loaded._Details.emplace_back(std::string(key), std::move(value));
        } };

    //What only this kind of file has comes first, since the pane shows as many rows as fit
    switch(request._Kind)
    {
        case Kind::Image:
            loaded._Picture = Texture::DecodeFile(entry._Path, true, request._PictureSide);

            if(!loaded._Picture)
            {
                loaded._Message = "Can't read this image";
                break;
            }

            add("Pixels", std::format("{} x {}", loaded._Picture->_SourceResolution.x, loaded._Picture->_SourceResolution.y));
        break;
        case Kind::Audio:
        {
            const std::optional<AudioMetadata> audio { ReadAudioMetadata(path) };

            if(!audio)
            {
                loaded._Message = "Can't read this file";
                break;
            }

            add("Title", audio->_Title);
            add("Artist", audio->_Artist);
            add("Album", audio->_Album);
            add("Length", 0.0 < audio->_Seconds ? formatDuration(audio->_Seconds) : std::string {});
            add("Year", audio->_Date);
            add("Genre", audio->_Genre);
            add("Track", audio->_Track);
            add("Format", AudioFileFormat::Unknown == audio->_Format ? codecName(audio->_Codec) : std::string(GetAudioFileFormatName(audio->_Format)));
            add("Rate", 0 < audio->_SampleRate ? std::format("{:g} kHz", audio->_SampleRate / 1000.0) : std::string {});
            add("Channels", 0 < audio->_Channels ? std::to_string(audio->_Channels) : std::string {});
            add("Bit rate", 0 < audio->_BitRate ? std::format("{} kbps", (audio->_BitRate + 500) / 1000) : std::string {});

            if(!audio->_CoverArt.empty())
                loaded._Picture = Texture::DecodeMemory(audio->_CoverArt, true, request._PictureSide);

            if(!loaded._Picture)
                loaded._Message = "No cover art";
        }
        break;
        case Kind::Video:
        {
            const std::optional<VideoMetadata> video { ReadVideoMetadata(path) };

            if(!video)
            {
                loaded._Message = "Can't read this video";
                break;
            }

            add("Title", video->_Title);
            add("Length", 0.0 < video->_Seconds ? formatDuration(video->_Seconds) : std::string {});
            add("Pixels", std::format("{} x {}", video->_Width, video->_Height));
            add("Frames", 0.0 < video->_FramesPerSecond ? std::format("{:g} fps", std::round(video->_FramesPerSecond * 1000.0) / 1000.0) : std::string {});
            add("Video", codecName(video->_VideoCodec));
            add("Audio", video->_HasAudio ? std::format("{}, {:g} kHz, {} channels", codecName(video->_AudioCodec), video->_SampleRate / 1000.0, video->_Channels) : std::string("None"));
            add("Format", video->_Container);

            //The same check decides the frame now and playing later
            if(video->_VideoCodecIsFree)
                loaded._CanShowFrame = true;
            else
                loaded._Message = std::format("No preview:\n{} isn't royalty-free", codecName(video->_VideoCodec)); //two lines, to fit the pane
        }
        break;
        case Kind::Folder:
        {
            //Counted here, off the main thread, since a big folder or a slow drive takes a while
            uSize itemCount { 0 };
            std::error_code error {};

            for(std::filesystem::directory_iterator iterator { entry._Path, std::filesystem::directory_options::skip_permission_denied, error }, end {}; !error && iterator != end && !stop.stop_requested(); iterator.increment(error))
                ++itemCount;

            add("Items", error ? std::string {} : std::to_string(itemCount));
        }
        break;
        default:
        break;
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

    if(loaded._CanShowFrame)
    {
        showFrame(loaded._Path);
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

    _message->SetText(message, false);
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
    LocalClock clock { ._Zone = localZone(), ._Info = {}, ._HasInfo = false };
    std::vector<std::pair<std::string, std::string>> details { { "Name", toUtf8(entry._Path.filename()) } };

    if(!isFolder)
        details.emplace_back("Size", formatSize(entry._Size));

    details.emplace_back("Date", formatTime(entry._LastWriteTime, clock));
    details.emplace_back("Type", typeName(entry));
    return details;
}

std::string RetroFuturaGUI::FileExplorer::FilePreview::formatDuration(const f64 seconds)
{
    const i64
        totalSeconds { static_cast<i64>(std::llround(seconds)) },
        hours { totalSeconds / 3600 },
        minutes { totalSeconds / 60 % 60 },
        rest { totalSeconds % 60 };

    if(0 < hours)
        return std::format("{}:{:02}:{:02}", hours, minutes, rest);

    return std::format("{}:{:02}", minutes, rest);
}

std::string RetroFuturaGUI::FileExplorer::FilePreview::codecName(std::string_view ffmpegName)
{
    constexpr std::array<std::pair<std::string_view, std::string_view>, 20> names
    {{
        { "h264", "H.264" }, { "hevc", "HEVC" }, { "vvc", "VVC" }, { "vc1", "VC-1" }, { "mpeg4", "MPEG-4 Part 2" },
        { "mpeg2video", "MPEG-2" }, { "mpeg1video", "MPEG-1" }, { "vp8", "VP8" }, { "vp9", "VP9" }, { "av1", "AV1" },
        { "theora", "Theora" }, { "ffv1", "FFV1" }, { "mjpeg", "Motion JPEG" }, { "aac", "AAC" }, { "mp3", "MP3" },
        { "mp2", "MP2" }, { "opus", "Opus" }, { "vorbis", "Vorbis" }, { "flac", "FLAC" }, { "pcm_s16le", "PCM" }
    }};
    const auto known { std::find_if(names.begin(), names.end(), [ffmpegName](const auto& name) { return name.first == ffmpegName; }) };
    return names.end() != known ? std::string(known->second) : std::string(ffmpegName);
}