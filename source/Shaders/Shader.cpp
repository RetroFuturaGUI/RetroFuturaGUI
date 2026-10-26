#include "Shader.hpp"
#include <format>
#include <fstream>
#include <print>

RetroFuturaGUI::Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath, const std::vector<std::string>& defines)
{
    std::string vertexCode = loadShaderSource(vertexPath, defines);
    std::string fragmentCode = loadShaderSource(fragmentPath, defines);
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    _programId = glCreateProgram();
    u32 vertexShader = compileShader(vShaderCode, GL_VERTEX_SHADER);
    u32 fragmentShader = compileShader(fShaderCode, GL_FRAGMENT_SHADER);
    u32 geometryShader = 0;

    if (geometryPath)
    {
        std::string geometryCode = loadShaderSource(geometryPath, defines);
        const char* gShaderCode = geometryCode.c_str();
        geometryShader = compileShader(gShaderCode, GL_GEOMETRY_SHADER);
    }

    glLinkProgram(_programId);

    i32 success;
    char infoLog[512];
    glGetProgramiv(_programId, GL_LINK_STATUS, &success);

    if (!success) 
    {
        glGetProgramInfoLog(_programId, 512, NULL, infoLog);
        std::println("ERROR::SHADER::LINKING_FAILED\n{}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryPath)
        glDeleteShader(geometryShader);

    //glUseProgram(_programId);
    // -1 for shaders that don't project anything - YuvToRgb, for one, renders into a texture, not onto the screen
    _projectionLocation = getUniformLocation("uProjection");
}

RetroFuturaGUI::Shader::~Shader()
{
    glDeleteProgram(_programId);
}

void RetroFuturaGUI::Shader::UseProgram()
{
    glUseProgram(_programId);
}

void RetroFuturaGUI::Shader::SetUniformBool(std::string_view name, bool value) const
{
    glUniform1i(getUniformLocation(name), (int)value);
}

void RetroFuturaGUI::Shader::SetUniformInt(std::string_view name, int value) const
{
    glUniform1i(getUniformLocation(name), value);
}

void RetroFuturaGUI::Shader::SetUniformFloat(std::string_view name, float value) const
{
    glUniform1f(getUniformLocation(name), value);
}

void RetroFuturaGUI::Shader::SetUniformFloat(std::string_view name, const f32* values, const u32 count) const
{
    glUniform1fv(getUniformLocation(name), count, values);
}

void RetroFuturaGUI::Shader::SetUniformVec2(std::string_view name, const glm::vec2& value) const
{
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}

void RetroFuturaGUI::Shader::SetUniformVec2(std::string_view name, const f32 * value, const u32 count) const
{
    glUniform2fv(getUniformLocation(name), count, value);
}

void RetroFuturaGUI::Shader::SetUniformVec2(std::string_view name, float x, float y) const
{
    glUniform2f(getUniformLocation(name), x, y);
}

void RetroFuturaGUI::Shader::SetUniformVec3(std::string_view name, const glm::vec3& value) const
{
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

void RetroFuturaGUI::Shader::SetUniformVec3(std::string_view name, const f32 * value, const u32 count) const
{
    glUniform3fv(getUniformLocation(name), count, value);
}

void RetroFuturaGUI::Shader::SetUniformVec3(std::string_view name, float x, float y, float z) const
{
    glUniform3f(getUniformLocation(name), x, y, z);
}

void RetroFuturaGUI::Shader::SetUniformVec4(std::string_view name, const glm::vec4& value) const
{
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

void RetroFuturaGUI::Shader::SetUniformVec4(std::string_view name, const float* value, const u32 count) const
{
    glUniform4fv(getUniformLocation(name), count, value);
}

void RetroFuturaGUI::Shader::SetUniformVec4(std::string_view name, float x, float y, float z, float w) const
{
    glUniform4f(getUniformLocation(name), x, y, z, w);
}

void RetroFuturaGUI::Shader::SetUniformMat2(std::string_view name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void RetroFuturaGUI::Shader::SetUniformMat3(std::string_view name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void RetroFuturaGUI::Shader::SetUniformMat4(std::string_view name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

u32 RetroFuturaGUI::Shader::GetProgramId() const
{
    return _programId;
}

i32 RetroFuturaGUI::Shader::GetProjectionLocation() const
{
    return _projectionLocation;
}

std::string RetroFuturaGUI::Shader::loadShaderSource(const char* shaderPath, const std::vector<std::string>& defines)
{
    std::string source;
    std::unordered_set<std::string> included;

    if(!appendWithIncludes(shaderPath, included, source))
        return {};

    if(defines.empty())
        return source;

    std::string defineLines;

    for(const std::string& define : defines)
        defineLines += std::format("#define {}\n", define);

    //#version has to stay the first line, so the defines go right below it
    const uSize versionStart { source.find("#version") };
    const uSize versionEnd { std::string::npos == versionStart ? std::string::npos : source.find('\n', versionStart) };
    source.insert(std::string::npos == versionEnd ? 0 : versionEnd + 1, defineLines);
    return source;
}

bool RetroFuturaGUI::Shader::appendWithIncludes(const std::filesystem::path& path, std::unordered_set<std::string>& included, std::string& source)
{
    std::ifstream file { path };

    if(!file)
    {
        std::println("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: {}", path.string());
        return false;
    }

    included.insert(std::filesystem::weakly_canonical(path).string());
    std::string line;

    while(std::getline(file, line))
    {
        const uSize first { line.find_first_not_of(" \t") };

        if(std::string::npos == first || 0 != line.compare(first, 8, "#include"))
        {
            source += line;
            source += '\n';
            continue;
        }

        const uSize
            open { line.find('"', first) },
            close { std::string::npos == open ? std::string::npos : line.find('"', open + 1) };

        if(std::string::npos == close)
        {
            std::println("ERROR::SHADER::INCLUDE_WITHOUT_QUOTED_FILE: {} in {}", line, path.string());
            return false;
        }

        const std::filesystem::path includedPath { path.parent_path() / line.substr(open + 1, close - open - 1) };

        if(included.contains(std::filesystem::weakly_canonical(includedPath).string()))
            continue;

        if(!appendWithIncludes(includedPath, included, source))
            return false;
    }

    return true;
}

u32 RetroFuturaGUI::Shader::compileShader(const char* shaderCode, const int shaderType)
{
    if (shaderType != GL_VERTEX_SHADER && shaderType != GL_FRAGMENT_SHADER && shaderType != GL_GEOMETRY_SHADER)
    {
        std::println("ERROR::SHADER::INVALID_SHADER_TYPE");
        return 0;
    }

    if (!shaderCode || shaderCode[0] == '\0')
    {
        std::println("ERROR::SHADER::EMPTY_SHADER_CODE");
        return 0;
    }

    u32 shaderObject;
    i32 success;
    char infoLog[512];

    shaderObject = glCreateShader(shaderType);
    glShaderSource(shaderObject, 1, &shaderCode, NULL);
    glCompileShader(shaderObject);
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);

    if (!success) 
    {
        const char* shaderTypeStr = "";

        switch(shaderType) 
        {
            case GL_VERTEX_SHADER: shaderTypeStr = "VERTEX"; break;
            case GL_FRAGMENT_SHADER: shaderTypeStr = "FRAGMENT"; break;
            case GL_GEOMETRY_SHADER: shaderTypeStr = "GEOMETRY"; break;
            default: shaderTypeStr = "UNKNOWN"; break;
        }

        glGetShaderInfoLog(shaderObject, 512, NULL, infoLog);
        std::println("ERROR::SHADER::{}::COMPILATION_FAILED\n{}", shaderTypeStr, infoLog);
        return 0;
    }

    glAttachShader(_programId, shaderObject);
    return shaderObject;
}

i32 RetroFuturaGUI::Shader::getUniformLocation(std::string_view name) const
{
    const auto found { _uniformLocations.find(name) };

    if(_uniformLocations.end() != found)
        return found->second;

    std::string key { name }; //GL wants a terminated string, and the map keeps it
    const i32 location { glGetUniformLocation(_programId, key.c_str()) };
    _uniformLocations.emplace(std::move(key), location);
    return location;
}