#include "Text.hpp"

RetroFuturaGUI::Text::Text(const GeometryParams2D& geometry, const TextParams& textParams)
: _projection(const_cast<Projection&>(geometry._Projection)), _parentSize(geometry._Size), 
  _text(textParams._Text), _fontPath(textParams._FontPath), _textColor(textParams._TextColor), 
  _textAlignment(textParams._TextAlignment), _textPadding(textParams._TextPadding)
{
    if(!_ft)
        initFreeTypeLibrary();

    initFontFace();
    bind();
    Resize(textParams._GlyphSize);
    Move(geometry._Position.x, geometry._Position.y);
    Rotate(geometry._Rotation);
    calculateTextSpan();
    SetTextAlignment(_textAlignment);
}

RetroFuturaGUI::Text::~Text()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
}

void RetroFuturaGUI::Text::Draw()
{
    f32 scale = _glyphSize.y * 0.00390625f;// equals dividing by 256.0f
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 copyX = x;
    ShaderManager::GetTextFillShader().UseProgram();
    ShaderManager::GetTextFillShader().SetUniformVec3("textColor", _textColor);
    ShaderManager::GetTextFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetTextFillShader().SetUniformMat4("uPosition", _translationMatrix);
    //ShaderManager::GetTextFillShader().SetUniformMat4("uScaling", _scaleMatrix);
    ShaderManager::GetTextFillShader().SetUniformMat4("uRotation", _rotationMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _textureArrayObj);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    i32 workingIndex = 0;

    std::string::const_iterator c;
    for (c = _text.begin(); c != _text.end(); ++c)
    {
        Character ch = _characters[*c];

        if (*c == '\n')
        {
            y -= ((ch.Size.y)) * 1.3 * scale;
            x = copyX;
        }
        else if (*c==' ')
        {
            x += (ch.Advance >> 6) * scale;
        }
        else
        {
            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (256 - ch.Bearing.y) * scale;

            _transforms[workingIndex] = glm::translate(glm::mat4(1.0f), glm::vec3(xpos, ypos, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(256 * scale, 256* scale, 0));
            _letterMap[workingIndex] = ch.TextureID;
            
            x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
            ++workingIndex;

            if (workingIndex == ARRAY_LIMIT )
            {
                textRenderCallback(workingIndex);
                workingIndex = 0;
            }
        }
    }

    textRenderCallback(workingIndex);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void RetroFuturaGUI::Text::SetTextAlignment(TextAlignment textAlignment)
{
    _textAlignment = textAlignment;

    switch(_textAlignment)
    {
        case TextAlignment::CENTER:
        {
            Move(_position.x - _textSpan.x * 0.5f, _position.y - _textBaseHeight * 0.5f);
        } break;
        case TextAlignment::RIGHT:
        {
            Move(_position.x - _textSpan.x + _parentSize.x * 0.5f - _textPadding, _position.y - _textBaseHeight * 0.5f);
        } break;
        default: //LEFT
        {
            Move(_position.x - _parentSize.x * 0.5f + _textPadding, _position.y - _textBaseHeight * 0.5f);
        }
    }
}

void RetroFuturaGUI::Text::Resize(const glm::vec2& glyphSize)
{
    _glyphSize = glyphSize;
    //_scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(_glyphSize, 1.0f));
}

void RetroFuturaGUI::Text::Move(const f32 x, const f32 y)
{
    _position = glm::vec2(x, y);
    _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_position, 0.0f));
}

void RetroFuturaGUI::Text::Rotate(const f32 rotation)
{
    _rotation = rotation;
    _rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
}

i32 RetroFuturaGUI::Text::initFreeTypeLibrary()
{
    i32 err = FT_Init_FreeType(&_ft);

    if (err == 0)
        return 0;

    std::println("ERROR::FREETYPE: Could not init FreeType Library");
    return -1;
}

