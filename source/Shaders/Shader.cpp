#include "Shader.hpp"
#include <algorithm>
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

RetroFuturaGUI::Shader::OpacityScope::OpacityScope(const f32 opacity)
    : _outerOpacity(_drawOpacity)
{
    _drawOpacity = _outerOpacity * std::clamp(opacity, 0.0f, 1.0f);
}

RetroFuturaGUI::Shader::OpacityScope::OpacityScope(const f32 opacity, const FrameComposition composition)
    : _outerOpacity(_drawOpacity)
{
    const f32 own { std::clamp(opacity, 0.0f, 1.0f) };
    _drawOpacity = FrameComposition::Instead == composition ? own : _outerOpacity * own;
}

RetroFuturaGUI::Shader::OpacityScope::~OpacityScope()
{
    _drawOpacity = _outerOpacity;
}

RetroFuturaGUI::Shader::FrameScope::FrameScope(const glm::mat4& frame, const FrameComposition composition)
    : _outerFrame(_drawFrame)
{
    _drawFrame = FrameComposition::Inside == composition ? _outerFrame * frame : frame;
}

RetroFuturaGUI::Shader::FrameScope::~FrameScope()
{
    _drawFrame = _outerFrame;
}

const glm::mat4& RetroFuturaGUI::Shader::GetDrawFrame()
{
    return _drawFrame;
}

RetroFuturaGUI::Shader::ClipScope::ClipScope(const glm::vec4& box, const f32 depth, const Projection& projection)
    : _outerPlanes(_clipPlanes), _outerPlaneCount(_clipPlaneCount), _outerBox(_clipBox), _outerBoxFrame(_clipBoxFrame),
      _outerBoxView(_clipBoxView), _outerBoxDepth(_clipBoxDepth)
{
    glm::vec4 cut { box };

    if(0 < _clipPlaneCount && _clipBoxFrame == _drawFrame && _clipBoxDepth == depth && _clipBoxView == projection.GetProjectionMatrix())
    {
        // Seen as the rectangle inside. Their overlap takes that rectangle's planes
        const glm::vec2
            low { glm::max(glm::vec2(box), glm::vec2(_clipBox)) },
            high { glm::min(glm::vec2(box) + glm::vec2(box.z, box.w), glm::vec2(_clipBox) + glm::vec2(_clipBox.z, _clipBox.w)) };
        cut = glm::vec4(low, glm::max(high - low, glm::vec2(0.0f)));
        _clipPlaneCount -= 4;
    }
    else if(_maxClipPlanes < _clipPlaneCount + 4)
    {
        // A third frame. The innermost rectangle so far stays, the outer one goes
        std::copy(_clipPlanes.begin() + 4, _clipPlanes.begin() + 8, _clipPlanes.begin());
        _clipPlaneCount = 4;
    }

    const std::array<glm::vec4, 4> planes { clipPlanesOf(cut, depth, _drawFrame, projection) };
    std::copy(planes.begin(), planes.end(), _clipPlanes.begin() + _clipPlaneCount);
    _clipPlaneCount += 4;
    _clipBox = cut;
    _clipBoxFrame = _drawFrame;
    _clipBoxView = projection.GetProjectionMatrix();
    _clipBoxDepth = depth;
    enableClipPlanes(_clipPlaneCount, _outerPlaneCount);
}

RetroFuturaGUI::Shader::ClipScope::~ClipScope()
{
    const u32 innerPlaneCount { _clipPlaneCount };
    _clipPlanes = _outerPlanes;
    _clipPlaneCount = _outerPlaneCount;
    _clipBox = _outerBox;
    _clipBoxFrame = _outerBoxFrame;
    _clipBoxView = _outerBoxView;
    _clipBoxDepth = _outerBoxDepth;
    enableClipPlanes(_clipPlaneCount, innerPlaneCount);
}

RetroFuturaGUI::Shader::UnclippedScope::UnclippedScope()
    : _outerPlaneCount(_clipPlaneCount)
{
    _clipPlaneCount = 0; //the planes stay where they are for the scopes around it
    enableClipPlanes(0, _outerPlaneCount);
}

RetroFuturaGUI::Shader::UnclippedScope::~UnclippedScope()
{
    _clipPlaneCount = _outerPlaneCount;
    enableClipPlanes(_clipPlaneCount, 0);
}

