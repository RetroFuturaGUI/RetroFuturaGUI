#include "ModuleSource.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <type_traits>

static_assert(std::is_standard_layout_v<RetroFuturaGUI::ModuleSource>);

#if RETROFUTURAGUI_TRACKER_MODULES

#include <libopenmpt/libopenmpt.h>

RetroFuturaGUI::ModuleSource::~ModuleSource()
{
    Close();
}

std::vector<u8> RetroFuturaGUI::ModuleSource::readFile(std::string_view path, const u64 maxSize, u64& fileSize)
{
    std::ifstream file { std::filesystem::path(std::u8string(path.begin(), path.end())), std::ios::binary | std::ios::ate };

    if(!file)
        return {};

    const std::streamoff size { file.tellg() };

    if(0 >= size)
        return {};

    fileSize = static_cast<u64>(size);
    std::vector<u8> data(static_cast<uSize>(std::min(fileSize, maxSize)));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));

    if(!file)
        return {};

    return data;
}

openmpt_module* RetroFuturaGUI::ModuleSource::createModule(const std::vector<u8>& data)
{
    if(data.empty())
        return nullptr;

    return openmpt_module_create_from_memory2(data.data(), data.size(), &openmpt_log_func_silent, nullptr, &openmpt_error_func_ignore, nullptr, nullptr, nullptr, nullptr);
}

std::string RetroFuturaGUI::ModuleSource::takeString(const char* text)
{
    if(!text)
        return {};

    std::string taken { text };
    openmpt_free_string(text);
    return taken;
}

bool RetroFuturaGUI::ModuleSource::IsModule(std::string_view path)
{
    u64 fileSize { 0 };
    const std::vector<u8> start { readFile(path, openmpt_probe_file_header_get_recommended_size(), fileSize) };

    if(start.empty())
        return false;

    return OPENMPT_PROBE_FILE_HEADER_RESULT_SUCCESS == openmpt_probe_file_header(OPENMPT_PROBE_FILE_HEADER_FLAGS_DEFAULT, start.data(), start.size(), fileSize,
        &openmpt_log_func_silent, nullptr, &openmpt_error_func_ignore, nullptr, nullptr, nullptr);
}

std::optional<RetroFuturaGUI::AudioMetadata> RetroFuturaGUI::ModuleSource::ReadMetadata(std::string_view path)
{
    if(!IsModule(path))
        return std::nullopt;

    u64 fileSize { 0 };
    openmpt_module* module { createModule(readFile(path, std::numeric_limits<u64>::max(), fileSize)) };

    if(!module)
        return std::nullopt;

    openmpt_module_select_subsong(module, 0);

    ModuleMetadata details;
    details._Type = takeString(openmpt_module_get_metadata(module, "type"));
    details._TypeName = takeString(openmpt_module_get_metadata(module, "type_long"));
    details._OriginalType = takeString(openmpt_module_get_metadata(module, "originaltype"));
    details._Container = takeString(openmpt_module_get_metadata(module, "container_long"));
    details._Tracker = takeString(openmpt_module_get_metadata(module, "tracker"));
    details._Message = takeString(openmpt_module_get_metadata(module, "message_raw"));
    details._Warnings = takeString(openmpt_module_get_metadata(module, "warnings"));
    details._ChannelCount = static_cast<uSize>(std::max(openmpt_module_get_num_channels(module), 0));
    details._OrderCount = static_cast<uSize>(std::max(openmpt_module_get_num_orders(module), 0));
    details._PatternCount = static_cast<uSize>(std::max(openmpt_module_get_num_patterns(module), 0));
    details._InstrumentCount = static_cast<uSize>(std::max(openmpt_module_get_num_instruments(module), 0));
    details._SampleCount = static_cast<uSize>(std::max(openmpt_module_get_num_samples(module), 0));

    for(i32 index = 0; index < openmpt_module_get_num_subsongs(module); ++index)
        details._SubsongNames.push_back(takeString(openmpt_module_get_subsong_name(module, index)));

    for(i32 index = 0; index < static_cast<i32>(details._InstrumentCount); ++index)
        details._InstrumentNames.push_back(takeString(openmpt_module_get_instrument_name(module, index)));

    for(i32 index = 0; index < static_cast<i32>(details._SampleCount); ++index)
        details._SampleNames.push_back(takeString(openmpt_module_get_sample_name(module, index)));

    AudioMetadata metadata;
    metadata._Format = AudioFileFormat::Module;
    metadata._Container = takeString(openmpt_module_get_metadata(module, "container")); //"umx" and such; empty for a plain module
    metadata._Codec = details._Type;
    metadata._CodecIsFree = true; //module formats carry no patents
    metadata._CodecIsAllowed = true;
    metadata._Channels = 2;
    metadata._ChannelLayout = "stereo";
    metadata._SampleFormat = "flt";
    metadata._Title = takeString(openmpt_module_get_metadata(module, "title"));
    metadata._Artist = takeString(openmpt_module_get_metadata(module, "artist"));
    metadata._Date = takeString(openmpt_module_get_metadata(module, "date"));
    metadata._Comment = details._Message;

    // An endless song (a pattern loop libopenmpt can't follow to its end) has no length
    const f64 seconds { openmpt_module_get_duration_seconds(module) };
    metadata._Seconds = std::isfinite(seconds) ? seconds : 0.0;

    // Every item libopenmpt has, as tags; the keys come as one list separated by semicolons
    const std::string keys { takeString(openmpt_module_get_metadata_keys(module)) };

    for(uSize start = 0; start < keys.size();)
    {
        const uSize end { std::min(keys.find(';', start), keys.size()) };
        const std::string key { keys.substr(start, end - start) };

        if(!key.empty())
            metadata._Tags.emplace_back(key, takeString(openmpt_module_get_metadata(module, key.c_str())));

        start = end + 1;
    }

    metadata._Details = std::move(details);
    openmpt_module_destroy(module);
    return metadata;
}

