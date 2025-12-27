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

        /// @brief Get the border fill shader
        /// @return Reference to the border fill Shader object
        static Shader& GetBorderFillShader();

        /// @brief Get the border fill gradient shader
        /// @return Reference to the border fill gradient Shader object
        static Shader& GetBorderFillGradientShader();

        /// @brief Get the line fill shader
        /// @return Reference to the line fill Shader object
        static Shader& GetLineFillShader();

        /// @brief Get the text fill shader
        /// @return Reference to the line fill Shader object
        static Shader& GetTextFillShader();

        /// @brief Get the text fill shader
        /// @return Reference to the line fill Shader object
        static Shader& GetTextureFillShader();

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
        static inline std::unique_ptr<Shader> _fill;
        static inline std::unique_ptr<Shader> _fillGradient;
        static inline std::unique_ptr<Shader> _borderFill;
        static inline std::unique_ptr<Shader> _borderFillGradient;
        static inline std::unique_ptr<Shader> _lineFill;
        static inline std::unique_ptr<Shader> _textFill;
        static inline std::unique_ptr<Shader> _textureFill;
    };
}