cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project("RetroFuturaGUI" VERSION 0.1.0)

message(STATUS "RetroFuturaGUI CMake Configuration")
message(STATUS "RetroFuturaGUI Version: ${PROJECT_VERSION}")
message(STATUS "Host OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Host Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Host OS Version: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Target Platform: ${TARGETPLATFORM}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/source/config.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/source/config.hpp" @ONLY)
message(STATUS "Configured file: ${CMAKE_CURRENT_SOURCE_DIR}/source/config.hpp")

if(DEFINED ENV{VCPKG_ROOT})
    message(STATUS "VCPKG_ROOT is set to: $ENV{VCPKG_ROOT}")
else()
    message(FATAL_ERROR "VCPKG is not installed or VCPKG_ROOT environment variable is not set. Please install VCPKG and/or set the VCPKG_ROOT environment variable.")
endif()

set(RetroFuturaGuiSources)

list(APPEND RetroFuturaGuiSources
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Animation/Transition.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Fonts/FontManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Fonts/FreeTypeGlyphSource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Fonts/ColorGlyphPainter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Fonts/Text.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/AudioDecodeThread.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/AudioMetadataReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/AudioPlayback.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/AudioSpectrum.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/AudioStream.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/AudioTrack.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/Decoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/HcaCipher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/MediaFrame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/MediaSource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/IsolatedDecoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/MediaPreview.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/MediaPreviewProtocol.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/MiniaudioImpl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/ModuleSource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/Packet.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/PacketQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/Resampler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/RingBufferSource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/VideoDecodeThread.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/VideoFrameQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/VideoMetadata.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/VideoPlayback.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback/VideoStill.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc/BackgroundWork.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc/ContextMenu.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc/DoubleEncodedString.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc/Mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc/Polyline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc/ResourceManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc/TextTypes.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc/Tooltip.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/OpenGlHelpers/ContextObjects.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/OpenGlHelpers/Projection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/OpenGlHelpers/ScissorClip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/OpenGlHelpers/SvgTexture.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles/ColorField.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles/Frame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles/ShadowRectangle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles/GlowRectangle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles/InnerGlowRectangle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles/IRectangle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles/Plane.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles/WavePlane.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Scenes/Scene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Scenes/SceneLoader.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders/FillStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders/FogStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders/DropShadow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders/Glow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders/InnerGlow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders/StateFillStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders/Shader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders/ShaderManager.cpp"
    
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Texture/ITexture.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Texture/Texture.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Texture/VideoTexture.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Button.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/CheckBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/ColorPicker.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/ColorPreview.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/ComboBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Histogram.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Image.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Label.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/LineDiagram.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Model.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/MultilineTextBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/ProgressBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/RadioButton.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/SeparatorLine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Slider.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/SvgImage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/TextBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Video.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/WindowTab.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Containers/Environment.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Containers/ExtendedComboBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Containers/Lasagna.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Containers/MenuBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Containers/Prefab.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Containers/RadioButtonGroup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Containers/ScrollView.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/IDropDown.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/IHierarchyNode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/IRangedMultiValue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/IRangedSingleValue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/IRangedValue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/ITextBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Mechanisms/TextEditor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Mechanisms/WidgetRegistry.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Styling/Flavor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Styling/StylingManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/ITypedText.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/IWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces/IClickable.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/IWidgetDecoration.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/Background.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/Border.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/WidgetText.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/Outline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/Caret.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/SelectedArea.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/Indicator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/Graph.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/Icon.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/DropDownList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/Box.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/ItemTextStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/ColorPickerIndicator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/CloseButton.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts/HoverText.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table/TableStyles.cpp"
    
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table/ITableTextProperties.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table/ITableWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table/Table.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table/TableCheckBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table/TableColor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table/TableImage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table/TableText.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/WindowDockDrag.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/WindowDocking.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/WindowInput.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/WindowLayout.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/WindowLayoutFile.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/WindowPopups.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/WindowTabs.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/IPopup.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/FileExplorer/FileExplorer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/FileExplorer/FileExplorerPrompts.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/FileExplorer/FilePreview.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/FileExplorer/FileTransfer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/FileExplorer/FileUndoHistory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/FileExplorer/ThumbnailCache.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/FileExplorer/StillCache.cpp"
)

if(DEFINED DYNLIB_MODE AND DYNLIB_MODE)
    list(APPEND RetroFuturaGuiSources
        "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Mechanisms/DynamicLibWidgetManager.cpp"
    )
endif()

