#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D uTexture;
uniform vec3 textColor;

void main()
{
    float alpha = texture(uTexture, TexCoord).r;
    FragColor = vec4(textColor, alpha);
}
