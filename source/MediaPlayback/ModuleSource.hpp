#pragma once
#include "AudioMetadata.hpp"
#include "config.hpp"
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <miniaudio.h>

struct openmpt_module;

//Set by CMake's RETROFUTURAGUI_TRACKER_MODULES option
#ifndef RETROFUTURAGUI_TRACKER_MODULES
#define RETROFUTURAGUI_TRACKER_MODULES 0
#endif

namespace RetroFuturaGUI
{
    /// @brief Whether this build plays tracker modules (CMake's RETROFUTURAGUI_TRACKER_MODULES is on by default).
    //         Off: libopenmpt isn't linked: nothing reads as a module and modules don't open.
    inline constexpr bool TrackerModulesEnabled { 0 != RETROFUTURAGUI_TRACKER_MODULES };

    /// @brief A tracker module (MOD, XM, S3M, IT, MPTM, MO3 and about 60 more formats) as a miniaudio data source,
    ///        rendered by libopenmpt to stereo float at the rate it's opened with. miniaudio's seek, cursor, length and
    ///        looping work on it as on a file it decodes itself; looping repeats the module from its own restart position.
    /// @note ma_data_source_base must stay the first member - miniaudio casts between the two.
    class ModuleSource
    {
    public:
        ModuleSource() = default;
        ModuleSource(const ModuleSource&) = delete;
        ModuleSource(ModuleSource&&) = delete;
        ~ModuleSource();
        ModuleSource& operator=(const ModuleSource&) = delete;
        ModuleSource& operator=(ModuleSource&&) = delete;

        /// @brief Whether libopenmpt recognises the start of the file as a module or a module container (UMX, ...), read
        ///        from its first few kilobytes. Always false without tracker modules.
        static bool IsModule(std::string_view path);

        /// @brief Whether libopenmpt knows the extension ("xm", ".IT", ...), for listing files by name - IsModule decides
        ///        by content. Always false without tracker modules.
        static bool IsModuleExtension(std::string_view extension);

        /// @brief What a module says about itself, without playing it: AudioFileFormat::Module with ModuleMetadata. The
        ///        common part has the title, artist, date, message (as the comment), the first subsong's length, and every
        ///        metadata item as a tag; it plays as stereo at any rate, so _SampleRate stays 0.
        static std::optional<AudioMetadata> ReadMetadata(std::string_view path);

        /// @brief Loads the whole file and renders it at sampleRate. The first subsong is selected, played once.
        bool Open(std::string_view path, const u32 sampleRate);
        void Close();
        bool IsOpen() const;

        /// @brief What ma_sound_init_from_data_source takes.
        ma_data_source* Get();

        /// @brief How many subsongs the module has - most have one, game music often several.
        uSize GetSubsongCount() const;

        /// @brief Switches to a subsong and plays it from its start; -1 plays all of them one after another.
        bool SelectSubsong(const i32 index);

    private:
        static ma_result onRead(ma_data_source* dataSource, void* framesOut, ma_uint64 frameCount, ma_uint64* framesRead);
        static ma_result onSeek(ma_data_source* dataSource, ma_uint64 frameIndex);
        static ma_result onGetDataFormat(ma_data_source* dataSource, ma_format* format, ma_uint32* channels, ma_uint32* sampleRate, ma_channel* channelMap, size_t channelMapCap);
        static ma_result onGetCursor(ma_data_source* dataSource, ma_uint64* cursor);
        static ma_result onGetLength(ma_data_source* dataSource, ma_uint64* length);
        static ma_result onSetLooping(ma_data_source* dataSource, ma_bool32 isLooping);

        /// @brief The whole file, or its first maxSize bytes; fileSize gets the full size. Empty when it can't be read.
        static std::vector<u8> readFile(std::string_view path, const u64 maxSize, u64& fileSize);

        /// @brief The module of the file in memory, or null. libopenmpt's messages go nowhere.
        static openmpt_module* createModule(const std::vector<u8>& data);

        /// @brief A metadata item, a subsong, instrument or sample name, taken over from the string libopenmpt hands out.
        static std::string takeString(const char* text);

        /// @brief libopenmpt's extensions in lowercase, without dots. libopenmpt builds its list anew on every question, which a folder
        ///        of thousands of files feels, so IsModuleExtension reads it once
        static std::set<std::string, std::less<>> readModuleExtensions();

        /// @brief The ASCII letters in lowercase, every other character as it is. How libopenmpt compares extensions
        static std::string lowerCase(std::string_view text);

        /// @brief Measures the selected subsong: its length, and the time of the order and row it repeats from (seeking
        ///        there once, then back to the start). Only with the lock held.
        void measureSong();

        /// @brief Where the song is, in seconds. libopenmpt keeps counting when the song repeats, so while repeating, past
        ///        the end this goes back to the restart position instead, as the music does. Only with the lock held.
        f64 getSongPosition() const;

        template<typename T> static void setIfRequested(T* out, const T value)
        {
            if(!out)
                return;

            *out = value;
        }

        ma_data_source_base _base {};
        openmpt_module* _module { nullptr };
        mutable std::mutex _mutex {}; //libopenmpt isn't thread-safe: the audio thread renders while the windows' thread seeks and asks
        u32 _sampleRate { 0 };
        f64
            _songSeconds { 0.0 }, //one pass through the selected subsong; 0 for an endless one
            _restartSeconds { 0.0 }, //where a repeat goes back to
            _positionOffset { 0.0 }; //the repeats libopenmpt counted before repeating was switched off
        bool _isRepeating { false };
    };
}