add_library("RetroFuturaGUI" STATIC "${RetroFuturaGuiSources}")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/PlatformBridge")

target_include_directories("RetroFuturaGUI" PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Animation"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Borders"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Fonts"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/MediaPlayback"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Misc"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Scenes"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Styling"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Containers"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Interfaces"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Mechanisms"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Parts"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Table"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Window/Dialog/FileExplorer"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Widgets/Prefabs"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/OpenGlHelpers"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Rectangles"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Shaders"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/Texture"
)

# Check for OpenGL installation
find_package(OpenGL REQUIRED)
if(OpenGL_FOUND)
    message(STATUS "OpenGL is installed and available for ${TARGETPLATFORM}")
else()
    message(FATAL_ERROR "OpenGL is not installed or not found. Please install OpenGL development libraries.")
endif()

if(TARGETPLATFORM STREQUAL "x64-osx")
    message(FATAL_ERROR "macOS not yet supported")
endif()

# Check for GLFW installation
find_package(glfw3 CONFIG REQUIRED)
if(glfw3_FOUND)
    message(STATUS "GLFW is installed and found")
else()
    message(FATAL_ERROR "GLFW is not found. Please install GLFW via VCPKG.")
endif()

# Check for gml installation
find_package(glm CONFIG REQUIRED)
if(glm_FOUND)
    message(STATUS "glm is installed and found")
else()
    message(FATAL_ERROR "glm is not found. Please install glm via VCPKG.")
endif()

# Check for GLEW installation
find_package(GLEW CONFIG REQUIRED)
if(GLEW_FOUND)
    message(STATUS "GLEW is installed and found")
else()
    message(FATAL_ERROR "GLEW is not found. Please install GLEW via VCPKG.")
endif()

# Check for glad installation
find_package(glad CONFIG REQUIRED)
if(TARGET glad::glad)
    message(STATUS "glad is installed and found")
else()
    message(FATAL_ERROR "glad is not found. Please install glad via VCPKG.")
endif()

# Check for Freetype installation
find_package(Freetype CONFIG REQUIRED)
if(TARGET Freetype::Freetype)
    message(STATUS "Freetype is installed and found")
else()
    message(FATAL_ERROR "Freetype is not found. Please install Freetype via VCPKG.")
endif()

# Check for HarfBuzz installation: shapes text with the FreeType fonts (MIT licensed)
find_package(harfbuzz CONFIG REQUIRED)
if(TARGET harfbuzz::harfbuzz)
    message(STATUS "HarfBuzz is installed and found")
else()
    message(FATAL_ERROR "HarfBuzz is not found. Please install harfbuzz via VCPKG.")
endif()

# Check for SheenBidi installation: splits right-to-left and left-to-right text into runs (Apache 2.0 licensed)
find_package(SheenBidi CONFIG REQUIRED)
if(TARGET SheenBidi::SheenBidi)
    message(STATUS "SheenBidi is installed and found")
else()
    message(FATAL_ERROR "SheenBidi is not found. Please install sheenbidi via VCPKG.")
endif()

