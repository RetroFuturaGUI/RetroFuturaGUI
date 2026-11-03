#pragma once
#include "config.hpp"
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

//What the fuzz targets that read files share: the input written to a file of its own, for readers that open paths
namespace RetroFuturaGUIFuzz
{
    /// @brief A temporary file holding one input, removed again when it goes. One per process, reused for every input.
    class InputFile
    {
    public:
        InputFile(const std::uint8_t* data, const std::size_t size, const char* extension)
        {
            _path = std::filesystem::temp_directory_path() / (std::string("RetroFuturaGUIFuzz-") + std::to_string(processId()) + extension);
            std::ofstream file { _path, std::ios::binary | std::ios::trunc };
            file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
        }

        InputFile(const InputFile&) = delete;
        InputFile& operator=(const InputFile&) = delete;

        ~InputFile()
        {
            std::error_code ignored {};
            std::filesystem::remove(_path, ignored);
        }

        /// @brief UTF-8, as the readers take it.
        std::string GetPath() const
        {
            const std::u8string text { _path.u8string() };
            return std::string(text.begin(), text.end());
        }

    private:
        static unsigned long processId();

        std::filesystem::path _path {};
    };
}

#if defined(TARGET_PLATFORM_WINDOWS)
#include <process.h>
inline unsigned long RetroFuturaGUIFuzz::InputFile::processId() { return static_cast<unsigned long>(_getpid()); }
#else
#include <unistd.h>
inline unsigned long RetroFuturaGUIFuzz::InputFile::processId() { return static_cast<unsigned long>(getpid()); }
#endif