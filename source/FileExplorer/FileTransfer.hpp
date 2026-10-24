#pragma once
#include "config.hpp"
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <optional>
#include <stop_token>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief Copies, moves and deletes files on a thread of its own, one operation at a time, and keeps what the main thread
    ///        shows of it: how far it got, a name clash waiting for an answer, and how it ended. Poll it once per frame. Stop, and
    ///        destroying it, end the operation after the chunk being copied; a file copied halfway is removed.
    class FileTransfer
    {
    public:
        enum class Operation : u32
        {
            Copy,
            Move,
            MoveToTrash,
            DeletePermanently,
            RestoreFromTrash //the sources are where the items were before they went to the bin
        };

        /// @brief What a copy or a move does when an item's name is taken where it goes.
        enum class ClashRule : u32
        {
            KeepBoth, //the one coming in gets a free name, "Song (2).mp3"
            Ask       //the worker waits for AnswerClash
        };

        /// @brief What to do with an item whose name is taken where it goes.
        enum class ClashAnswer : u32
        {
            Replace,
            Skip,
            KeepBoth
        };

        /// @brief Whether a clash answer is for this item alone or for every clash left in the operation.
        enum class ClashScope : u32
        {
            ThisItem,
            RemainingItems
        };

        struct Request
        {
            Operation _Operation { Operation::Copy };
            std::vector<std::filesystem::path>
                _Sources {},
                _Targets {}; //Copy and Move: each source's own path to go to, moving items back where they were; empty uses _Destination
            std::filesystem::path _Destination {}; //the folder Copy and Move put the items in
            ClashRule _ClashRule { ClashRule::KeepBoth };
        };

        /// @brief How far the operation got: "Copying 3 of 12: Song.mp3".
        struct Progress
        {
            Operation _Operation { Operation::Copy };
            uSize
                _DoneCount { 0 },  //of the items asked for, not the files inside a folder
                _TotalCount { 0 };
            std::filesystem::path _Current {}; //the item being worked on
        };

        /// @brief A name taken where an item goes, waiting for AnswerClash.
        struct Clash
        {
            std::filesystem::path
                _Source {},
                _Target {};
        };

        /// @brief An item done: where it was, and where it is now. _To is empty once it is deleted or in the recycle bin.
        struct DoneItem
        {
            std::filesystem::path
                _From {},
                _To {};
        };

        /// @brief How an operation ended.
        struct Outcome
        {
            Operation _Operation { Operation::Copy };
            uSize _TotalCount { 0 };
            std::vector<DoneItem> _Done {};
            std::vector<std::filesystem::path>
                _NotTrashed {}, //MoveToTrash: items the recycle bin can't take, left where they are
                _Skipped {};    //clashes answered with Skip
            std::filesystem::path _FailedPath {}; //the first item that failed; empty when none did
            std::error_code _Error {};            //why it failed
            uSize _FailedCount { 0 };
            bool _IsStopped { false };
        };

        FileTransfer() = default;
        FileTransfer(const FileTransfer&) = delete;
        FileTransfer(FileTransfer&&) = delete;
        auto operator =(const FileTransfer&) = delete;
        auto operator =(FileTransfer&&) = delete;

        /// @brief Stops the operation after the current chunk and waits for it.
        ~FileTransfer();

        /// @brief Starts an operation on the worker thread.
        /// @return false while another one runs or its outcome hasn't been taken
        bool Start(Request&& request);

        /// @brief Returns whether an operation runs, or its outcome waits for TakeOutcome.
        bool IsBusy() const;

        /// @brief Ends the operation after the chunk being copied, removing the file copied halfway. A clash waiting for an
        ///        answer ends it at once. The outcome still arrives, with _IsStopped set.
        void Stop();

        Progress GetProgress() const;

        /// @brief The clash the worker waits on, if any.
        std::optional<Clash> GetClash() const;

        /// @brief Answers the clash the worker waits on; nothing when none waits.
        void AnswerClash(const ClashAnswer answer, const ClashScope scope);

        /// @brief Takes the outcome of the operation once it has ended, which makes the transfer free for the next one.
        std::optional<Outcome> TakeOutcome();

        /// @brief Returns whether path is folder or lies inside it, by the file system's identity of each folder above path.
        static bool IsInside(const std::filesystem::path& path, const std::filesystem::path& folder);

    private:
        /// @brief The worker's side: runs the request item by item, then leaves the outcome.
        void run(Request&& request, std::stop_token stop);

        /// @brief Copies or moves one item, the request's item at index, asking about a clash first when the rule says so.
        void transferItem(const Request& request, const uSize index, Outcome& outcome, std::optional<ClashAnswer>& standingAnswer, std::stop_token stop);

        /// @brief Waits for the main thread to answer a clash.
        /// @return The answer, or nothing when the operation was stopped meanwhile.
        std::optional<std::pair<ClashAnswer, ClashScope>> askAboutClash(const std::filesystem::path& source, const std::filesystem::path& target, std::stop_token stop);

        void setProgress(const uSize doneCount, const std::filesystem::path& current);

        /// @brief Counts a failed item; the first one's path and error are kept, for the message.
        static void noteFailure(Outcome& outcome, const std::filesystem::path& path, const std::error_code& error);

        /// @brief Copies a file or a folder with everything in it to target, which must not exist.
        /// @return false when stopped or failed; the file being copied is removed, what was copied before stays
        static bool copyTree(const std::filesystem::path& source, const std::filesystem::path& target, std::stop_token stop, std::error_code& errorOut);

        /// @brief Copies one file in chunks, checking for a stop after each, and gives the copy the original's write time.
        static bool copyFile(const std::filesystem::path& source, const std::filesystem::path& target, std::stop_token stop, std::error_code& errorOut);

        /// @brief Deletes a file or a folder with everything in it, checking for a stop between entries.
        static bool deleteTree(const std::filesystem::path& path, std::stop_token stop, std::error_code& errorOut);

        /// @brief Moves a file or a folder to target, which must not exist: renamed on the same volume, otherwise copied and the
        ///        original deleted once the copy is whole.
        static bool moveTree(const std::filesystem::path& source, const std::filesystem::path& target, std::stop_token stop, std::error_code& errorOut);

        mutable std::mutex _mutex {};
        std::condition_variable_any _clashAnswered {};
        Progress _progress {};
        std::optional<Clash> _clash {};
        std::optional<std::pair<ClashAnswer, ClashScope>> _clashAnswer {};
        std::optional<Outcome> _outcome {};
        bool _isBusy { false };

        static constexpr uSize _chunkSize { 1024 * 1024 }; //what a copy reads and writes at once; a stop takes effect after it

        std::jthread _worker {}; //last, so it is stopped and joined before the state above goes
    };
}