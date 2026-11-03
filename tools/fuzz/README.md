# Fuzz targets

Under MSVC's AddressSanitizer, read untrusted files run against inputs libFuzzer generates. Rerun them after
changing a parser.

| Target | What it reads |
|---|---|
| FuzzAudioMetadata | `AudioMetadataReader::Read` on any file: FFmpeg's demuxers and every hand-written reader behind them (WAV smpl, MP3 Xing/LAME, ADX, HCA, Vorbis, Opus, AIFF, Matroska, cover art), and libopenmpt |
| FuzzHeaders | `ReadHcaHeader` and `ReadMatroskaHeader` on bytes |
| FuzzMediaPreview | everything the decoder helper does to a file: images (stb_image, lunasvg), songs and their cover art, videos and their frames |
| FuzzPreviewProtocol | the decoder helper's replies as the program reads them, and its requests |

## Building

A fuzzing build folder of its own, since the instrumented library links into the fuzz targets only. From a Visual Studio x64 developer prompt (`vcvars64.bat`):

```
cmake -S . -B build/fuzz -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
      -DVCPKG_TARGET_TRIPLET=x64-windows -DTARGETPLATFORM=x64-windows -DRETROFUTURAGUI_FUZZING=ON
cmake --build build/fuzz --target RetroFuturaGUIFuzz
```

The targets land in `build/fuzz/fuzz/`, with the DLLs they need copied beside them.

## Running

AddressSanitizer's runtime (`clang_rt.asan_dbg_dynamic-x86_64.dll`) has to be found: run from the developer prompt, or put
`VC\Tools\MSVC\<version>\bin\Hostx64\x64` on PATH. Without it a target exits at once with 0xC0000135.

Give each a folder of real files to start from (a corpus; it grows as libFuzzer finds new paths) and a time:

```
build\fuzz\fuzz\FuzzAudioMetadata.exe corpus\audio -max_total_time=600 -rss_limit_mb=4096
```

A crash leaves its input as `crash-<hash>` in the current folder. Rerun the target with that file alone to reproduce it.
`FuzzSetup.hpp` makes the Debug standard library's own range checks (a subspan past the end, say) print what failed and
abort, so those crashes are saved too instead of ending the process silently.

FuzzAudioMetadata and FuzzMediaPreview write every input to a file and run FFmpeg's debug build, so they manage only a few
to a few dozen inputs a second; give them hours, not minutes.
FuzzAudioMetadata and FuzzMediaPreview take the first byte of an input to pick the file's kind and extension, so a real
file goes into their corpus with one byte in front.