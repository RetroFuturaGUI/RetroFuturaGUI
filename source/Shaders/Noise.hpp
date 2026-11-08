#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    /// @brief How noise blends between the random values at the corners of its cells: Linear shows the cells as facets,
    ///        Cubic softens them, Quintic gives the softest, roundest blobs
    enum class NoiseSmoothing : u32
    {
        Linear,
        Cubic,
        Quintic
    };

    /// @brief The noise the glass is made of: it shifts the window seen through the glass, and without a backdrop image it is
    ///        the gray backdrop itself. Sizes are in window pixels, so the pattern keeps its size when the window is resized
    struct GlassNoise
    {
        f32 _CellSize { 128.0f }; // how big the noise cells shifting the window behind are
        f32 _Distortion { 12.0f }; // how far the window behind is shifted at most, in pixels
        f32 _BackdropCellSize { 256.0f }; // the gray backdrop's cells, shown when the glass has no backdrop image
        NoiseSmoothing _Smoothing { NoiseSmoothing::Linear };
        u32 _Seed { 0 }; // another seed gives another pattern
    };

    /// @brief The noise the fog is made of. How many layers of finer noise it adds up, and how strongly, is SetFogDensity's.
    ///        Sizes are in pixels
    struct FogNoise
    {
        f32 _CellSize { 170.0f }; // the cloud's coarsest cells
        f32 _DetailCellSize { 55.0f }; // the finer shading inside the cloud
        NoiseSmoothing _Smoothing { NoiseSmoothing::Quintic };
        u32 _Seed { 0 }; // another seed gives another pattern
    };
}
