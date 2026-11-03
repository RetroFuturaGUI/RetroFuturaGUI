# Fuzz targets for the readers of untrusted files with MSVC's AddressSanitizer and libFuzzer. Added by RetroFuturaGUI's
# CMakeLists when RETROFUTURAGUI_FUZZING is on, which also instruments the library itself; see README.md here.

set(RetroFuturaGuiFuzzTargets FuzzAudioMetadata FuzzHeaders FuzzMediaPreview FuzzPreviewProtocol)

foreach(target IN LISTS RetroFuturaGuiFuzzTargets)
    add_executable(${target} "${CMAKE_CURRENT_SOURCE_DIR}/${target}.cpp")
    target_link_libraries(${target} PRIVATE "RetroFuturaGUI")
    # The objects name AddressSanitizer's and libFuzzer's runtimes themselves; libFuzzer brings main
    target_compile_options(${target} PRIVATE /fsanitize=address /fsanitize=fuzzer)
    target_link_options(${target} PRIVATE /INCREMENTAL:NO)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fuzz")
endforeach()

add_custom_target(RetroFuturaGUIFuzz DEPENDS ${RetroFuturaGuiFuzzTargets})