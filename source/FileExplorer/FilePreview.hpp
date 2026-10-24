#pragma once
#include "FileExplorer.hpp"
#include "Texture.hpp"
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    class Image;
    class Video;

    /// @brief The explorer's preview pane beside the list: a picture - an image, a song's cover art, or a video's frame from
    ///        10% in, for royalty-free codecs only - over a row kept for playback controls, and the selection's details below.
    ///        Files are read and decoded off the main thread; what arrives shows from Update. The selected video stays open and
    ///        paused, the groundwork for playing it here later.
    class FileExplorer::FilePreview
    {
    public:
        /// @brief What the pane shows. A song and a video are kinds of their own, apart from their pictures, so a player can
        ///        take over from them later.
        enum class Kind : u32
        {
            None,   //nothing selected
            Image,
            Audio,
            Video,
            Folder,
            Several,
            Other   //a file without a picture: details only
        };

        /// @brief Builds the pane into a cell of the explorer's grid, the image's or the media's kinds only when their flags
        ///        are set.
        FilePreview(ViewHost& host, Lasagna& grid, const ViewCell& cell, const glm::vec2& windowSize, const ViewFont& font, const FileExplorerFlags flags);
        FilePreview() = delete;
        FilePreview(const FilePreview&) = delete;
        FilePreview(FilePreview&&) = delete;
        auto operator =(const FilePreview&) = delete;
        auto operator =(FilePreview&&) = delete;
        ~FilePreview() = default;

        /// @brief Returns whether every widget of the pane could be built.
        bool IsBuilt() const;

        /// @brief Shows the selected entries. The same file again, unchanged, keeps what shows. A cloud file that isn't on this
        ///        computer is downloaded to be previewed, which the pane says meanwhile.
        void Show(const std::vector<Entry>& selection);

        /// @brief Runs each frame from the explorer's update: shows what finished loading.
        void Update();

        Kind GetKind() const;

    private:
        /// @brief What the worker reads for one file, all copies, so the job never touches the pane.
        struct Request
        {
            u64 _Generation { 0 };
            Kind _Kind { Kind::None };
            Entry _Entry {};
            i32 _PictureSide { 0 }; //the longest side a picture is decoded to
        };

        /// @brief What the worker hands back.
        struct Loaded
        {
            u64 _Generation { 0 };
            Kind _Kind { Kind::None };
            std::filesystem::path _Path {};
            std::optional<DecodedImage> _Picture {};
            std::vector<std::pair<std::string, std::string>> _Details {};
            std::string _Message {}; //shown in the picture's place: no cover art, a codec that isn't royalty-free
            bool _CanShowFrame { false }; //a video whose picture may be decoded: opened on the main thread for its frame
        };

        /// @brief The media cell's layers; one shows at a time.
        enum class MediaLayer : u32
        {
            Picture,
            Frame,
            Message
        };

        /// @brief The worker's side: reads and decodes the file, and writes out its details.
        static Loaded load(Request&& request, std::stop_token stop);

        /// @brief The main thread's side: shows what a job loaded, unless something else was selected since.
        void showLoaded(Loaded&& loaded);

        /// @brief Opens a video for its frame from 10% in, paused; on the main thread, since the frame's texture is GL's.
        void showFrame(const std::filesystem::path& path);
        void showMessage(std::string_view message);

        /// @brief Shows one of the media cell's layers and hides the others; none hides them all.
        void showLayer(const std::optional<MediaLayer> layer);
        void setDetails(const std::vector<std::pair<std::string, std::string>>& details);

        /// @brief Lets go of the open video, so nothing holds the file.
        void closeVideo();

        /// @brief The frame's widget, made the first time a video shows, since it opens an audio engine of its own.
        Video* frameWidget();

        /// @brief Has the worker load the request; a job stuck on a slow download is let go after a while.
        void post(Request&& request);

        static Kind kindOf(const Entry& entry, const FileExplorerFlags flags);

        /// @brief Name, size, modified and type, as every file has them.
        static std::vector<std::pair<std::string, std::string>> fileDetails(const Entry& entry);

        /// @brief "3:25", or "1:02:03" past an hour.
        static std::string formatDuration(const f64 seconds);

        /// @brief "H.264" for FFmpeg's "h264": the names the codecs that aren't royalty-free are known by.
        static std::string codecName(std::string_view ffmpegName);

        ViewHost& _host;
        Lasagna
            * _pane { nullptr },
            * _media { nullptr };
        Image* _picture { nullptr };
        Video* _frame { nullptr };
        Label* _message { nullptr };
        Table* _details { nullptr };
        ViewFont _font {};
        glm::vec3 _mediaSize { 0.0f };
        FileExplorerFlags _flags { FileExplorerFlags::SingleFile };
        Kind _kind { Kind::None };
        std::filesystem::path _shownPath {};
        std::filesystem::file_time_type _shownWriteTime {};
        uSize _shownCount { 0 };
        std::unique_ptr<BackgroundWork> _work { std::make_unique<BackgroundWork>(BackgroundWork::WhenDestroyed::LetJobFinish) }; //its jobs use only copies
        std::chrono::steady_clock::time_point _lastPostAt {};
        u64 _generation { 0 };

        static constexpr std::chrono::milliseconds _jobPatience { 2000 }; //a job busy longer is stuck, a download say, and let go
        static constexpr f32 _keyColumnFontSizes { 4.5f }; //the key column's width: "Channels", the longest key, fits
        static constexpr i32 _minimumPictureSide { 512 };
        inline static const AxisDefinition _axis
        {
            ._RowDefinition = { 0.47f, 0.08f, 0.45f }, //picture, the playback controls' row, details
            ._ColumnDefinition = { 1.0f },
            ._LayerDefinition = { 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f } //the picture, the video's frame, a message
        };
    };
}