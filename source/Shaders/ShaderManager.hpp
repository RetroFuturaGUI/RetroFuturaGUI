#pragma once
#include <memory>
#include "Shader.hpp"

namespace RetroFuturaGUI
{
    class ShaderManager
    {
    public:
        /// @brief Get the solid fill shader
        /// @return Reference to the solid fill Shader object
        static Shader& GetSolidFillShader();

        /// @brief Get the linear gradient shader
        /// @return Reference to the linear gradient Shader object
        static Shader& GetLinearGradientShader();

        /// @brief Get the radial gradient shader
        /// @return Reference to the radial gradient Shader object
        static Shader& GetRadialGradientShader();
        
        /// @brief Get the hue star gradient shader
        /// @return Reference to the hue star gradient Shader object
        static Shader& GetHueStarGradientShader();

        /// @brief Get the border solid fill shader
        /// @return Reference to the border solid fill Shader object
        static Shader& GetBorderSolidFillShader();

        /// @brief Get the border linear gradient shader
        /// @return Reference to the border linear gradient Shader object
        static Shader& GetBorderLinearGradientShader();

        /// @brief Get the border radial gradient shader
        /// @return Reference to the border radial gradient Shader object
        static Shader& GetBorderRadialGradientShader();

        /// @brief Get the border hue star gradient shader
        /// @return Reference to the border hue star gradient Shader object
        static Shader& GetBorderHueStarGradientShader();

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

        /// @brief Get the SVG mask linear gradient shader
        /// @return Reference to the SVG mask linear gradient Shader object
        static Shader& GetSvgMaskLinearGradientShader();

        /// @brief Get the SVG mask radial gradient shader
        /// @return Reference to the SVG mask radial gradient Shader object
        static Shader& GetSvgMaskRadialGradientShader();

        /// @brief Get the SVG mask hue star gradient shader
        /// @return Reference to the SVG mask hue star gradient Shader object
        static Shader& GetSvgMaskHueStarGradientShader();

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
        static inline std::unique_ptr<Shader> _solidFill;
        static inline std::unique_ptr<Shader> _linearGradient;
        static inline std::unique_ptr<Shader> _radialGradient;
        static inline std::unique_ptr<Shader> _hueStarGradient;
        static inline std::unique_ptr<Shader> _borderSolidFill;
        static inline std::unique_ptr<Shader> _borderLinearGradient;
        static inline std::unique_ptr<Shader> _borderRadialGradient;
        static inline std::unique_ptr<Shader> _borderHueStarGradient;
        static inline std::unique_ptr<Shader> _lineFill;
        static inline std::unique_ptr<Shader> _fontAtlasFill;
        static inline std::unique_ptr<Shader> _textureFill;
        static inline std::unique_ptr<Shader> _svgMaskSolidFill;
        static inline std::unique_ptr<Shader> _svgMaskLinearGradient;
        static inline std::unique_ptr<Shader> _svgMaskRadialGradient;
        static inline std::unique_ptr<Shader> _svgMaskHueStarGradient;
    };
}