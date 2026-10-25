#pragma once
#include "config.hpp"
#include <filesystem>
#include <optional>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The file operations every explorer in the app did, newest last, for Undo. Explorers run on the main thread, and so
    ///        does this
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
                _To {};
        };

        struct Step
        {
            Kind _Kind { Kind::NewFolder };
            std::vector<Item> _Items {};
        };

        FileUndoHistory() = delete;

        /// @brief Adds a step. past the limit the oldest one goes.
        static void Record(Step&& step);

        /// @brief Takes the newest step off to undo it.
        static std::optional<Step> TakeLast();

        static bool HasSteps();

    private:
        static constexpr uSize _stepLimit { 100 };
        static inline std::vector<Step> _steps {};
    };
}