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

#file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}/installed/${TARGETPLATFORM}/include" VCPKG_INSTALL_DIR)

add_executable("RetroFuturaGUI" 
    "${CMAKE_CURRENT_SOURCE_DIR}/source/widgets/IWidgetBase.cpp"
)

target_include_directories("RetroFuturaGUI" PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/widgets"
)

# Check for OpenGL installation
if(TARGETPLATFORM STREQUAL "x64-windows")
    find_package(OpenGL REQUIRED)
    if(OpenGL_FOUND)
        message(STATUS "OpenGL is installed and available for ${TARGETPLATFORM}")
    else()
        message(FATAL_ERROR "OpenGL is not installed or not found. Please install OpenGL development libraries.")
    endif()
elseif(TARGETPLATFORM STREQUAL "x64-linux")
    message(FATAL_ERROR "Linux not yet supported")
elseif(TARGETPLATFORM STREQUAL "x64-osx")
    message(FATAL_ERROR "macOS not yet supported")
endif()

# Check for GLFW installation
find_package(glfw3 CONFIG REQUIRED)
if(glfw3_FOUND)
    message(STATUS "GLFW is installed and found: ${GLFW3_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "GLFW is not found. Please install GLFW via VCPKG.")
endif()

# Check for gml installation
find_package(glm CONFIG REQUIRED)
if(glm_FOUND)
    message(STATUS "glm is installed and found: ${glm_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "glm is not found. Please install glm via VCPKG.")
endif()

# Check for GLEW installation
find_package(GLEW CONFIG REQUIRED)
if(GLEW_FOUND)
    message(STATUS "GLEW is installed and found: ${GLEW_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "GLEW is not found. Please install GLEW via VCPKG.")
endif()

# Check for glad installation
find_package(glad CONFIG REQUIRED)
if(TARGET glad::glad)
    message(STATUS "glad is installed and found: ${glad_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "glad is not found. Please install glad via VCPKG.")
endif()

# Check for Freetype installation
find_package(Freetype CONFIG REQUIRED)
if(TARGET Freetype::Freetype)
    message(STATUS "Freetype is installed and found: ${Freetype_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Freetype is not found. Please install Freetype via VCPKG.")
endif()

target_link_libraries("RetroFuturaGUI" PRIVATE
        OpenGL::GL 
        glfw
        Freetype::Freetype
        glm::glm-header-only
        GLEW::GLEW
        glad::glad
)
# & "C:\Program Files\CMake\bin\cmake.exe" .. CMakeLists.txt -DTARGETPLATFORM=x64-windows -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"