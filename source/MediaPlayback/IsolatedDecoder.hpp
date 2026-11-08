#pragma once
#include "ChildProcess.hpp"
#include "MediaPreview.hpp"
#include "MediaPreviewProtocol.hpp"
#include "config.hpp"
#include <chrono>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief Has MediaPreview's work done in a process of its own. The decoder helper (RetroFuturaGUIDecoder) is shipped beside
    ///        the program or library RetroFuturaGUI. A file that crashes a decoder or even takes it over, takes only
    ///        the helper down. That file's preview says it can't be read, and the next request starts a new helper. Without
    ///        the helper beside it, the work is done in this process
    /// @note Not thread-safe: one per worker thread
    class IsolatedDecoder
    {
    public:
        IsolatedDecoder() = default;
        IsolatedDecoder(const IsolatedDecoder&) = delete;
        IsolatedDecoder(IsolatedDecoder&&) = delete;
        IsolatedDecoder& operator=(const IsolatedDecoder&) = delete;
        IsolatedDecoder& operator=(IsolatedDecoder&&) = delete;
        ~IsolatedDecoder() = default;

        /// @brief MediaPreview::ReadPreview in the helper
        /// @param path UTF-8.
        MediaPreviewResult ReadPreview(const MediaKind kind, std::string_view path, const i32 pictureSide);

        /// @brief MediaPreview::ReadPreviewDetails in the helper
        MediaPreviewResult ReadPreviewDetails(const MediaKind kind, std::string_view path, const i32 pictureSide);

        /// @brief MediaPreview::ReadThumbnail in the helper
        std::optional<DecodedImage> ReadThumbnail(const MediaKind kind, std::string_view path, const i32 pictureSide);

        /// @brief Where the helper is looked for
        static std::filesystem::path GetHelperPath();

        /// @brief Whether the helper is there, so the work runs apart from this process
        static bool IsHelperAvailable();

    private:
        /// @brief Sends a request and returns the reply, starting the helper when it isn't running. Nothing when it can't be
        ///        started ends while working, sends something malformed, or takes longer than _replyPatience - it's stopped then
        std::optional<MediaPreviewResult> exchange(const MediaRequest& request);

        PlatformBridge::ChildProcess _helper {};

        static constexpr std::chrono::milliseconds _replyPatience { 30000 }; //a cloud file is downloaded first
    };
}
