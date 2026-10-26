#pragma once
#include <array>
#include <memory>
#include "Shader.hpp"

namespace RetroFuturaGUI
{
    class ShaderManager
    {
    public:
        /// @brief Get the shader of a Plane with the given fill
        /// @return Reference to the Plane Shader object for that fill
        static Shader& GetPlaneShader(FillType fillType);

        /// @brief Get the shader of a WavePlane with the given fill, which is a Plane's shader with the wave line
        /// @return Reference to the WavePlane Shader object for that fill
        static Shader& GetWavePlaneShader(FillType fillType);

        /// @brief Get the shader of a Frame with the given fill
        /// @return Reference to the Frame Shader object for that fill
        static Shader& GetFrameShader(FillType fillType);

        /// @brief Get the shader of a ColorField with the given shape
        /// @return Reference to the ColorField Shader object for that shape
        static Shader& GetColorFieldShader(ColorFieldShape shape);

        /// @brief Get the line fill shader
        /// @return Reference to the line fill Shader object
        static Shader& GetLineFillShader();

        /// @brief Get the font atlas fill shader
        /// @return Reference to the font atlas fill Shader object
        static Shader& GetFontAtlasFillShader();

        /// @brief Get the texture fill shader
        /// @return Reference to the texture fill Shader object
        static Shader& GetTextureFillShader();

        /// @brief Get the SVG mask solid fill shader
        /// @return Reference to the SVG mask solid fill Shader object
        static Shader& GetSvgMaskSolidFillShader();

        /// @brief Get the SVG mask linear shift shader
        /// @return Reference to the SVG mask linear shift Shader object
        static Shader& GetSvgMaskLinearShiftShader();

        /// @brief Get the SVG mask radial gradient shader
        /// @return Reference to the SVG mask radial gradient Shader object
        static Shader& GetSvgMaskRadialGradientShader();

        /// @brief Get the SVG mask hue star gradient shader
        /// @return Reference to the SVG mask hue star gradient Shader object
        static Shader& GetSvgMaskHueStarGradientShader();

        /// @brief Get the YUV to RGB shader, which converts decoded video pictures into an RGBA texture
        /// @return Reference to the YUV to RGB Shader object
        static Shader& GetYuvToRgbShader();

        /// @brief Initialize the ShaderManager and compile all shaders
        static void Init();

    private:
        ShaderManager() = default;
        ~ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager(ShaderManager&&) = delete;
        auto operator=(const ShaderManager&) = delete;
        auto operator=(ShaderManager&&) = delete;
        static ShaderManager& GetInstance()
        {
            static ShaderManager Instance;
            return Instance;
        }
        
        static void compileShaders();
        static constexpr uSize _fillTypeCount { 4 }; //solid, linear shift, radial and hue star
        static constexpr uSize _colorFieldShapeCount { 2 };
        static inline std::array<std::unique_ptr<Shader>, _fillTypeCount> _planeShaders;
        static inline std::array<std::unique_ptr<Shader>, _fillTypeCount> _wavePlaneShaders;
        static inline std::array<std::unique_ptr<Shader>, _fillTypeCount> _frameShaders;
        static inline std::array<std::unique_ptr<Shader>, _colorFieldShapeCount> _colorFieldShaders;
        static inline std::unique_ptr<Shader> _lineFill;
        static inline std::unique_ptr<Shader> _fontAtlasFill;
        static inline std::unique_ptr<Shader> _textureFill;
        static inline std::unique_ptr<Shader> _svgMaskSolidFill;
        static inline std::unique_ptr<Shader> _svgMaskLinearShift;
        static inline std::unique_ptr<Shader> _svgMaskRadialGradient;
        static inline std::unique_ptr<Shader> _svgMaskHueStarGradient;
        static inline std::unique_ptr<Shader> _yuvToRgb;
    };
}