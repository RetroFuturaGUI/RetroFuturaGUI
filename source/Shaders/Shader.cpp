#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <print>

RetroFuturaGUI::Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode = loadShaderFile(vertexPath);
    std::string fragmentCode = loadShaderFile(fragmentPath);
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    _programId = glCreateProgram();
	compileShader(vShaderCode, GL_VERTEX_SHADER);
	compileShader(fShaderCode, GL_FRAGMENT_SHADER);
}

RetroFuturaGUI::Shader::~Shader()
{
    glDeleteProgram(_programId);
}

void RetroFuturaGUI::Shader::UseProgram()
{
    glUseProgram(_programId);
}

void RetroFuturaGUI::Shader::SetUniformBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(_programId, name.c_str()), (int)value);
}

void RetroFuturaGUI::Shader::SetUniformInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(_programId, name.c_str()), value);
}

void RetroFuturaGUI::Shader::SetUniformFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(_programId, name.c_str()), value);
}

void RetroFuturaGUI::Shader::SetUniformVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(_programId, name.c_str()), 1, &value[0]);
}

void RetroFuturaGUI::Shader::SetUniformVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(_programId, name.c_str()), x, y);
}

void RetroFuturaGUI::Shader::SetUniformVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(_programId, name.c_str()), 1, &value[0]);
}

void RetroFuturaGUI::Shader::SetUniformVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(_programId, name.c_str()), x, y, z);
}

void RetroFuturaGUI::Shader::SetUniformVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(_programId, name.c_str()), 1, &value[0]);
}

void RetroFuturaGUI::Shader::SetUniformVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(_programId, name.c_str()), x, y, z, w);
}

void RetroFuturaGUI::Shader::SetUniformMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(_programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void RetroFuturaGUI::Shader::SetUniformMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(_programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void RetroFuturaGUI::Shader::SetUniformMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(_programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

uint32_t RetroFuturaGUI::Shader::GetProgramId() const
{
    return _programId;
}

std::string RetroFuturaGUI::Shader::loadShaderFile(const char* shaderPath)
{
    std::string shaderCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        shaderFile.open(shaderPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::println("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: {0}", shaderPath);
    }

    return shaderCode;
}

void RetroFuturaGUI::Shader::compileShader(const char* shaderCode, const int shaderType)
{
	if (shaderType != GL_VERTEX_SHADER && shaderType != GL_FRAGMENT_SHADER)
	{
		std::println("ERROR::SHADER::INVALID_SHADER_TYPE");
		return;
	}

	if (shaderCode == nullptr || shaderCode[0] == '\0')
	{
		std::println("ERROR::SHADER::EMPTY_SHADER_CODE");
		return;
	}

    uint32_t shaderObject;
    int success;
    char infoLog[512];

    shaderObject = glCreateShader(shaderType);
    glShaderSource(shaderObject, 1, &shaderCode, NULL);
    glCompileShader(shaderObject);
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shaderObject, 512, NULL, infoLog);
        std::println("ERROR::SHADER::{}::COMPILATION_FAILED\n{}", shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT", infoLog);
        return;
    };

    glAttachShader(_programId, shaderObject);
    glLinkProgram(_programId);
    glGetProgramiv(_programId, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(_programId, 512, NULL, infoLog);
        std::println("ERROR::SHADER::{}::LINKING_FAILED\n{}", shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT", infoLog);
        return;
    }

    glDeleteShader(shaderObject);
}