i32 RetroFuturaGUI::Text::initFontFace()
{
    if (_fontPath.empty())
    {
        std::println("ERROR::FREETYPE: Failed to load font_name");
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(_ft, _fontPath.c_str(), 0, &face)) 
    {
        std::println("ERROR::FREETYPE: Failed to load font");
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 256, 256);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &_textureArrayObj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _textureArrayObj);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 256, 256, 128, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    // load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; ++c)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::println("ERROR::FREETYTPE: Failed to load Glyph");
            continue;
        }

        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0, 0, 0, int(c),
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows, 1,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        Character character = 
        {
            i32(c),
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };

        _characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    FT_Done_Face(face);
    FT_Done_FreeType(_ft);

    for (int i = 0; i < ARRAY_LIMIT; ++i) 
    {
        _letterMap.push_back(0);
        _transforms.push_back(glm::mat4(1.0f));
    }
}

void RetroFuturaGUI::Text::bind()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RetroFuturaGUI::Text::textRenderCallback(const u32 length)
{
    u32 shaderID = ShaderManager::GetTextFillShader().GetProgramId();

    if (!length)
        return;
    
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "transforms"), length, GL_FALSE, &_transforms[0][0][0]);
    glUniform1iv(glGetUniformLocation(shaderID, "letterMap"), length, &_letterMap[0]);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, length);
}

void RetroFuturaGUI::Text::calculateTextSpan()
{
    f32 scale = _glyphSize.y * _1emFraction;
    f32 maxLineWidth = 0.0f;
    f32 totalHeight = 0.0f;
    f32 currentLineWidth = 0.0f;
    f32 lineMaxAbove = 0.0f;   // max distance above baseline (bearing.y)
    f32 lineMaxBelow = 0.0f;   // max distance below baseline (size.y - bearing.y)
    f32 penX = 0.0f;           // pen x in pixels (already scaled)
    f32 globalMaxAbove = 0.0f; // max height from orign of a character

    auto finalizeLine = [&]() // if no glyph, fallback to a default line height (one em = 256px in glyph-space)
    {
        f32 lineHeight = (lineMaxAbove + lineMaxBelow);

        if (lineHeight <= 0.0f)
            lineHeight = 256.0f * scale;

        totalHeight += lineHeight * _lineSpacingFactor;
        maxLineWidth = max(maxLineWidth, currentLineWidth);

        globalMaxAbove = max(globalMaxAbove, lineMaxAbove);

        currentLineWidth = 0.0f;
        lineMaxAbove = 0.0f;
        lineMaxBelow = 0.0f;
        penX = 0.0f;
    };

    for (const char c : _text)
    {
        if (c == '\n')
        {
            finalizeLine();
            continue;
        }

        auto it = _characters.find(c);

        if (it == _characters.end()) // unknown char: treat as zero-width (you may want a fallback advance)
            continue;

        const Character &ch = it->second;
        f32 advancePx = static_cast<f32>(ch.Advance >> 6) * scale; // advance in pixels

        if (c == ' ')
        {
            penX += advancePx;
            currentLineWidth = max(currentLineWidth, penX);
            continue;
        }
        
        f32 glyphLeft = penX + static_cast<f32>(ch.Bearing.x) * scale;
        f32 glyphRight = glyphLeft + static_cast<f32>(ch.Size.x) * scale;
        currentLineWidth = max(currentLineWidth, glyphRight);

        // update line ascent/descent using bearing and bitmap size
        lineMaxAbove = max(lineMaxAbove, static_cast<f32>(ch.Bearing.y) * scale);
        lineMaxBelow = max(lineMaxBelow, (static_cast<f32>(ch.Size.y) - static_cast<f32>(ch.Bearing.y)) * scale);
        penX += advancePx;
    }

    finalizeLine();
    _textSpan = glm::vec2(maxLineWidth, totalHeight);
    _textBaseHeight = globalMaxAbove;
}