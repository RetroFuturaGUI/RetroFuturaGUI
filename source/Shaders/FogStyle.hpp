#pragma once
#include "IncludeHelper.hpp"
#include "Shader.hpp"
#include <span>

namespace RetroFuturaGUI
{
    /// @brief The fog laid over a fill, for Plane and Text
    class FogStyle
    {
    protected:
        FogStyle() = default;
        ~FogStyle() = default;

        /// @brief Sets the overall opacity of the fog.
        void SetFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the fog drifts over time.
        void SetFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the fog's fractal noise. Turns the fog on when non-empty.
        void SetFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which fog appears. Higher values carve larger clear (fog-free) gaps out of the cloud.
        void SetFogClearing(const f32 clearing);

        /// @brief ShaderFeatures::FogEffect while a fog density is set, else 0, for FillStyle::uploadFill's extra features.
        u32 getFogFeatures() const;

        /// @brief Uploads the fog's uniforms. The fog drifts on by one step.
        void uploadFog(Shader& shader);

    private:
        std::span<f32> _fogDensity;
        uSize _fogDensityCount { 0 };
        f32
            _fogAlpha { 0.0f },
            _fogSpeed { 0.0f },
            _fogClearing { 0.5f },
            _fogAnimationOffset { 0.0f };
    };
}