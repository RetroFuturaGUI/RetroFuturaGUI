#pragma once
#include <memory>
#include "Shader.hpp"

namespace RetroFuturaGUI
{
    class ShaderManager
    {
    public:
        /// @brief Get the fill shader
        /// @return Reference to the fill Shader object
        static Shader& GetFillShader();
        /// @brief Get the fill gradient shader
        /// @return Reference to the fill gradient Shader object
        static Shader& GetFillGradientShader();
        /// @brief Get the line fill shader
        /// @return Reference to the line fill Shader object
        static Shader& GetLineFillShader();

        /// @brief Initialize the ShaderManager and compile all shaders
        static void Init();

    private:
        ShaderManager() = default;
        ~ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager(ShaderManager&&) = delete;
        void operator=(const ShaderManager&) = delete;
        void operator=(ShaderManager&&) = delete;
        static ShaderManager& GetInstance()
        {
            static ShaderManager Instance;
            return Instance;
        }
        
        static void compileShaders();
        static std::unique_ptr<Shader> _fill;
        static std::unique_ptr<Shader> _fillGradient;
        static std::unique_ptr<Shader> _lineFill;
    };
}
