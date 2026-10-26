#pragma once
#include "Plane.hpp"

namespace RetroFuturaGUI
{
    /// @brief A Plane that can be cut down to a sine wave running through its middle, such as IRangedValue's graph in its
    ///        Wave mode. With the wave hidden it draws like any Plane.
    class WavePlane final : public Plane
    {
    public:
        /// @brief Constructs a WavePlane that draws using the given projection. The wave starts hidden.
        WavePlane(Projection* projection);

        /// @brief Draws the plane, cut down to the wave line while the wave is shown.
        void Draw() override;

        /// @brief Shows or hides the wave. Hidden, the whole plane is drawn.
        void ShowWave(const bool show);

        /// @brief Sets the thickness of the wave line, in pixels.
        void SetWaveThickness(const f32 thickness);

        /// @brief Sets the amplitude (peak-to-center height) of the wave, in pixels.
        void SetWaveHeight(const f32 height);

        /// @brief Sets the horizontal distance between wave peaks, in pixels.
        void SetWaveLength(const f32 length);

    private:
        f32
            _waveThickness { 3.0f },
            _waveHeight { 6.0f },
            _waveLength { 40.0f };
    };
}
