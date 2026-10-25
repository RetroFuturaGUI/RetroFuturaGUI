#pragma once
#include "FileExplorer.hpp"
#include "Texture.hpp"
#include <array>
#include <chrono>
#include <deque>
#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The list's thumbnails, one for each row in view: an image scaled down, a video's frame from 10% in for royalty-free
    ///        codecs, a song's cover art, and a type icon for everything else and until a thumbnail is ready. Files are decoded off
    ///        the main thread and uploaded on it; a cloud file that isn't on this computer is never read and keeps its icon.
    ///        Once there are too many, the thumbnails seen longest ago are dropped. The rows in view keep theirs, and a row out of view
    ///        shows its icon, so no cell ever points at a thumbnail that went.
    class FileExplorer::ThumbnailCache
    {
    public:
        /// @brief Draws the type icons, which needs a GL context current.
        ThumbnailCache();
        ThumbnailCache(const ThumbnailCache&) = delete;
        ThumbnailCache(ThumbnailCache&&) = delete;
        auto operator =(const ThumbnailCache&) = delete;
        auto operator =(ThumbnailCache&&) = delete;
        ~ThumbnailCache() = default;

        /// @brief Gives every row of the column its type icon, after the list was filled.
        void ShowIcons(Table& table, const std::vector<Entry>& entries, const uSize column);

        /// @brief Runs each frame while thumbnails show: the rows in view get the thumbnails that are ready, the ones still missing
        ///        are asked for, and rows that left the view get their icons back.
        void Update(Table& table, const std::vector<Entry>& entries, const uSize column);

    private:
        /// @brief A file's thumbnail, or its being made. Without a texture, the file shows its icon.
        struct Thumbnail
        {
            std::unique_ptr<Texture> _Texture { nullptr };
            std::filesystem::file_time_type _WriteTime {}; //of the file the thumbnail was made from
            u64 _LastShownUpdate { 0 };
            bool _IsPending { false };
        };

        /// @brief The file a job on the worker thread decodes. The job keeps its own copy of the path and kind, so it never
        ///        reads the cache while the main thread changes the cache. It may destroy it before a stuck job ends.
        struct Request
        {
            std::filesystem::path _Path {};
            FileKind _Kind { FileKind::Other };
        };

        /// @brief What the worker hands back.
        struct Decoded
        {
            std::filesystem::path _Path {};
            std::optional<DecodedImage> _Image {}; //none when the file has no picture to show
        };

        /// @brief the worker decodes the file's picture and scales it down to a thumbnail.
        static Decoded decode(Request&& request, std::stop_token stop);

        /// @brief The main thread uploads a decoded picture as its file's thumbnail.
        void take(Decoded&& decoded);

        /// @brief The file's thumbnail, or its icon until the thumbnail is ready. A missing thumbnail is requested, unless the queue is full. then it's requested on a later update.
        const ITexture* pictureOf(const Entry& entry);
        const ITexture* iconOf(const FileKind kind) const;

        /// @brief Deletes the thumbnails shown longest ago while there are more than the limit. Rendered ones and the ones being made stay.
        void letGoOfOldest();

        /// @brief Gives up on a job that takes longer than _jobPatience
        void letGoOfStuckJob();

        std::unordered_map<std::filesystem::path::string_type, Thumbnail> _thumbnails {};
        std::deque<std::filesystem::path::string_type> _pendingKeys {}; //...in the order posted, which is the order they finish in
        std::array<std::unique_ptr<Texture>, 5> _icons {}; //by FileKind
        std::unique_ptr<BackgroundWork> _work { std::make_unique<BackgroundWork>(BackgroundWork::WhenDestroyed::LetJobFinish) }; //its jobs use only copies
        std::chrono::steady_clock::time_point _waitingSince {}; //for the job running now
        uSize
            _shownFirstRow { 0 },
            _shownEndRow { 0 }; //the rows in view at the last update, up to but without this one
        u64 _updateCount { 0 };

        static constexpr uSize
            _thumbnailLimit { 512 }, //64 x 64 pixels make 16 KB a thumbnail, so 8 MB at most
            _pendingLimit { 4 }; //few, so scrolling fast past rows doesn't leave a long queue of rows no longer in view
        static constexpr i32
            _pictureSide { 64 }, //the thumbnail column less the cell's padding
            _iconSide { 56 }; //the row less the cell's padding, so icons draw at the size they were drawn at
        static constexpr std::chrono::milliseconds _jobPatience { 5000 };
    };
}