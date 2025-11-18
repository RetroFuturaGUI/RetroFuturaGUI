#version 330 core

layout(location = 0) out vec4 Color;
uniform vec4 uColor;

void main()
{
    Color = uColor;
}

/*  Usage: 
    i32 locColor = glGetUniformLocation(_shader->GetProgramId(), "uColors");
    glm::vec4 color = {0.5f, 0.0f, 1.0f, 1.0f};
    glUniform4fv(locColor, 1, &color[0]);
*/