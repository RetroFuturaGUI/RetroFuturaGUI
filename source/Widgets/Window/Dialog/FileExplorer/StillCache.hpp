#pragma once
#include "FileExplorer.hpp"
#include "Texture.hpp"
#include <atomic>
#include <filesystem>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace RetroFuturaGUI
{
    /// @brief The video stills the preview pane and the thumbnails share, so a video's frame is decoded once for both: at the
    ///        pane's size, which the thumbnails scale down. Each still belongs to the file as it was written when decoded. Used
    ///        from both workers' threads
    class FileExplorer::StillCache
    {
    public:
        StillCache() = default;
        StillCache(const StillCache&) = delete;
        StillCache(StillCache&&) = delete;
        ~StillCache() = default;
        auto operator =(const StillCache&) = delete;
        auto operator =(StillCache&&) = delete;

        /// @brief Returns a copy of the file's still when it was decoded from the file as written now and is at least minSide
        ///        large, or as large as the video itself is
        std::optional<DecodedImage> Find(const std::filesystem::path& path, const std::filesystem::file_time_type writeTime, const i32 minSide);

        /// @brief Keeps a file's still, in place of the one kept before. The ones used longest ago go beyond the limit
        void Keep(const std::filesystem::path& path, const std::filesystem::file_time_type writeTime, DecodedImage still);

        /// @brief Sets the size stills are decoded to: the preview pane's picture
        void SetStillSide(const i32 side);
        i32 GetStillSide() const;

    private:
        struct Still
        {
            std::filesystem::file_time_type _WriteTime {};
            DecodedImage _Image {};
            u64 _LastUsed { 0 };
        };

        /// @brief Deletes the stills used longest ago while there are more than the limit.
        void letGoOfOldest();

        std::mutex _mutex;
        std::unordered_map<std::filesystem::path::string_type, Still> _stills {};
        u64 _useCount { 0 };
        std::atomic<i32> _stillSide { 256 }; //until the pane says its size.
        static constexpr uSize _stillLimit { 16 }; //a few hundred KB each at the pane's size.
    };
}