#pragma once
#include "config.hpp"
#include <filesystem>
#include <optional>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The file operations every explorer in the app did, newest last, for Undo, and the steps undone since, for Redo.
    ///        Explorers run on the main thread, and so does this
    class FileUndoHistory
    {
    public:
        /// @brief What a step did decides how it is undone.
        enum class Kind : u32
        {
            NewFolder,
            Rename,
            Copy,
            Move,
            MoveToTrash
        };

        /// @brief One item of a step: where it was and where the step put it. NewFolder has only _To, MoveToTrash only _From.
        struct Item
        {
            std::filesystem::path
                _From {},
                _To {},
                _Replaced {};
        };

        struct Step
        {
            Kind _Kind { Kind::NewFolder };
            std::vector<Item> _Items {};
        };

        FileUndoHistory() = delete;

        /// @brief Adds a new step, past the limit the oldest one goes. The undone steps go, since Redo would no longer fit
        static void Record(Step&& step);

        /// @brief Takes the newest step off to undo it.
        static std::optional<Step> TakeLast();

        static bool HasSteps();

        /// @brief Keeps a step that was undone, for Redo
        static void KeepUndone(Step&& step);

        /// @brief Takes the step undone last off to redo it
        static std::optional<Step> TakeLastUndone();

        static bool HasUndoneSteps();

        /// @brief Adds a step that was redone, or one that couldn't be undone, back to the history, keeping the undone steps
        static void RecordAgain(Step&& step);

    private:
        static constexpr uSize _stepLimit { 100 };
        static inline std::vector<Step>
            _steps {},
            _undoneSteps {};
    };
}