bool RetroFuturaGUI::ModuleSource::Open(std::string_view path, const u32 sampleRate)
{
    Close();

    if(0 == sampleRate)
        return false;

    u64 fileSize { 0 };
    openmpt_module* module { createModule(readFile(path, std::numeric_limits<u64>::max(), fileSize)) };

    if(!module)
        return false;

    static const ma_data_source_vtable vtable
    {
        .onRead = &ModuleSource::onRead,
        .onSeek = &ModuleSource::onSeek,
        .onGetDataFormat = &ModuleSource::onGetDataFormat,
        .onGetCursor = &ModuleSource::onGetCursor,
        .onGetLength = &ModuleSource::onGetLength,
        .onSetLooping = &ModuleSource::onSetLooping,
        .flags = 0
    };

    ma_data_source_config config { ma_data_source_config_init() };
    config.vtable = &vtable;

    if(MA_SUCCESS != ma_data_source_init(&config, &_base))
    {
        openmpt_module_destroy(module);
        return false;
    }

    openmpt_module_select_subsong(module, 0);

    const std::scoped_lock lock { _mutex };
    _module = module;
    _sampleRate = sampleRate;
    measureSong();
    return true;
}

void RetroFuturaGUI::ModuleSource::measureSong()
{
    if(!_module)
        return;

    const f64 seconds { openmpt_module_get_duration_seconds(_module) };
    _songSeconds = std::isfinite(seconds) ? seconds : 0.0;
    _restartSeconds = 0.0;
    _positionOffset = 0.0;
    const i32 subsong { openmpt_module_get_selected_subsong(_module) };

    if(0 > subsong)
        return; // all subsongs one after another start over from the first one's start

    const f64 restart { openmpt_module_set_position_order_row(_module, openmpt_module_get_restart_order(_module, subsong), openmpt_module_get_restart_row(_module, subsong)) };
    openmpt_module_set_position_seconds(_module, 0.0);

    if(0.0 < restart && restart < _songSeconds)
        _restartSeconds = restart;
}

f64 RetroFuturaGUI::ModuleSource::getSongPosition() const
{
    const f64 position { openmpt_module_get_position_seconds(_module) - _positionOffset };
    const f64 repeatingSeconds { _songSeconds - _restartSeconds };

    // Played once, the last row's ticks run a little past the length libopenmpt estimates, and stay there
    if(!_isRepeating || position < _songSeconds || 0.0 >= repeatingSeconds)
        return position;

    return _restartSeconds + std::fmod(position - _restartSeconds, repeatingSeconds);
}

void RetroFuturaGUI::ModuleSource::Close()
{
    const std::scoped_lock lock { _mutex };

    if(!_module)
        return;

    ma_data_source_uninit(&_base);
    openmpt_module_destroy(_module);
    _module = nullptr;
    _sampleRate = 0;
    _songSeconds = 0.0;
    _restartSeconds = 0.0;
    _positionOffset = 0.0;
    _isRepeating = false;
}

bool RetroFuturaGUI::ModuleSource::IsOpen() const
{
    const std::scoped_lock lock { _mutex };
    return _module != nullptr;
}

ma_data_source* RetroFuturaGUI::ModuleSource::Get()
{
    return &_base;
}

uSize RetroFuturaGUI::ModuleSource::GetSubsongCount() const
{
    const std::scoped_lock lock { _mutex };

    if(!_module)
        return 0;

    return static_cast<uSize>(std::max(openmpt_module_get_num_subsongs(_module), 0));
}

bool RetroFuturaGUI::ModuleSource::SelectSubsong(const i32 index)
{
    const std::scoped_lock lock { _mutex };

    if(!_module)
        return false;

    if(1 != openmpt_module_select_subsong(_module, index))
        return false;

    measureSong();
    return true;
}