# Check for Stb installation
find_package(Stb REQUIRED COMPONENTS stb_image)
if(Stb_FOUND)
    message(STATUS "Stb is installed and found: ${Stb_DIR}")
    target_include_directories("RetroFuturaGUI" PUBLIC ${Stb_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Stb is not found. Please install Stb via VCPKG.")
endif()

# Check for Assimp installation
find_package(assimp CONFIG REQUIRED)
if(TARGET assimp::assimp)
    message(STATUS "Assimp is installed and found")
else()
    message(FATAL_ERROR "Assimp is not found. Please install Assimp via VCPKG.")
endif()

# check for lunasvg
find_package(lunasvg CONFIG REQUIRED)
if(TARGET lunasvg::lunasvg)
    message(STATUS "lunasvg is installed and found")
else()
    message(FATAL_ERROR "lunasvg is not found. Please install lunasvg via VCPKG.")
endif()

# check for ffmpeg
find_package(FFMPEG REQUIRED)
if(FFMPEG_FOUND)
    message(STATUS "FFMPEG is installed and found")
else()
    message(FATAL_ERROR "FFMPEG is not found. Please install FFMPEG via VCPKG.")
endif()

# check for miniaudio
find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")
if(MINIAUDIO_INCLUDE_DIRS)
    message(STATUS "miniaudio is installed and found")
else()
    message(FATAL_ERROR "miniaudio is not found. Please install miniaudio via VCPKG.")
endif()

target_include_directories("RetroFuturaGUI" PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_directories("RetroFuturaGUI" PUBLIC ${FFMPEG_LIBRARY_DIRS})
target_include_directories("RetroFuturaGUI" PUBLIC ${MINIAUDIO_INCLUDE_DIRS})

target_link_libraries("RetroFuturaGUI" PUBLIC
    OpenGL::GL 
    glfw
    Freetype::Freetype
    harfbuzz::harfbuzz
    SheenBidi::SheenBidi
    glm::glm-header-only
    GLEW::GLEW
    glad::glad
    assimp::assimp
    lunasvg::lunasvg
    ${FFMPEG_LIBRARIES}
    PlatformBridge
)

string(TOLOWER "${TARGETPLATFORM}" TARGETPLATFORM_LOWER)

if(TARGETPLATFORM_LOWER STREQUAL "x64-linux")
    target_compile_definitions("RetroFuturaGUI" PUBLIC TARGET_PLATFORM_LINUX)
elseif(TARGETPLATFORM_LOWER STREQUAL "x64-windows")
    #NOMINMAX: windows.h, which PlatformBridge's headers bring along, would otherwise define min and max as macros
    target_compile_definitions("RetroFuturaGUI" PUBLIC TARGET_PLATFORM_WINDOWS NOMINMAX)
endif()

# Decode royalty-free codecs only (DecodingPolicy.hpp's FreeCodecs): on by default, for a build that may ship commercially.
# Off, FFmpeg decodes every codec its build has, H.264, HEVC and AAC among them, for personal or non-commercial, or licensed commercial use.
option(RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY "Decode royalty-free codecs only. Whoever ships a build with this off takes on the licences of the codecs it decodes (H.264, HEVC, AAC, ...)" ON)

if(RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY)
    target_compile_definitions("RetroFuturaGUI" PUBLIC RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY=1)
else()
    target_compile_definitions("RetroFuturaGUI" PUBLIC RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY=0)
    message(WARNING "RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY is off: this build decodes every codec FFmpeg has, H.264, HEVC and AAC among them. Shipping it means taking on those codecs' patent licences - keep it to personal or non-commercial use")
endif()

# Tracker modules (MOD, XM, S3M, IT and about 60 more) through libopenmpt (BSD), which brings mpg123 (LGPL 2.1, linked
# dynamically), libvorbis, libogg and zlib along for the samples some modules compress. Off leaves all of them out.
option(RETROFUTURAGUI_TRACKER_MODULES "Play tracker modules (MOD, XM, S3M, IT, ...) through libopenmpt" ON)

if(RETROFUTURAGUI_TRACKER_MODULES)
    find_package(libopenmpt CONFIG REQUIRED)
    target_link_libraries("RetroFuturaGUI" PRIVATE libopenmpt::libopenmpt)
    target_compile_definitions("RetroFuturaGUI" PUBLIC RETROFUTURAGUI_TRACKER_MODULES=1)
else()
    target_compile_definitions("RetroFuturaGUI" PUBLIC RETROFUTURAGUI_TRACKER_MODULES=0)
endif()

# Fuzzing (MSVC): the library is built with AddressSanitizer and libFuzzer's coverage counters, and tools/fuzz adds the fuzz
# targets. Such a library links into the fuzz targets only, so a fuzzing build folder builds just them (tools/fuzz/README.md).
option(RETROFUTURAGUI_FUZZING "Build RetroFuturaGUI instrumented for fuzzing, with the fuzz targets in tools/fuzz (MSVC)" OFF)

if(RETROFUTURAGUI_FUZZING)
    if(NOT MSVC)
        message(FATAL_ERROR "RETROFUTURAGUI_FUZZING uses MSVC's /fsanitize=address,fuzzer")
    endif()

    # The runtime checks and incremental linking don't go together with AddressSanitizer
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    target_compile_options("RetroFuturaGUI" PRIVATE /fsanitize=address /fsanitize-coverage=inline-8bit-counters /fsanitize-coverage=edge /fsanitize-coverage=trace-cmp)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tools/fuzz")
else()
    # The decoder helper: the file explorer's previews and thumbnails are decoded in a process of their own (IsolatedDecoder),
    # so a malicious or broken file takes only the helper down. Built into the top build folder, beside the program and
    # library that use RetroFuturaGUI, where IsolatedDecoder looks for it.
    add_executable("RetroFuturaGUIDecoder" "${CMAKE_CURRENT_SOURCE_DIR}/source/DecoderHelper/RetroFuturaGUIDecoder.cpp")
    target_link_libraries("RetroFuturaGUIDecoder" PRIVATE "RetroFuturaGUI")
    set_target_properties("RetroFuturaGUIDecoder" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