RetroFuturaGUI::Shader::DepthClearScope::DepthClearScope(std::function<void()> clear)
    : _outerClear(std::move(_depthClear))
{
    _depthClear = std::move(clear);
}

RetroFuturaGUI::Shader::DepthClearScope::~DepthClearScope()
{
    _depthClear = std::move(_outerClear);
}

void RetroFuturaGUI::Shader::ClearDepth()
{
    if(!_depthClear)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        return;
    }

    _depthClear();
}

std::array<glm::vec4, 4> RetroFuturaGUI::Shader::clipPlanesOf(const glm::vec4& box, const f32 depth, const glm::mat4& frame, const Projection& projection)
{
    // The box's corners and middle where the frame places them. Counterclockwise from the bottom left. A frame that carries a
    // turned tab's content has a perspective of its own, so the place is divided by its w
    const auto place = [&frame, depth](const f32 x, const f32 y) { const glm::vec4 placed { frame * glm::vec4(x, y, depth, 1.0f) }; return glm::vec3(placed) / placed.w; };
    const std::array<glm::vec3, 4> corners { place(box.x, box.y), place(box.x + box.z, box.y), place(box.x + box.z, box.y + box.w), place(box.x, box.y + box.w) };
    const glm::vec3 middle { place(box.x + box.z * 0.5f, box.y + box.w * 0.5f) };
    std::array<glm::vec4, 4> planes {};

    for(uSize edge = 0; 4 > edge; ++edge)
    {
        // The plane holds the edge and the view or the orthographic direction the eye looks along
        const glm::vec3
            start { corners[edge] },
            end { corners[(edge + 1) % 4] },
            towardEye { projection.GetTowardEye(start) };
        glm::vec3 normal { glm::cross(end - start, towardEye) };
        const f32 normalLength { glm::length(normal) };

        if(1e-6f > normalLength)
        {
            planes[edge] = glm::vec4(0.0f, 0.0f, 0.0f, -1.0f); //seen edge-on (the box shows nothing)
            continue;
        }

        normal /= normalLength;

        if(0.0f > glm::dot(normal, middle - start))
            normal = -normal; //kept on the side the box lies on

        planes[edge] = glm::vec4(normal, -glm::dot(normal, start));
    }

    return planes;
}

void RetroFuturaGUI::Shader::enableClipPlanes(const u32 count, const u32 previousCount)
{
    for(u32 plane = std::min(count, previousCount); std::max(count, previousCount) > plane; ++plane)
    {
        if(count > plane)
            glEnable(GL_CLIP_DISTANCE0 + plane);
        else
            glDisable(GL_CLIP_DISTANCE0 + plane);
    }
}

void RetroFuturaGUI::Shader::UseProgram()
{
    glUseProgram(_programId);
    SetUniformFloat("uOpacity", _drawOpacity); //a program without it, such as YuvToRgb's, ignores it
    SetUniformMat4("uFrame", _drawFrame); //likewise YuvToRgb draws into a texture, never tilted

    if(0 < _clipPlaneCount)
        SetUniformVec4("uClipPlanes", glm::value_ptr(_clipPlanes[0]), _clipPlaneCount); //outside every ClipScope the planes are off
}

void RetroFuturaGUI::Shader::SetUniformBool(std::string_view name, bool value) const
{
    glUniform1i(getUniformLocation(name), (int)value);
}

void RetroFuturaGUI::Shader::SetUniformInt(std::string_view name, int32_t value) const
{
    glUniform1i(getUniformLocation(name), value);
}

void RetroFuturaGUI::Shader::SetUniformFloat(std::string_view name, f32 value) const
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

void RetroFuturaGUI::Shader::SetUniformVec2(std::string_view name, f32 x, f32 y) const
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

void RetroFuturaGUI::Shader::SetUniformVec3(std::string_view name, f32 x, f32 y, f32 z) const
{
    glUniform3f(getUniformLocation(name), x, y, z);
}

void RetroFuturaGUI::Shader::SetUniformVec4(std::string_view name, const glm::vec4& value) const
{
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

void RetroFuturaGUI::Shader::SetUniformVec4(std::string_view name, const f32* value, const u32 count) const
{
    glUniform4fv(getUniformLocation(name), count, value);
}

void RetroFuturaGUI::Shader::SetUniformVec4(std::string_view name, f32 x, f32 y, f32 z, f32 w) const
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

u32 RetroFuturaGUI::Shader::compileShader(const char* shaderCode, const int32_t shaderType)
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