ma_result RetroFuturaGUI::ModuleSource::onRead(ma_data_source* dataSource, void* framesOut, ma_uint64 frameCount, ma_uint64* framesRead)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    if(!framesOut)
        return MA_INVALID_ARGS;

    if(!framesRead)
        return MA_INVALID_ARGS;

    ModuleSource* source { static_cast<ModuleSource*>(dataSource) };
    const std::scoped_lock lock { source->_mutex };

    if(!source->_module)
        return MA_INVALID_OPERATION;

    // Fewer frames than asked for only at the end of the song - never while it repeats
    *framesRead = openmpt_module_read_interleaved_float_stereo(source->_module, static_cast<i32>(source->_sampleRate), static_cast<size_t>(frameCount), static_cast<float*>(framesOut));
    return 0 < *framesRead ? MA_SUCCESS : MA_AT_END;
}

ma_result RetroFuturaGUI::ModuleSource::onSeek(ma_data_source* dataSource, ma_uint64 frameIndex)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    ModuleSource* source { static_cast<ModuleSource*>(dataSource) };
    const std::scoped_lock lock { source->_mutex };

    if(!source->_module)
        return MA_INVALID_OPERATION;

    openmpt_module_set_position_seconds(source->_module, static_cast<f64>(frameIndex) / source->_sampleRate);
    source->_positionOffset = 0.0; // libopenmpt's count starts over from the new position
    return MA_SUCCESS;
}

ma_result RetroFuturaGUI::ModuleSource::onGetDataFormat(ma_data_source* dataSource, ma_format* format, ma_uint32* channels, ma_uint32* sampleRate, ma_channel* channelMap, size_t channelMapCap)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    const ModuleSource* source { static_cast<const ModuleSource*>(dataSource) };

    setIfRequested(format, ma_format_f32);
    setIfRequested(channels, ma_uint32 { 2 });
    setIfRequested(sampleRate, ma_uint32 { source->_sampleRate });
    ma_channel_map_init_standard(ma_standard_channel_map_default, channelMap, channelMapCap, 2); // ignores a null map
    return MA_SUCCESS;
}

ma_result RetroFuturaGUI::ModuleSource::onGetCursor(ma_data_source* dataSource, ma_uint64* cursor)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    if(!cursor)
        return MA_INVALID_ARGS;

    const ModuleSource* source { static_cast<const ModuleSource*>(dataSource) };
    const std::scoped_lock lock { source->_mutex };

    if(!source->_module)
        return MA_INVALID_OPERATION;

    *cursor = static_cast<ma_uint64>(std::llround(source->getSongPosition() * source->_sampleRate));
    return MA_SUCCESS;
}

ma_result RetroFuturaGUI::ModuleSource::onGetLength(ma_data_source* dataSource, ma_uint64* length)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    if(!length)
        return MA_INVALID_ARGS;

    const ModuleSource* source { static_cast<const ModuleSource*>(dataSource) };
    const std::scoped_lock lock { source->_mutex };

    if(!source->_module)
        return MA_INVALID_OPERATION;

    // One pass through the subsong, repeating or not; an endless one has no length
    if(0.0 >= source->_songSeconds)
        return MA_NOT_IMPLEMENTED;

    *length = static_cast<ma_uint64>(std::llround(source->_songSeconds * source->_sampleRate));
    return MA_SUCCESS;
}

ma_result RetroFuturaGUI::ModuleSource::onSetLooping(ma_data_source* dataSource, ma_bool32 isLooping)
{
    if(!dataSource)
        return MA_INVALID_ARGS;

    ModuleSource* source { static_cast<ModuleSource*>(dataSource) };
    const std::scoped_lock lock { source->_mutex };

    if(!source->_module)
        return MA_INVALID_OPERATION;

    // Switched off after some repeats, the clock goes on from where the music is rather than from libopenmpt's count
    if(!isLooping && source->_isRepeating)
        source->_positionOffset = openmpt_module_get_position_seconds(source->_module) - source->getSongPosition();

    // -1 repeats forever from the module's own restart position, so miniaudio never sees an end to loop over
    openmpt_module_set_repeat_count(source->_module, isLooping ? -1 : 0);
    source->_isRepeating = MA_TRUE == isLooping;
    return MA_SUCCESS;
}

#else

//Without tracker modules nothing reads as a module, so nothing else here is ever reached

RetroFuturaGUI::ModuleSource::~ModuleSource() = default;
bool RetroFuturaGUI::ModuleSource::IsModule(std::string_view) { return false; }
std::optional<RetroFuturaGUI::AudioMetadata> RetroFuturaGUI::ModuleSource::ReadMetadata(std::string_view) { return std::nullopt; }
bool RetroFuturaGUI::ModuleSource::Open(std::string_view, const u32) { return false; }
void RetroFuturaGUI::ModuleSource::Close() {}
bool RetroFuturaGUI::ModuleSource::IsOpen() const { return false; }
ma_data_source* RetroFuturaGUI::ModuleSource::Get() { return &_base; }
uSize RetroFuturaGUI::ModuleSource::GetSubsongCount() const { return 0; }
bool RetroFuturaGUI::ModuleSource::SelectSubsong(const i32) { return false; }

#endif